#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lorawan-module.h"

using namespace ns3;

int main(int argc, char *argv[])
{
    CommandLine cmd;
    cmd.Parse(argc, argv);

    NodeContainer gateways;
    gateways.Create(1);

    NodeContainer endDevices;
    endDevices.Create(2);

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(gateways);
    mobility.Install(endDevices);

    // LoRaWAN helper
    Ptr<LorawanHelper> lorawanHelper = CreateObject<LorawanHelper>();

    // Install the gateway
    lorawanHelper->InstallGateway(gateways.Get(0));

    // Install end devices and attach them to the gateway
    for (uint32_t i = 0; i < endDevices.GetN(); ++i)
    {
        lorawanHelper->InstallEndDevice(endDevices.Get(i), gateways.Get(0));
    }

    Simulator::Stop(Seconds(10));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
