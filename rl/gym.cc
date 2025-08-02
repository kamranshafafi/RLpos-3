/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 Technische Universität Berlin
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Piotr Gawlowicz <gawlowicz@tkn.tu-berlin.de>
 */

#include "gym.h"
#include "../lib/json.hpp"
#include "../lib/simlogs.h"
#include "utilgym.h"
#include "math.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/log.h"
#include "ns3/minstrel-ht-wifi-manager.h"
#include "ns3/node-list.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/random-variable-stream.h"
#include "ns3/seq-ts-size-header.h"
#include "ns3/simulator.h"
#include "ns3/wifi-mac.h"
#include "ns3/wifi-module.h"
#include "ns3/wifi-phy.h"
#include "ns3/packet-sink.h"
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <regex>

float throughput_rx = 0, rxByteCounter_gym = 0, oldRxByteCounter_gym = 0, nLoS = 0, nUE = 0, sumDatarate = 0;

namespace ns3
{

  using json = nlohmann::json;
  using namespace std;
  NS_LOG_COMPONENT_DEFINE("MyGymEnv");
  NS_OBJECT_ENSURE_REGISTERED(MyGymEnv);

  MyGymEnv::MyGymEnv(json config, std::string path)
  {
    configLogs(config, "MyGymEnv");
    configLogs(config, "utilgym");
    NS_LOG_INFO("INFO: Constructing MyGymEnv...");

    this->config = config;
    this->decision_interval = Seconds(config["Gym"]["decisionInterval"].get<double>());
    // this->Now = Simulator::Now();
    Simulator::Schedule(Seconds(2.1), &MyGymEnv::ScheduleNextStateRead, this);
    this->obShape = {
        5,
    };

    throughput_rxlog.open(path + "/throughput_rx.csv", std::ios_base::out | std::ios_base::app);
    rewardlog.open(path + "/rewards.csv",
                   std::fstream::in | std::fstream::out | std::fstream::app);
    actionslog.open(path + "/action.csv",
                    std::ios_base::out |
                        std::ios_base::app);
    observationlog.open(path + "/observation.csv",
                        std::ios_base::out |
                            std::ios_base::app);
    // if (throughput_rxlog.is_open())
    // {
    //   NS_LOG_INFO(" File 'throughput_rx.csv' opened successfully..");
    //   Simulator::Schedule(Seconds(2.1), &MyGymEnv::ThroughputMonitor, this);
    // }
    // else
    // {
    //   // Handle error, print a message, or throw an exception
    //   std::cerr << "Error opening throughput_rxlog file!" << std::endl;
    //   std::cout << "Path for throughput_rxlog: " << path + "throughput_rx.csv" << std::endl;
    // }

    Config::ConnectWithoutContext("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx",
                                  MakeCallback(&MyGymEnv::ReceivePacket_RX, this));

    NS_LOG_INFO("INFO: Constructing MyGymEnv... Ok!");
  }

  void
  MyGymEnv::ScheduleNextStateRead()
  {
    NS_LOG_FUNCTION(this);
    Simulator::Schedule(decision_interval, &MyGymEnv::ScheduleNextStateRead, this);
    Notify();
  }

  Ptr<OpenGymSpace>
  MyGymEnv::GetActionSpace()
  {
    NS_LOG_INFO("INFO: Defining Action Space...");
    uint32_t numMovements = config["ns3Sim"]["configSim"]["UAV_directions"].size();
    Ptr<OpenGymDiscreteSpace> actionSpace = CreateObject<OpenGymDiscreteSpace>(numMovements);
    NS_LOG_INFO("INFO: Defining Action Space... Ok!");
    return actionSpace;
  }

  Ptr<OpenGymSpace>
  MyGymEnv::GetObservationSpace()
  {

    NS_LOG_INFO("INFO: Defining Observation Space...");
    Ptr<OpenGymBoxSpace> observationSpace = CreateObject<OpenGymBoxSpace>(-50.0, 100.0, this->obShape, TypeNameGet<float>());
    NS_LOG_INFO("INFO: Defining Observation Space... Ok!");
    return observationSpace;
  }

  bool
  MyGymEnv::ExecuteActions(Ptr<OpenGymDataContainer> action)
  {
    Now = Simulator::Now();
    NS_LOG_INFO("INFO: MyGymEnv::ExecuteActions start at...." << Now.GetSeconds());
    NS_LOG_INFO("DEBUG: Action ID: " << action);
    ApplyAction(this->config, action, actionslog);
    json defaultValuesUE = config["ns3Sim"]["defaultValues"]["UENodes"];
    nUE = defaultValuesUE.size();
    std ::cout << "Number of UE: " << nUE << std::endl;
    sumDatarate = 0;
    for (int j = 0; j < nUE; ++j)
    {
      double dataRate = std::stod(defaultValuesUE[j]["DataRate"].get<std::string>()); // Convert string to double
      sumDatarate += dataRate;                                                        // Add to sumDatarate
    }
    std ::cout << "Sum of datarate: " << sumDatarate << std::endl;
    NS_LOG_INFO("INFO: MyGymEnv::ExecuteActions start ....ok!");
    return true;
  }

  Ptr<OpenGymDataContainer>
  MyGymEnv::GetObservation()
  {
    Now = Simulator::Now();
    NS_LOG_INFO("INFO: MyGymEnv::GetObservation() start at...." << Now.GetSeconds());
    // Obtain the UAV's current 3D position
    Vector UAVPosition = currentUAVPositions();

    float x = UAVPosition.x;
    float y = UAVPosition.y;
    float z = UAVPosition.z;
    // float throughput = CalculateThroughput(sink);
    // float nLoS = NumOfLoS(config);

    NS_LOG_INFO("The new position of UAV is:"
                << "  " << x << "," << y << ","
                << z);

    // Create a container for your discrete observation space
    Ptr<OpenGymBoxContainer<float>> observation = CreateObject<OpenGymBoxContainer<float>>(obShape);
    nLoS = static_cast<float>(NumOfLoS(config));
    double frequency = decision_interval.GetSeconds();

    throughput_rx =
        ((rxByteCounter_gym - oldRxByteCounter_gym) * 8) / (1e6 * frequency);
    oldRxByteCounter_gym = rxByteCounter_gym;

    throughput_rxlog
        << Now.GetSeconds() << "," << throughput_rx << "," << rxByteCounter_gym << "," << oldRxByteCounter_gym << "," << nLoS << "," << x << "; " << y << "; " << z << std::endl;

    NS_LOG_INFO("THROUGHPUT MONITOR ----> SimTime: " << Now << " | Throughput RX: " << throughput_rx << " (Mbps) and position of UAV is: " << x << "; " << y << "; " << z); // Mbit/<0.1s> is only valid if frequency=0.1. If frequency=1 -> Mbit/s
    // Add the position vector to the observation
    observation->AddValue(x);
    observation->AddValue(y);
    observation->AddValue(z);
    observation->AddValue(nLoS);
    observation->AddValue(throughput_rx / sumDatarate);
    Now = Simulator::Now();
    observationlog << Now.GetSeconds() << "," << x << "  ; " << y << "  ; " << z << "," << nLoS << "," << throughput_rx
                   << std::endl;

    NS_LOG_INFO("INFO: MyGymEnv::GetObservation() start ....ok! and the observation is: " << x << "; " << y << "; " << z << " nLoS: " << nLoS << " and Throughput: " << throughput_rx);
    return observation;
  }

  float
  MyGymEnv::GetReward()
  {
    Now = Simulator::Now();
    NS_LOG_INFO("Reward computation start at " << Now.GetSeconds());

    float RewnLoS = nLoS / nUE;
    float RewThroughput = throughput_rx / (sumDatarate);
    float reward = (0.8 * RewnLoS) + (0.2 * RewThroughput);

    rewardlog << Now.GetSeconds() << "," << reward << "," << nLoS << std::endl;

    NS_LOG_INFO("DEBUG: Reward value: " << reward << "\n");
    return reward;
  }

  void MyGymEnv::ReceivePacket_RX(Ptr<const Packet> packet, const Address &)
  {
    rxByteCounter_gym += packet->GetSize();
  }

  // void
  // MyGymEnv::ThroughputMonitor()
  // {
  //   double frequency = decision_interval.GetSeconds();
  //   throughput_rx =
  //       ((rxByteCounter_gym - oldRxByteCounter_gym) * 8) / (1e6 * frequency);
  //   Now = Simulator::Now();
  //   NodeContainer nodes = NodeContainer::GetGlobal();
  //   Ptr<Node> uavNode = nodes.Get(0);
  //   Ptr<MobilityModel> mobility = uavNode->GetObject<MobilityModel>();
  //   throughput_rxlog
  //       << Now.GetSeconds() << "     ; " << throughput_rx << "     ; " << rxByteCounter_gym << "     ; " << oldRxByteCounter_gym << "     with nLoS= " << nLoS << "   ;   " << mobility->GetPosition() << std::endl;

  //   NS_LOG_INFO("THROUGHPUT MONITOR ----> SimTime: " << Now << " | Throughput RX: " << throughput_rx << " (Mbps) ans position of UAV is: " << mobility->GetPosition()); // Mbit/<0.1s> is only valid if frequency=0.1. If frequency=1 -> Mbit/s

  //   oldRxByteCounter_gym = rxByteCounter_gym;
  //   Simulator::Schedule(Seconds(frequency), &MyGymEnv::ThroughputMonitor, this);
  // }

  bool
  MyGymEnv::GetGameOver()
  {
    ResetAttributes();
    return CheckGameOver(this->config);
  }

  void
  MyGymEnv::ResetAttributes()
  {
    // throughput_rx = 0;
    // rxByteCounter_gym = 0;
    // oldRxByteCounter_gym = 0;
  }

  std::string
  MyGymEnv::GetExtraInfo()
  { // Define extra info. Optional
    std::string myInfo = "testInfo";
    return myInfo;
  }

  TypeId
  MyGymEnv::GetTypeId(void)
  {
    static TypeId tid = TypeId("MyGymEnv")
                            .SetParent<OpenGymEnv>()
                            .SetGroupName("OpenGym")
                            .AddConstructor<MyGymEnv>();
    return tid;
  }
  void
  MyGymEnv::DoDispose()
  {
  }
  MyGymEnv::~MyGymEnv()
  {
  } // destructor
  MyGymEnv::MyGymEnv()
  {
  } // og constructor

} // namespace ns3
