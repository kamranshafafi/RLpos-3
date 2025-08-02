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

#ifndef MY_GYM_ENTITY_H
#define MY_GYM_ENTITY_H

#include "gym.h"
#include "../lib/json.hpp"
#include "ns3/core-module.h"
#include "ns3/log.h"
#include "ns3/node-list.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/opengym-module.h"
#include "ns3/wifi-module.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include "ns3/packet-sink.h"

namespace ns3
{

  using json = nlohmann::json;
  class MyGymEnv : public OpenGymEnv
  {
  public:
    MyGymEnv(json jsonFile, std::string path);
    MyGymEnv();
    virtual ~MyGymEnv();
    static TypeId GetTypeId(void);
    virtual void DoDispose();

    Ptr<OpenGymSpace> GetActionSpace();
    Ptr<OpenGymSpace> GetObservationSpace();
    bool ExecuteActions(Ptr<OpenGymDataContainer> action);
    Ptr<OpenGymDataContainer> GetObservation();
    float GetReward();
    bool GetGameOver();
    std::string GetExtraInfo();

  private:
    void ScheduleNextStateRead();
    void ResetAttributes();
    // void ThroughputMonitor();
    void ReceivePacket_RX(Ptr<const Packet> packet, const Address &);

    json config;
    Time decision_interval;
    Ptr<PacketSink> sink{};
    std::vector<uint32_t> obShape;
    Time Now;
    float nLoS;
    std::ofstream throughput_rxlog;
    std::ofstream observationlog;
    std::ofstream rewardlog;
    std::ofstream actionslog;
  };

} // namespace ns3

#endif // MY_GYM_ENTITY_H