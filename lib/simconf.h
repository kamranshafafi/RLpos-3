/*
###############################################
# Author:                                     #
# kamran shafafi [kamran.shafafi@inesctec.pt] #
###############################################
*/

#include "json.hpp"
#include <ns3/wifi-module.h>
#include <ns3/mobility-module.h>
#include <ns3/internet-module.h>
#include <ns3/building.h>
#include <vector>
#include <string>
#include "ns3/packet-sink.h"

namespace ns3
{

  using json = nlohmann::json;

  /**
   * @brief Configures the mobility model for each node throughout the simulation.
   * @param config the json object that contains config values.
   * @param nodes the node container to install the applications.
   * @param interfaces the ipv4 interface container to retrief the IPv4 addresses.
   */
  void configApps(json config, NodeContainer nodes, Time start, Time stop, Ipv4InterfaceContainer interfaces);

  /**
   * @brief Implementation of the Building in the scenario.
   * @param config the json object that contains config values.
   */
  std::vector<Ptr<Building>> BuildingImplement(json config);

  /**
   * @brief Configures the mobility model for each node throughout the simulation.
   * @param config the json object that contains config values.
   */
  void configUEMobility(json config);

  /**
   * @brief Configures the initial nodes position according to the json config file.
   * @warning this function uses for configure or install constante mobility model.
   * @param config the json object that contains config values.
   * @param nodes the node container to install the applications.
   * @param nodeType the type of node is it UE or UAV.
   * @return The configured mobilityhelper.
   */
  void configUAVPosition(json config, int xPosition, int yPosition, int zPosition);

  /**
   * @brief Configures WiFi parameters such as standard and Rate Adaptation Algorithm.
   * @param config the json object that contains config values.
   * @return The configured WifiHelper.
   */
  WifiHelper configWifi(json config);

  /**
   * @brief Configures multiple WiFi Phy parameters.
   * @param config the json object that contains config values.
   * @param freqMHz The configured channel frequency.
   * @return The configured YansWifiPhyHelper.
   */
  YansWifiPhyHelper configWifiPhy(json config);

  /**
   * @brief Configures AdHocWiFiMac default parameters and enables/disables Frame Aggregation.
   * @param config the json object that contains config values.
   * @return The configured WifiMacHelper.
   */
  WifiMacHelper configWifiMac(json config);

} // namespace ns3