/*
###############################################
# Author:                                     #
# kamran shafafi [kamran.shafafi@inesctec.pt] #
###############################################
*/

#include "lib/json.hpp"
#include "lib/simconf.h"
#include "lib/simlogs.h"
#include "lib/myutils.h"
#include "rl/utilgym.h"
#include "rl/gym.h"
#include "rl/utilgym.h"
#include <ns3/network-module.h>
#include <ns3/wifi-module.h>
#include <ns3/internet-module.h>
#include <ns3/flow-monitor-module.h>
#include <ns3/core-module.h>
#include <ns3/opengym-module.h>
#include <ns3/ns3-ai-module.h>
#include <ns3/core-module.h>
#include "ns3/packet-sink.h"

using namespace ns3;
using json = nlohmann::json;
NS_LOG_COMPONENT_DEFINE("sim");
Ptr<PacketSink> sink;

int main(int argc, char **argv)
{
    json config = SetupJsonConfig();

    configLogs(config, "sim");
    configLogs(config, "simlogs");
    configLogs(config, "myutils");
    configLogs(config, "simconf");
    configLogs(config, "utilgym");

    NS_LOG_INFO("INFO: Starting NS-3 Simulation configuration...");

    config = configCMD(argc, argv, config);
    json defaultValues = config["ns3Sim"]["defaultValues"];
    json configSim = config["ns3Sim"]["configSim"];

    RngSeedManager::SetSeed(defaultValues["simSeed"].get<double>());
    RngSeedManager::SetRun(defaultValues["simRun"].get<double>());

    // all network Nodes
    NodeContainer UEUAV;
    UEUAV.Create(defaultValues["UENodes"].size() + defaultValues["UAVNodes"].size());

    // config Mobility
    configUEMobility(config);

    int xPosition = defaultValues["UAVNodes"][0]["positionVector"][0].get<int32_t>();
    int yPosition = defaultValues["UAVNodes"][0]["positionVector"][1].get<int32_t>();
    int zPosition = defaultValues["UAVNodes"][0]["positionVector"][2].get<int32_t>();

    configUAVPosition(config, xPosition, yPosition, zPosition);

    // Buildings
    std::vector<Ptr<Building>> buildings = BuildingImplement(config);

    NumOfLoS(config);

    WifiHelper wifi;
    wifi = configWifi(config);

    YansWifiPhyHelper wifiPhy;
    wifiPhy = configWifiPhy(config);

    WifiMacHelper wifiMac;
    wifiMac = configWifiMac(config);

    NetDeviceContainer devices;
    devices = wifi.Install(wifiPhy, wifiMac, UEUAV);

    InternetStackHelper internet;
    internet.Install(UEUAV);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.0.0.0", "255.255.255.0");

    Ipv4InterfaceContainer interfaces;
    interfaces = ipv4.Assign(devices);

    Time start = Seconds(0);
    Time stop = Seconds(config["ns3Sim"]["defaultValues"]["simDuration"].get<double>());

    configApps(config, UEUAV, start, stop, interfaces);

    NS_LOG_INFO("INFO: Starting NS-3 Simulation configuration... Ok!");

    Ptr<MyGymEnv> myGymEnv = nullptr;
    Ptr<OpenGymInterface> openGymInterface = nullptr;

    if (configSim["enableGym"].get<bool>())
    {
        NS_LOG_INFO("INFO: Starting Gym...");
        openGymInterface = CreateObject<OpenGymInterface>(defaultValues["openGymPort"].get<double>());
        myGymEnv = CreateObject<MyGymEnv>(config, config["ns3Sim"]["configSim"]["path"].get<std::string>());
        myGymEnv->SetOpenGymInterface(openGymInterface);
        NS_LOG_INFO("INFO: Starting Gym... Ok!");
    }

    NS_LOG_INFO("INFO: Starting Simulation.");
    Simulator::Stop(Seconds(config["ns3Sim"]["defaultValues"]["simDuration"].get<double>()));
    Simulator::Run();
    NS_LOG_INFO("INFO: Finishing Simulation.");
    Simulator::Destroy();
    return 0;
}
