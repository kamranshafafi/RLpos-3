/*
###############################################
# Author:                                     #
# Kamran Shafafi[kamran.shafafi@inesctec.pt] #
###############################################
*/

#include <math.h>
#include <cmath>
#include <string>
#include <map>
#include "myutils.h"
#include "simlogs.h"
#include "json.hpp"
#include "ns3/node.h"
#include <ns3/log.h>
#include "ns3/core-module.h"

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("myutils");
  using json = nlohmann::json;

  uint64_t determineWifiChannelFrequencyHz(json config)
  {
    json configSim = config["ns3Sim"]["configSim"];
    std::string wifiStandard = configSim["wifiStandard"].get<std::string>();

    double wifiChannelNumber = config["ns3Sim"]["defaultValues"]["wifiChannelNumber"].get<double>();

    if (wifiStandard == "WIFI_STANDARD_80211b" || wifiStandard == "WIFI_STANDARD_80211g" ||
        wifiStandard == "WIFI_STANDARD_80211n")
      return ((2407 + 5 * wifiChannelNumber) * (1e6));
    else
      return ((5e3 + 5 * wifiChannelNumber) * (1e6));
  }

  json configCMD(int argc, char **argv, json config)
  {
    // TODO Clean deprecated parameters
    NS_LOG_INFO("INFO: Configuring CLI...");

    double openGymPort = config["ns3Sim"]["defaultValues"]["openGymPort"].get<double>();
    double simSeed = config["ns3Sim"]["defaultValues"]["simSeed"].get<double>();
    double simRun = config["ns3Sim"]["defaultValues"]["simRun"].get<double>();
    std::string trafficProtocol = config["ns3Sim"]["configSim"]["trafficProtocol"].get<std::string>();
    std::string path = config["ns3Sim"]["configSim"]["path"].get<std::string>();
    bool enableGym = config["ns3Sim"]["configSim"]["enableGym"].get<bool>();
    std::string simType = config["Agent"]["simType"].get<std::string>();

    CommandLine cmd; // EVERY PARAMETER CAN BE OVERWRITTEN BY PASSING IT AS ARGUMENT
    cmd.AddValue("openGymPort", "Port number for OpenGym env. Default: 5555", openGymPort);
    cmd.AddValue("simSeed", "Seed for random generator. Default: 1", simSeed);
    cmd.AddValue("simRun", "Run for random generator. Default: 1", simRun);
    cmd.AddValue("trafficProtocol", "tcp for TCP, udp for UDP", trafficProtocol);
    cmd.AddValue("path", "File path to save the logs", path);         // null in .json
    cmd.AddValue("enableGym", "Bool that enables ns3gym", enableGym); // default false... but if sim starts from agent.py it is true.
    cmd.AddValue("simType", "eval for Evaluation and train for Training", simType);
    cmd.Parse(argc, argv);

    config["ns3Sim"]["defaultValues"]["openGymPort"] = openGymPort;
    config["ns3Sim"]["defaultValues"]["simSeed"] = simSeed;
    config["ns3Sim"]["defaultValues"]["simRun"] = simRun;
    config["ns3Sim"]["configSim"]["trafficProtocol"] = trafficProtocol;
    config["ns3Sim"]["configSim"]["path"] = path;
    config["ns3Sim"]["configSim"]["enableGym"] = enableGym;
    config["Agent"]["simType"] = simType;

    NS_LOG_INFO("INFO: Configuring CLI... Ok!");
    return config;
  }

  json SetupJsonConfig()
  {
    json jsonConfig;
    std::ifstream i;
    i.open("scratch/fw-phd/config/input.json");
    if (i)
      i >> jsonConfig;
    else
    {
      std::cout << "Error: Config file could not be opened! \n";
      exit(1);
    }

    return jsonConfig;
  }

} // namespace ns3
