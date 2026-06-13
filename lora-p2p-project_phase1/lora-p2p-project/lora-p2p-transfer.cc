/*
 * LoRa P2P Image Transfer Simulation
 * Daily Image Capture and Transfer
 * Hardware: Camera -> Raspberry Pi 5 -> EBYTE E220-900T22D LoRa Module (868MHz)
 * Platform: Raspberry Pi 5 with Camera Module
 * Distance: 1-1.2 km
 * Tx Power: 22 dBm
 * Schedule: ONE 20 MB image per day, complete transfer before next capture
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include <fstream>
#include <map>
#include <set>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LoRaP2PTransfer");

// LoRa Configuration - EBYTE E220-900T22D (868MHz)
const uint16_t PORT = 9999;
const uint32_t CHUNK_SIZE = 92;  // LoRa payload size
const double ACK_TIMEOUT = 10.0;
const double CHUNK_DELAY = 0.5;  // Inter-packet delay

// Hardware specifications
const double LORA_FREQUENCY = 868.0;    // MHz (European ISM band)
const double TX_POWER = 22.0;           // dBm (EBYTE E220-900T22D)
const double DISTANCE = 1100.0;         // meters (1.1 km typical)
const double LORA_DATA_RATE = 5.47;     // kbps (SF7, BW125, CR4/5)

// Global metrics
struct Metrics {
    uint32_t chunksSent = 0;
    uint32_t retransmissions = 0;
    uint32_t acksSent = 0;
    uint32_t acksReceived = 0;
    uint32_t chunksReceived = 0;  // Track received chunks
    Time startTime;
} g_metrics;

// Global animation interface pointer for updating
AnimationInterface* g_anim = nullptr;

// Simple sender application
class ImageSender : public Application {
public:
    void Setup(Address addr, const std::vector<uint8_t>& data) {
        m_peer = addr;
        m_imageData = data;
        m_totalChunks = (data.size() + CHUNK_SIZE - 1) / CHUNK_SIZE;
    }

protected:
    void StartApplication() override {
        m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
        m_socket->Bind();
        m_socket->Connect(m_peer);
        m_socket->SetRecvCallback(MakeCallback(&ImageSender::HandleRead, this));
        
        g_metrics.startTime = Simulator::Now();
        Simulator::Schedule(Seconds(1.0), &ImageSender::SendChunks, this);
    }

    void StopApplication() override {
        if (m_socket) {
            m_socket->Close();
        }
    }

private:
    void SendChunks() {
        NS_LOG_INFO("[SENDER] Starting transmission of " << m_totalChunks << " chunks");
        for (uint32_t i = 0; i < m_totalChunks; i++) {
            m_pendingChunks.insert(i);
            Simulator::Schedule(Seconds(i * CHUNK_DELAY), &ImageSender::SendChunk, this, i);
        }
        Simulator::Schedule(Seconds(ACK_TIMEOUT + m_totalChunks * CHUNK_DELAY), 
                          &ImageSender::CheckAcks, this);
    }

    void SendChunk(uint32_t seqNum) {
        uint32_t offset = seqNum * CHUNK_SIZE;
        uint32_t size = std::min(CHUNK_SIZE, (uint32_t)(m_imageData.size() - offset));
        
        // Create packet: [type(1)][seqNum(4)][total(4)][data]
        uint8_t buffer[1024];
        buffer[0] = 1; // DATA packet
        *((uint32_t*)(buffer + 1)) = seqNum;
        *((uint32_t*)(buffer + 5)) = m_totalChunks;
        memcpy(buffer + 9, &m_imageData[offset], size);
        
        Ptr<Packet> packet = Create<Packet>(buffer, 9 + size);
        m_socket->Send(packet);
        g_metrics.chunksSent++;
        
        // Update NetAnim with chunk count
        if (g_anim) {
            std::string status = "Sent: " + std::to_string(g_metrics.chunksSent) + "/" + std::to_string(m_totalChunks);
            g_anim->UpdateNodeDescription(GetNode(), status);
        }
        
        NS_LOG_INFO("[SENDER] Sent chunk " << seqNum << "/" << m_totalChunks);
    }

    void HandleRead(Ptr<Socket> socket) {
        Ptr<Packet> packet;
        while ((packet = socket->Recv())) {
            uint8_t buffer[1024];
            packet->CopyData(buffer, packet->GetSize());
            
            if (buffer[0] == 2) { // ACK packet
                uint32_t numAcks = (packet->GetSize() - 1) / 4;
                for (uint32_t i = 0; i < numAcks; i++) {
                    uint32_t seqNum = *((uint32_t*)(buffer + 1 + i * 4));
                    m_pendingChunks.erase(seqNum);
                    g_metrics.acksReceived++;
                }
                NS_LOG_INFO("[SENDER] Received ACKs for " << numAcks << " chunks");
            }
        }
    }

    void CheckAcks() {
        if (m_pendingChunks.empty()) {
            NS_LOG_INFO("[SENDER] ✓ Transfer complete!");
            NS_LOG_INFO("[SENDER] Duration: " << (Simulator::Now() - g_metrics.startTime).GetSeconds() << "s");
        } else {
            NS_LOG_INFO("[SENDER] Retransmitting " << m_pendingChunks.size() << " missing chunks");
            for (uint32_t seqNum : m_pendingChunks) {
                SendChunk(seqNum);
                g_metrics.retransmissions++;
            }
            Simulator::Schedule(Seconds(ACK_TIMEOUT), &ImageSender::CheckAcks, this);
        }
    }

    Ptr<Socket> m_socket;
    Address m_peer;
    std::vector<uint8_t> m_imageData;
    uint32_t m_totalChunks;
    std::set<uint32_t> m_pendingChunks;
    bool m_transferComplete = false;  // Flag to prevent re-capture/retransmit after completion
};

// Simple receiver application
class ImageReceiver : public Application {
public:
    void Setup(uint32_t expectedSize) {
        m_expectedSize = expectedSize;
    }

protected:
    void StartApplication() override {
        m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
        m_socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), PORT));
        m_socket->SetRecvCallback(MakeCallback(&ImageReceiver::HandleRead, this));
        
        Simulator::Schedule(Seconds(2.0), &ImageReceiver::SendAcks, this);
    }

    void StopApplication() override {
        if (m_socket) {
            m_socket->Close();
        }
        if (m_complete) {
            SaveImage();
        }
    }

private:
    void HandleRead(Ptr<Socket> socket) {
        Ptr<Packet> packet;
        Address from;
        while ((packet = socket->RecvFrom(from))) {
            uint8_t buffer[1024];
            packet->CopyData(buffer, packet->GetSize());
            
            if (buffer[0] == 1) { // DATA packet
                uint32_t seqNum = *((uint32_t*)(buffer + 1));
                uint32_t total = *((uint32_t*)(buffer + 5));
                uint32_t dataSize = packet->GetSize() - 9;
                
                if (m_chunks.find(seqNum) == m_chunks.end()) {
                    std::vector<uint8_t> data(buffer + 9, buffer + 9 + dataSize);
                    m_chunks[seqNum] = data;
                    m_peerAddr = InetSocketAddress::ConvertFrom(from);
                    g_metrics.chunksReceived++;
                    
                    // Update NetAnim with received count
                    if (g_anim) {
                        std::string status = "Received: " + std::to_string(m_chunks.size()) + "/" + std::to_string(total);
                        g_anim->UpdateNodeDescription(GetNode(), status);
                    }
                    
                    NS_LOG_INFO("[RECEIVER] Got chunk " << seqNum << "/" << total 
                                << " (total: " << m_chunks.size() << ")");
                    
                    if (m_chunks.size() == total) {
                        NS_LOG_INFO("[RECEIVER] ✓ All chunks received!");
                        m_complete = true;
                        SaveImage();  // Save immediately when complete
                        if (g_anim) {
                            g_anim->UpdateNodeDescription(GetNode(), "COMPLETE: " + std::to_string(total) + " chunks");
                        }
                    }
                }
            }
        }
    }

    void SendAcks() {
        if (m_complete) {
            // Transfer complete - send final ACK but don't reschedule
            if (m_chunks.empty()) return;
            
            uint8_t buffer[1024];
            buffer[0] = 2; // ACK packet
            uint32_t count = 0;
            for (auto& pair : m_chunks) {
                *((uint32_t*)(buffer + 1 + count * 4)) = pair.first;
                count++;
            }
            
            Ptr<Packet> ackPacket = Create<Packet>(buffer, 1 + count * 4);
            m_socket->SendTo(ackPacket, 0, m_peerAddr);
            g_metrics.acksSent++;
            NS_LOG_INFO("[RECEIVER] Sent final ACKs (TRANSFER COMPLETE)");
            return;  // Don't reschedule anymore
        }
        
        if (m_chunks.empty()) {
            Simulator::Schedule(Seconds(2.0), &ImageReceiver::SendAcks, this);
            return;
        }
        
        // Create ACK packet
        uint8_t buffer[1024];
        buffer[0] = 2; // ACK packet
        uint32_t count = 0;
        for (auto& pair : m_chunks) {
            *((uint32_t*)(buffer + 1 + count * 4)) = pair.first;
            count++;
        }
        
        Ptr<Packet> ackPacket = Create<Packet>(buffer, 1 + count * 4);
        m_socket->SendTo(ackPacket, 0, m_peerAddr);
        g_metrics.acksSent++;
        
        NS_LOG_INFO("[RECEIVER] Sent ACKs for " << count << " chunks");
        
        if (!m_complete) {
            Simulator::Schedule(Seconds(2.0), &ImageReceiver::SendAcks, this);
        }
    }

    void SaveImage() {
        std::vector<uint8_t> image;
        for (auto& pair : m_chunks) {
            image.insert(image.end(), pair.second.begin(), pair.second.end());
        }
        
        std::ofstream file("received_image.bin", std::ios::binary);
        file.write((char*)image.data(), image.size());
        file.close();
        
        NS_LOG_INFO("[RECEIVER] Saved " << image.size() << " bytes to received_image.bin");
    }

    Ptr<Socket> m_socket;
    Address m_peerAddr;
    uint32_t m_expectedSize;
    bool m_complete = false;
    std::map<uint32_t, std::vector<uint8_t>> m_chunks;
};

int main(int argc, char* argv[]) {
    double lossRate = 0.0;
    uint32_t imageSizeKB = 20480;  // 20 MB default
    double simTime = 36000.0;      // 10 hours for large image transfer

    CommandLine cmd;
    cmd.AddValue("loss", "Packet loss rate (0.0-1.0)", lossRate);
    cmd.AddValue("size", "Image size in KB", imageSizeKB);
    cmd.AddValue("time", "Simulation time (seconds)", simTime);
    cmd.Parse(argc, argv);

    std::cout << "\n=== LoRa P2P Image Transfer ===" << std::endl;
    std::cout << "Schedule: ONE image per day (20 MB)" << std::endl;
    std::cout << "Image: " << imageSizeKB << " KB (" << (imageSizeKB/1024.0) << " MB)" << std::endl;
    std::cout << "Loss: " << (lossRate * 100) << "%" << std::endl;
    std::cout << "Chunk size: " << CHUNK_SIZE << " bytes\n" << std::endl;

    LogComponentEnable("LoRaP2PTransfer", LOG_LEVEL_INFO);

    // Create nodes: Camera, RPi5s and LoRa modules
    NodeContainer nodes;
    nodes.Create(5);  // 0: Camera, 1: RPi5-TX, 2: LoRa-TX, 3: LoRa-RX, 4: RPi5-RX
    
    NodeContainer cameraSide;  // Camera connected to RPi5
    cameraSide.Add(nodes.Get(0));
    cameraSide.Add(nodes.Get(1));
    
    NodeContainer txSide;  // RPi5 and LoRa module on transmitter side
    txSide.Add(nodes.Get(1));
    txSide.Add(nodes.Get(2));
    
    NodeContainer rxSide;  // RPi5 and LoRa module on receiver side
    rxSide.Add(nodes.Get(3));
    rxSide.Add(nodes.Get(4));
    
    NodeContainer loraModules;  // LoRa-to-LoRa wireless link
    loraModules.Add(nodes.Get(2));
    loraModules.Add(nodes.Get(3));

    // Mobility model - staggered layout for clear visualization
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    // Arrange in staggered positions to avoid overlap
    positionAlloc->Add(Vector(50.0, 200.0, 0.0));     // Camera Module (top left)
    positionAlloc->Add(Vector(300.0, 150.0, 0.0));    // RPi5 #1 (middle left)
    positionAlloc->Add(Vector(550.0, 100.0, 0.0));    // E220-900T22D TX (bottom left)
    positionAlloc->Add(Vector(1750.0, 100.0, 0.0));   // E220-900T22D RX (bottom right)
    positionAlloc->Add(Vector(2000.0, 150.0, 0.0));   // RPi5 #2 (middle right)
    mobility.SetPositionAllocator(positionAlloc);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    // CSI connection: Camera <-> RPi5 (high speed, local)
    PointToPointHelper csi;
    csi.SetDeviceAttribute("DataRate", StringValue("1Gbps"));  // CSI-2 interface speed
    csi.SetChannelAttribute("Delay", StringValue("1us"));      // Local connection

    NetDeviceContainer csiDevices = csi.Install(cameraSide);  // Camera <-> RPi5-TX

    // UART connection: RPi5 <-> LoRa module (high speed, local)
    PointToPointHelper uart;
    uart.SetDeviceAttribute("DataRate", StringValue("115200bps")); // UART baud rate
    uart.SetChannelAttribute("Delay", StringValue("1us"));         // Local connection

    NetDeviceContainer uartTxDevices = uart.Install(txSide);  // RPi5-TX <-> LoRa-TX
    NetDeviceContainer uartRxDevices = uart.Install(rxSide);  // LoRa-RX <-> RPi5-RX

    // LoRa wireless link: 1.1 km distance
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5470bps")); // LoRa SF7 data rate
    p2p.SetChannelAttribute("Delay", StringValue("10ms"));      // Air propagation + processing

    NetDeviceContainer loraDevices = p2p.Install(loraModules);

    // Add packet loss to LoRa receiver (wireless link)
    if (lossRate > 0) {
        Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
        em->SetAttribute("ErrorRate", DoubleValue(lossRate));
        loraDevices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));
    }

    // Internet stack
    InternetStackHelper internet;
    internet.Install(nodes);

    // Assign IP addresses
    Ipv4AddressHelper ipv4;
    
    // Camera to RPi5 CSI network
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer cameraInterfaces = ipv4.Assign(csiDevices);
    
    // TX side UART network
    ipv4.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer txInterfaces = ipv4.Assign(uartTxDevices);
    
    // LoRa wireless network
    ipv4.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer loraInterfaces = ipv4.Assign(loraDevices);
    
    // RX side UART network
    ipv4.SetBase("10.1.4.0", "255.255.255.0");
    Ipv4InterfaceContainer rxInterfaces = ipv4.Assign(uartRxDevices);
    
    // Enable routing so packets can traverse RPi -> LoRa -> LoRa -> RPi
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Create image data
    std::vector<uint8_t> imageData(imageSizeKB * 1024);
    for (size_t i = 0; i < imageData.size(); i++) {
        imageData[i] = i % 256;
    }
    uint32_t totalChunks = (imageData.size() + CHUNK_SIZE - 1) / CHUNK_SIZE;
    std::cout << "Total chunks: " << totalChunks << "\n" << std::endl;

    // Install applications on Camera (source) and RPi5-RX (destination)
    Ptr<ImageSender> sender = CreateObject<ImageSender>();
    Ptr<ImageReceiver> receiver = CreateObject<ImageReceiver>();

    // Sender on Camera, send to RPi5-RX (packets route through RPi5-TX -> LoRa modules)
    sender->Setup(InetSocketAddress(rxInterfaces.GetAddress(1), PORT), imageData);
    receiver->Setup(imageData.size());

    nodes.Get(0)->AddApplication(sender);  // Camera captures and sends
    nodes.Get(4)->AddApplication(receiver); // RPi5-RX receives

    sender->SetStartTime(Seconds(0));
    sender->SetStopTime(Seconds(simTime));
    receiver->SetStartTime(Seconds(0));
    receiver->SetStopTime(Seconds(simTime));

    // NetAnim configuration
    AnimationInterface anim("lora-p2p-transfer.xml");
    g_anim = &anim;  // Set global pointer for updates
    
    // Set node descriptions (will be updated with chunk counts)
    anim.UpdateNodeDescription(nodes.Get(0), "Camera (Ready)");
    anim.UpdateNodeDescription(nodes.Get(1), "RPi5-TX (Ready)");
    anim.UpdateNodeDescription(nodes.Get(2), "LoRa-TX (Idle)");
    anim.UpdateNodeDescription(nodes.Get(3), "LoRa-RX (Waiting)");
    anim.UpdateNodeDescription(nodes.Get(4), "RPi5-RX (Waiting)");
    
    // Set distinct colors for each component type
    anim.UpdateNodeColor(nodes.Get(0), 255, 140, 0);   // Dark Orange for Camera
    anim.UpdateNodeColor(nodes.Get(1), 0, 200, 0);     // Green for RPi5-TX
    anim.UpdateNodeColor(nodes.Get(2), 255, 0, 0);     // Red for LoRa-TX
    anim.UpdateNodeColor(nodes.Get(3), 0, 0, 255);     // Blue for LoRa-RX
    anim.UpdateNodeColor(nodes.Get(4), 0, 200, 0);     // Green for RPi5-RX
    
    // Make nodes BIGGER for better visibility
    anim.UpdateNodeSize(nodes.Get(0)->GetId(), 15.0, 15.0);  // Camera - larger
    anim.UpdateNodeSize(nodes.Get(1)->GetId(), 20.0, 20.0);  // RPi5-TX - largest
    anim.UpdateNodeSize(nodes.Get(2)->GetId(), 12.0, 12.0);  // LoRa-TX - medium
    anim.UpdateNodeSize(nodes.Get(3)->GetId(), 12.0, 12.0);  // LoRa-RX - medium
    anim.UpdateNodeSize(nodes.Get(4)->GetId(), 20.0, 20.0);  // RPi5-RX - largest
    
    // Enable packet metadata for chunk visualization
    anim.EnablePacketMetadata(true);
    anim.EnableIpv4RouteTracking("lora-routes.xml", Seconds(0), Seconds(simTime), Seconds(5));
    
    std::cout << "NetAnim trace file: lora-p2p-transfer.xml\n" << std::endl;
    std::cout << "Chunk visualization: Node labels show chunk counts" << std::endl;
    std::cout << "Packets: Each packet represents one 92-byte chunk\n" << std::endl;
    std::cout << "=== Daily Capture Schedule ===" << std::endl;
    std::cout << "Camera captures ONE 20 MB image per day" << std::endl;
    std::cout << "No new capture until current transfer completes\n" << std::endl;
    std::cout << "Hardware Setup:" << std::endl;
    std::cout << "  Camera Module <-CSI-> Raspberry Pi 5 <-UART-> EBYTE E220-900T22D (868MHz, 22dBm)" << std::endl;
    std::cout << "  EBYTE E220-900T22D <-UART-> Raspberry Pi 5" << std::endl;
    std::cout << "Wireless Distance: " << DISTANCE << " meters (1.1 km)" << std::endl;
    std::cout << "LoRa Data Rate: " << LORA_DATA_RATE << " kbps (SF7)\n" << std::endl;

    // Run
    Simulator::Stop(Seconds(simTime));
    Simulator::Run();

    // Results
    std::cout << "\n=== Results ===" << std::endl;
    std::cout << "Chunks sent: " << g_metrics.chunksSent << std::endl;
    std::cout << "Chunks received: " << g_metrics.chunksReceived << std::endl;
    std::cout << "Retransmissions: " << g_metrics.retransmissions << std::endl;
    std::cout << "ACKs sent: " << g_metrics.acksSent << std::endl;
    std::cout << "ACKs received: " << g_metrics.acksReceived << std::endl;
    std::cout << "Retransmit rate: " 
              << (g_metrics.chunksSent > 0 ? (g_metrics.retransmissions * 100.0 / g_metrics.chunksSent) : 0) 
              << "%\n" << std::endl;

    Simulator::Destroy();
    return 0;
}
