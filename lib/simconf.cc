/*
###############################################
# Author:                                     #
# Kamran Shafafi [kamran.shafafi@inesctec.pt] #
###############################################
*/
#include <iostream>
#include <sstream>
#include <string>
#include "simconf.h"
#include "myutils.h"
#include "../rl/utilgym.h"
#include "json.hpp"
#include <ns3/log.h>
#include "ns3/packet-sink.h"
#include <ns3/wifi-module.h>
#include <ns3/core-module.h>
#include <ns3/internet-module.h>
#include <ns3/applications-module.h>
#include <ns3/mobility-module.h>
#include <ns3/buildings-module.h>
#include <ns3/mobility-building-info.h>
#include <ns3/buildings-helper.h>
#include <ns3/buildings-channel-condition-model.h>
#include <ns3/itu-r-1411-los-propagation-loss-model.h>
#include <ns3/itu-r-1411-nlos-over-rooftop-propagation-loss-model.h>

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("simconf");
  using json = nlohmann::json;

  void
  configApps(json config, NodeContainer nodes, Time start, Time stop, Ipv4InterfaceContainer interfaces)
  {
    NS_LOG_INFO("INFO: Configuring Apps...");
    json defaultValues = config["ns3Sim"]["defaultValues"];
    std::string appType = config["ns3Sim"]["configSim"]["trafficProtocol"].get<std::string>();
    ApplicationContainer appContainertx, appContainerrx;
    json defaultValuesUE = config["ns3Sim"]["defaultValues"]["UENodes"];
    Ptr<PacketSink> sink;

    // static Ipv4InterfaceContainer localinterfaces = interfaces;

    if (appType == "tcp")
    {
      Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1400)); // 1448 tomaso
      PacketSinkHelper rx("ns3::TcpSocketFactory",
                          InetSocketAddress(Ipv4Address::GetAny(), 9)); // port number 9
      appContainerrx = rx.Install(nodes.Get(0));
      sink = StaticCast<PacketSink>(appContainerrx.Get(0));
    }
    else if (appType == "udp")
    {
      PacketSinkHelper rx("ns3::UdpSocketFactory",
                          InetSocketAddress(Ipv4Address::GetAny(), 9)); // port number 9
      appContainerrx = rx.Install(nodes.Get(0));
      sink = StaticCast<PacketSink>(appContainerrx.Get(0));
    }
    else
    {
      NS_LOG_ERROR("Unknown application type: " << appType);
      // Add appropriate error handling or set a default value for sink
    }

    if (appType == "tcp")
    {
      for (std::size_t j = 0; j < defaultValuesUE.size(); ++j)
      {
        BulkSendHelper tx("ns3::TcpSocketFactory",
                          InetSocketAddress(interfaces.GetAddress(0), 9));
        tx.SetAttribute(
            "MaxBytes",
            UintegerValue(0)); // Set the amount of data to send in bytes.  Zero is unlimited.
        appContainertx.Add(tx.Install(nodes.Get(j + 1)));
      }
    }
    else if (appType == "udp")
    {
      for (std::size_t j = 0; j < defaultValuesUE.size(); ++j)
      {
        OnOffHelper tx("ns3::UdpSocketFactory", InetSocketAddress(interfaces.GetAddress(0), 9)); // send to rx
        tx.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
        tx.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]")); // allways ON
        tx.SetAttribute("PacketSize", UintegerValue(1400));
        tx.SetAttribute("DataRate", DataRateValue(DataRate(defaultValuesUE[j]["DataRate"].get<std::string>()))); // transmission data-rate for the UDP connection in Mbps for each tx nodes
        appContainertx.Add(tx.Install(nodes.Get(j + 1)));
      }
    }

    uint32_t nApplicationsrx = appContainerrx.GetN();
    uint32_t nApplicationstx = appContainertx.GetN();

    NS_LOG_INFO("nApplicationsrx is: " << nApplicationsrx << ", and nApplicationstx is: " << nApplicationstx);

    // Define the start time
    Time startTime = start;
    // Print a notification when the application is about to start
    std::cout << "My application is starting at " << startTime.GetSeconds() << " seconds." << std::endl;

    // Start your application
    appContainertx.Start(startTime);
    appContainerrx.Start(startTime);

    // Define the simulation duration based on the configuration
    // Time stopTime = Now + decision_interval;
    Time stopTime = stop;
    std::cout << "My application is stoping at " << stopTime.GetSeconds() << " seconds." << std::endl;

    // Stop your application after the defined simulation duration
    appContainertx.Stop(stopTime);
    appContainerrx.Stop(stopTime);

    NS_LOG_INFO("INFO: Configuring Apps... Ok!");
    return;
  }

  std::vector<Ptr<Building>>
  BuildingImplement(json config)
  {
    NS_LOG_INFO("INFO: Implementation Buildings...");
    json BuildingsCord = config["ns3Sim"]["defaultValues"]["Buildings"];
    std::vector<Ptr<Building>> buildings;

    for (std::size_t i = 0; i < BuildingsCord.size(); ++i)
    {
      Ptr<Building> b = CreateObject<Building>();
      buildings.push_back(b);
      b->SetBoundaries(Box(BuildingsCord[i]["BuildingPosition"][0].get<double>(), BuildingsCord[i]["BuildingPosition"][1].get<double>(), BuildingsCord[i]["BuildingPosition"][2].get<double>(), BuildingsCord[i]["BuildingPosition"][3].get<double>(), BuildingsCord[i]["BuildingPosition"][4].get<double>(), BuildingsCord[i]["BuildingPosition"][5].get<double>()));
      b->SetBuildingType(Building::Residential);
      b->SetExtWallsType(Building::ConcreteWithWindows);
      b->SetNFloors(BuildingsCord[i]["BuildingPosition"][6].get<uint8_t>());
      b->SetNRoomsX(BuildingsCord[i]["BuildingPosition"][4].get<uint8_t>());
      b->SetNRoomsY(BuildingsCord[i]["BuildingPosition"][8].get<uint8_t>());
    }
    NS_LOG_INFO("INFO: Implementation Buildings...OK!");
    return buildings;
  }

  void
  configUEMobility(json config)
  {
    NS_LOG_INFO("INFO: Configuring the UEs Mobility...");
    json configSim = config["ns3Sim"]["configSim"];
    json defaultValuesUE = config["ns3Sim"]["defaultValues"]["UENodes"];
    NodeContainer nodes = NodeContainer::GetGlobal();
    std::string mobilityModel = configSim["mobilityModel"].get<std::string>();

    if (mobilityModel == "const")
      NS_LOG_INFO("INFO: the mobility of the scenario is constant...");
    {
      Ptr<MobilityModel> mobility{};
      Ptr<MobilityBuildingInfo> buildingInfo = CreateObject<MobilityBuildingInfo>();
      for (std::size_t i = 0; i < defaultValuesUE.size(); ++i)
      {
        NS_LOG_INFO("Positioning Node UE: " << i << " ...");
        mobility = CreateObject<ConstantPositionMobilityModel>();

        mobility->SetPosition(Vector(defaultValuesUE[i]["positionVector"][0].get<double>(),   // x
                                     defaultValuesUE[i]["positionVector"][1].get<double>(),   // y
                                     defaultValuesUE[i]["positionVector"][2].get<double>())); // z

        nodes.Get(i + 1)->AggregateObject(mobility);

        std::cout << "The position of UE #" << i << " is: " << nodes.Get(i + 1)->GetObject<MobilityModel>()->GetPosition() << std::endl;

        BuildingsHelper::Install(nodes.Get(i + 1));
      }
      buildingInfo->MakeConsistent(mobility);
      NS_LOG_INFO("INFO: Configuring the UEs Mobility... Ok!");
    }
    return;
  }

  void
  configUAVPosition(json config, int xPosition, int yPosition, int zPosition)
  {
    NS_LOG_INFO("INFO: Configuring the positions of UAV ...");

    NodeContainer nodes = NodeContainer::GetGlobal();
    Ptr<MobilityBuildingInfo> buildingInfo = CreateObject<MobilityBuildingInfo>();
    Ptr<MobilityModel> mobility = CreateObject<ConstantPositionMobilityModel>();

    mobility->SetPosition(Vector(xPosition,   // x
                                 yPosition,   // y
                                 zPosition)); // z

    nodes.Get(0)->AggregateObject(mobility);

    std::cout << "The new position of UAV is: " << nodes.Get(0)->GetObject<MobilityModel>()->GetPosition() << std::endl;

    BuildingsHelper::Install(nodes.Get(0));
    buildingInfo->MakeConsistent(mobility);

    // mobility.SetPositionAllocator(positionAlloc);
    NS_LOG_INFO("INFO: Configuring the positions of UAV ... Ok!");
    return;
  }

  WifiHelper
  configWifi(json config)
  {
    NS_LOG_INFO("INFO: Configuring Wifi...");

    WifiHelper wifi;

    json configSim = config["ns3Sim"]["configSim"];
    std::string wifiStandard = configSim["wifiStandard"].get<std::string>();
    std::string RemotStationManager = configSim["RemotStManager"].get<std::string>();
    std::vector<std::string> dataRates;

    if (wifiStandard == "WIFI_STANDARD_80211n")
    {
      dataRates = configSim["htDataRates"]; // define datarates   HT->11n VHT->11ac
      wifi.SetStandard(WIFI_STANDARD_80211n);
    }
    else if (wifiStandard == "WIFI_STANDARD_80211ac")
    {
      dataRates = configSim["vhtDataRates"]; // define datarates   HT->11n VHT->11ac
      wifi.SetStandard(WIFI_STANDARD_80211ac);
    }

    if (RemotStationManager == "min")
      wifi.SetRemoteStationManager("ns3::MinstrelHtWifiManager");
    else if (RemotStationManager == "id")
      wifi.SetRemoteStationManager("ns3::IdealWifiManager");

    else if (RemotStationManager == "fixed")
    {
      std::string dataRateStr = dataRates.at(configSim["fixedMCS"].get<double>());
      wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                                   StringValue(dataRateStr), "ControlMode",
                                   StringValue(dataRateStr));
    }
    NS_LOG_INFO("INFO: Configuring Wifi... Ok!");
    return wifi;
  }

  YansWifiPhyHelper
  configWifiPhy(json config)
  {
    NS_LOG_INFO("INFO: Configuring WifiPhy...");
    json defaultValues = config["ns3Sim"]["defaultValues"];
    YansWifiPhyHelper wifiPhy;

    int channelBW = defaultValues["channelBW"].get<uint32_t>();
    int GuardInterval = defaultValues["GuardInterval"].get<uint32_t>();
    Time guardInterval = NanoSeconds(GuardInterval);

    uint64_t frequency = determineWifiChannelFrequencyHz(config);

    double freqMHz = frequency / 1e6;
    int CHANNEL_NUMBER = defaultValues["wifiChannelNumber"].get<uint32_t>();

    wifiPhy.Set("Frequency", UintegerValue(freqMHz));      // https://en.wikipedia.org/wiki/List_of_WLAN_channels
    wifiPhy.Set("ChannelWidth", UintegerValue(channelBW)); // MHz || 20 -> 5180 || 40 -> 5230 || 80 -> 5290 || 160 ->5250
    Config::Set("/NodeList/*/DeviceList/*/F/Phy/GuardInterval", TimeValue(guardInterval));
    wifiPhy.Set("ChannelNumber", UintegerValue(CHANNEL_NUMBER));
    wifiPhy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
    wifiPhy.SetErrorRateModel("ns3::YansErrorRateModel");

    // wifiPhy.SetErrorRateModel("ns3::NistErrorRateModel"); // just to make sure NistError is selected
    // ns-3 supports RadioTap and Prism tracing extensions for 802.11

    wifiPhy.Set("RxGain", DoubleValue(defaultValues["antennaGain"].get<double>()));   // dBi
    wifiPhy.Set("TxGain", DoubleValue(defaultValues["antennaGain"].get<double>()));   // dBi
    wifiPhy.Set("TxPowerStart", DoubleValue(defaultValues["txpower"].get<double>())); // dBm = 100mW
    wifiPhy.Set("TxPowerEnd", DoubleValue(defaultValues["txpower"].get<double>()));   // dBm = 100mW

    int nLoS = NumOfLoS(config);

    Ptr<YansWifiChannel> wifiChannel{};

    if (nLoS) //  have LOS
    {
      wifiChannel = wifiChannelLoS(config);
    }
    else // none LoS
    {
      wifiChannel = wifiChannelNLoS(config);
    }

    wifiPhy.SetChannel(wifiChannel);

    NS_LOG_INFO("INFO: Configuring WifiPhy... Ok!");
    return wifiPhy;
  }

  WifiMacHelper
  configWifiMac(json config)
  {

    NS_LOG_INFO("INFO: Configuring WifiMac...");
    WifiMacHelper wifiMac;

    Ssid ssid = Ssid("UEUAV");
    wifiMac.SetType("ns3::AdhocWifiMac", "QosSupported", BooleanValue(true), "Ssid", SsidValue(ssid));

    NS_LOG_INFO("INFO: Configuring WifiMac... Ok!");
    return wifiMac;
  }

} // namespace ns3