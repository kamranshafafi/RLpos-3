/*
###############################################
# Author:                                     #
# Ruben Queiros [ruben.m.queiros@inesctec.pt] #
###############################################
*/
#pragma once

#include <string>
#include "json.hpp"
#include <ns3/wifi-module.h>
#include <ns3/core-module.h>

namespace ns3
{

  using json = nlohmann::json;

  /**
   * @brief Configures the logging levels based on the values in a JSON config file.
   * @param config - the json object that contains the loglevel value.
   * @param component - The component to log.
   */
  void configLogs(json config, const char *component);

} // namespace ns3