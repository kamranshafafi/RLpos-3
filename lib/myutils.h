/*
###############################################
# Author:                                     #
# Kamran Shafafi [Kamran.shafafi@inesctec.pt] #
###############################################
*/
#pragma once

#include "json.hpp"
#include <ns3/core-module.h>
#include <ns3/network-module.h>

namespace ns3
{

    using json = nlohmann::json;

    /**
     * @brief Configures the command line interface based on the values in a JSON file
     *
     * @param argc Number of arguments
     * @param argv Argument values
     * @param config the json object  that contains the initial config values
     * @return the json object that contains the updated config values
     */
    json configCMD(int argc, char **argv, json config);
    uint64_t determineWifiChannelFrequencyHz(json config);

    /**
     * @brief Opens and returns the json object of the /fw-phd/config/config.json file.
     * @return the json object.
     */
    json SetupJsonConfig();

} // namespace ns3