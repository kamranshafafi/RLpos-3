/*
###############################################
# Author:                                     #
# kamran shafafi [kamran.shafafi@inesctec.pt] #
###############################################
*/

#pragma once
#include "gym.h"
#include "../lib/json.hpp"
#include <ns3/opengym-module.h>
#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include "ns3/packet-sink.h"
#include <ns3/wifi-module.h>
#include <ns3/itu-r-1411-los-propagation-loss-model.h>
#include <ns3/itu-r-1411-nlos-over-rooftop-propagation-loss-model.h>
#include <ns3/hybrid-buildings-propagation-loss-model.h>

namespace ns3
{
    using json = nlohmann::json;

    // Define a structure to represent a sphere
    struct Sphere
    {
        double x, y, z; // Center coordinates
        double dmax;    // Sphere's dmax
        double SNR;
    };

    // Function to calculate dmax from SNR, PT, f, and PN
    double CalculateDmax(double SNR, double PT, double f, double PN);
    // Function to check if a coordinate is inside the intersection of spheres
    bool IsCoordinateInIntersection(const std::vector<Sphere> &spheres, int x, int y, int z);
    void SetTxPower(int nodeID, double value);

    /**
     * @brief computing the number of LOS.ssss
     * @param config the json object that contains config values.
     **/

    std::vector<std::string> ListOfLoS(json config);
    int NumOfLoS(json config);
    std::vector<double> NLoSLoss(json config);
    std::vector<double> LoSLoss(json config);
    Ptr<YansWifiChannel> wifiChannelLoS(json config);
    Ptr<YansWifiChannel> wifiChannelNLoS(json config);
    Ptr<ItuR1411LosPropagationLossModel> lossModelLoS(json config);
    Ptr<ItuR1411NlosOverRooftopPropagationLossModel> lossModelNLoS(json config);
    Ptr<HybridBuildingsPropagationLossModel> lossModelLoShybrid(json config);

    /**
     * @brief Apply the received Action in ns-3, depending on Gym Version
     * @param config JSON config file.
     * @param action Action received from the agent and passed by Gym.
     */
    void ApplyAction(json config, Ptr<OpenGymDataContainer> action, std::ofstream &actionslog);

    /**
     * @brief Calculates the new position of UAV.
     * @return Vector new Position of UAV.
     */
    Vector currentUAVPositions();

    /**
     * @brief Checks if episode has ended, with the game over condition, depending on Gym Version
     * @param config JSON config file.
     */
    bool CheckGameOver(json config);

    /**
     * @brief Defines specific Discrete Action Spaces depending on Gym Version.
     * @param config JSON config file.
     * @return the number of discrete actions
     */
    uint32_t DefineDiscreteActionSpace(json config);
    uint32_t DefineDiscreteObservationSpace(json config);
    void CalculateThroughput(float rxByteCounter);
    // void ReceivePacket();

} // namespace ns3s