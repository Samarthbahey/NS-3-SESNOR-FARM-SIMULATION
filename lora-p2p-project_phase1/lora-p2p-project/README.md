# LoRa P2P Image Transfer Simulation

NS-3 simulation of image transfer over LoRa using EBYTE E220-900T22D modules.

## Hardware Setup
- **Camera**: Camera Module connected to Raspberry Pi 5 via CSI-2
- **TX Side**: Raspberry Pi 5 → EBYTE E220-900T22D (868MHz, 22dBm)
- **RX Side**: EBYTE E220-900T22D → Raspberry Pi 5
- **Distance**: 1-1.2 km
- **Data Rate**: 5.47 kbps (SF7, BW125, CR4/5)

## Files
- `lora-p2p-transfer.cc` - Main simulation source code (19 KB)
- `lora-p2p-transfer` - Compiled executable binary (2.2 MB)
- `lora-p2p-transfer.xml` - NetAnim trace file for visualization (181 KB)
- `lora-routes.xml` - IPv4 route tracking file (96 KB)
- `received_image.bin` - Received image data (generated after simulation)
- `lora-test.cc` - Additional test/development file
- `lora-wifi-0-0.pcap` - Packet capture file for analysis (Wireshark compatible)

## Usage
Copy the source file to your NS-3 scratch directory:
```bash
cp lora-p2p-transfer.cc /path/to/ns-3.45/scratch/
```

Build and run:
```bash
cd /path/to/ns-3.45
./ns3 run scratch/lora-p2p-transfer
```

With parameters:
```bash
./ns3 run "scratch/lora-p2p-transfer --loss=0.05 --size=20480 --time=36000"
```

## Parameters
- `--loss`: Packet loss rate (0.0-1.0)
- `--size`: Image size in KB (default: 20480 = 20 MB)
- `--time`: Simulation time in seconds (default: 36000 = 10 hours)

## Visualization
Open the `lora-p2p-transfer.xml` file in NetAnim to visualize the packet transmission.

## Schedule
ONE 20 MB image per day - no new capture until current transfer completes.
