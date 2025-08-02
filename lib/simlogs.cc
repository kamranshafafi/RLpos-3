/*
###############################################
# Author:                                     #
# Ruben Queiros [ruben.m.queiros@inesctec.pt] #
###############################################
*/

#include "simlogs.h"
#include "json.hpp"
#include <ns3/log.h>
#include <ns3/mobility-model.h>
#include <ns3/constant-velocity-mobility-model.h>
#include <ns3/node.h>
#include <ns3/node-container.h>
#include <ns3/vector.h>
#include "ns3/core-module.h"

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("simlogs");
  using json = nlohmann::json;

   void
  configLogs(json config, const char *component)
  {
    NS_LOG_INFO("INFO: Configuring logs for " << component << " ...");
    // TODO currently, it assumes same debug level for every component.
    json logLevels = config["ns3Sim"]["logLevels"];
    // LogComponentEnable("MinstrelHtWifiManager", LOG_LEVEL_DEBUG);
    if (logLevels["ERROR"].get<bool>())
      LogComponentEnable(component, LOG_ERROR);
    if (logLevels["WARN"].get<bool>())
      LogComponentEnable(component, LOG_WARN);
    if (logLevels["INFO"].get<bool>())
    {
      LogComponentEnable(component, LOG_INFO);
    }
    if (logLevels["DEBUG"].get<bool>())
      LogComponentEnable(component, LOG_DEBUG);

    NS_LOG_INFO("INFO: Configuring logs for " << component << " ... Ok!");
  }

} // namespace ns3