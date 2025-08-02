/*
###############################################
# Author:                                     #
# Kamran Shafafi [kamran.shafafi@inesctec.pt] #
###############################################
*/

#include "../lib/json.hpp"
#include "../lib/myutils.h"
#include "../lib/simconf.h"
#include "utilgym.h"
#include "gym.h"
#include <ns3/mobility-module.h>
#include <ns3/network-module.h>
#include <ns3/wifi-module.h>
#include <ns3/mobility-module.h>
#include "ns3/packet-sink.h"
#include <ns3/log.h>
#include <ns3/core-module.h>
#include <ns3/node-container.h>
#include "ns3/network-module.h"
#include <ns3/buildings-module.h>
#include <ns3/mobility-building-info.h>
#include <ns3/buildings-helper.h>
#include <ns3/buildings-channel-condition-model.h>
#include <ns3/itu-r-1411-los-propagation-loss-model.h>
#include <ns3/itu-r-1411-nlos-over-rooftop-propagation-loss-model.h>
#include <ns3/hybrid-buildings-propagation-loss-model.h>

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("utilgym");
  using json = nlohmann::json;

  // Function to calculate dmax from SNR, PT, f, and PN
  double
  CalculateDmax(double SNR, double PT, double f, double PN)
  {
    double dmax = std::pow(10, (PT - SNR - 20 * std::log10(f) - 20 * std::log10(4 * M_PI / 299792458.0) - PN) / 20);
    return dmax;
  }

  // Function to check if a coordinate is inside the intersection of spheres
  bool
  IsCoordinateInIntersection(const std::vector<Sphere> &spheres, int x, int y, int z)
  {
    // Iterate through spheres and check if the coordinate is inside each sphere
    for (const Sphere &sphere : spheres)
    {
      double distance = std::sqrt(std::pow(x - sphere.x, 2) + std::pow(y - sphere.y, 2) + std::pow(z - sphere.z, 2));
      if (distance > sphere.dmax)
      {
        return false; // Outside this sphere
      }
    }
    return true; // Inside all spheres
  }

  void
  SetTxPower(int nodeID, double value)
  {
    std::ostringstream oss;
    oss << "/NodeList/" << nodeID << "/DeviceList/*/$ns3::WifiNetDevice/Phy/TxPowerStart";
    Config::Set(oss.str(), DoubleValue(value));

    oss.str("");
    oss.clear();

    oss << "/NodeList/" << nodeID << "/DeviceList/*/$ns3::WifiNetDevice/Phy/TxPowerEnd";
    Config::Set(oss.str(), DoubleValue(value));
  }

  void ApplyAction(json config, Ptr<OpenGymDataContainer> action, std::ofstream &actionslog)
  {
    NS_LOG_DEBUG("Update the position of UAV and environment...");

    Ptr<OpenGymDiscreteContainer>
        discrete = DynamicCast<OpenGymDiscreteContainer>(action);
    json configVenue = config["Gym"]["venueSize"];
    int Xmin = configVenue["Xmin"].get<int>();
    int Ymin = configVenue["Ymin"].get<int>();
    int Zmin = configVenue["Zmin"].get<int>();
    int Xmax = configVenue["Xmax"].get<int>();
    int Ymax = configVenue["Ymax"].get<int>();
    int Zmax = configVenue["Zmax"].get<int>();
    int gridResolution = configVenue["gridResolution"].get<int>();

    double PN = config["ns3Sim"]["defaultValues"]["PN"].get<double>();
    uint64_t frequency = determineWifiChannelFrequencyHz(config);
    json defaultValues = config["ns3Sim"]["defaultValues"];
    NodeContainer nodes = NodeContainer::GetGlobal();

    // Extract sphere data from JSON
    std::vector<Sphere> spheres;
    int numSpheres = config["ns3Sim"]["defaultValues"]["UENodes"].size(); // Get the number of spheres
    double PT = config["ns3Sim"]["defaultValues"]["txpower"].get<double>();
    int xpos{}, ypos{}, zpos{};

    for (int i = 0; i < numSpheres; ++i)
    {
      Sphere sphere;
      sphere.x = config["ns3Sim"]["defaultValues"]["UENodes"][i]["positionVector"][0];
      sphere.y = config["ns3Sim"]["defaultValues"]["UENodes"][i]["positionVector"][1];
      sphere.z = config["ns3Sim"]["defaultValues"]["UENodes"][i]["positionVector"][2];
      sphere.SNR = config["ns3Sim"]["defaultValues"]["UENodes"][i]["SNR"]; // SNR for this sphere

      // Add the sphere to the vector
      spheres.push_back(sphere);
    }
    for (auto &s : spheres)
    {
      // Calculate dmax for this sphere
      s.dmax = CalculateDmax(s.SNR, PT, frequency, PN);
    }

    do
    {
      /* code */
      // Extract the discrete action value, which represents a grid position.
      uint32_t movementTypeID = discrete->GetValue();
      std::vector<std::string> movements = config["ns3Sim"]["configSim"]["UAV_directions"];
      std::string posUpdate = movements.at(movementTypeID);

      Ptr<MobilityModel> mobility = nodes.Get(0)->GetObject<MobilityModel>();
      Vector pos = mobility->GetPosition();
      double now = Simulator::Now().GetSeconds();
      actionslog << now << "," << posUpdate
                 << std::endl;

      xpos = pos.x;
      ypos = pos.y;
      zpos = pos.z;

      if (posUpdate == "left")
      {
        if (ypos == Ymin)
          nodes.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(xpos, ypos, zpos));
        else
        {
          ypos = ypos - gridResolution;
          nodes.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(xpos, ypos, zpos));
        }
      }
      else if (posUpdate == "right")
      {
        if (ypos == Ymax)
          nodes.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(xpos, ypos, zpos));
        else
        {
          ypos = ypos + gridResolution;
          nodes.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(xpos, ypos, zpos));
        }
      }
      if (posUpdate == "back")
      {
        if (xpos == Xmin)
          nodes.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(xpos, ypos, zpos));
        else
        {
          xpos = xpos - gridResolution;
          nodes.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(xpos, ypos, zpos));
        }
      }
      else if (posUpdate == "front")
      {
        if (xpos == Xmax)
          nodes.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(xpos, ypos, zpos));
        else
        {
          xpos = xpos + gridResolution;
          nodes.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(xpos, ypos, zpos));
        }
      }

      if (posUpdate == "down")
      {
        if (zpos == Zmin)
          nodes.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(xpos, ypos, zpos));
        else
        {
          zpos = zpos - gridResolution;
          nodes.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(xpos, ypos, zpos));
        }
      }
      else if (posUpdate == "up")
      {
        if (zpos == Zmax)
          nodes.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(xpos, ypos, zpos));
        else
        {
          zpos = zpos + gridResolution;
          nodes.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(xpos, ypos, zpos));
        }
      }
      else if (posUpdate == "same")
        nodes.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(xpos, ypos, zpos));

      NS_LOG_INFO("Performing action and checking coordinates...");

    } while (!IsCoordinateInIntersection(spheres, xpos, ypos, zpos));

    std::cout << "The new position of UAV is: " << nodes.Get(0)->GetObject<MobilityModel>()->GetPosition() << std::endl;
    NS_LOG_INFO("Update the position of UAV and environment... Ok!");
  }

  void CalculateThroughput(float rxByteCounter)
  {
    NS_LOG_INFO("INFO: clculating the throughput...");
    // Calculate the throughput for the current interval
    float throughput{};
    if (rxByteCounter)
    {
      // float currentTotalRx = sink->GetTotalRx();
      // NS_LOG_INFO("INFO: clculating TotalRx is:" << rxByteCounter << "," << sink);
      throughput = rxByteCounter * 8.0 / (1e6 * 0.1);
      Time Now = Simulator::Now();
      NS_LOG_INFO("INFO: clculating the throughput...ok and the throughput is: " << throughput << "at" << Now);
    }
    NS_LOG_INFO("INFO: clculating the throughput...ok.......s and the throughput is: " << throughput);
    // return throughput;
  }

  Vector currentUAVPositions()
  {
    NS_LOG_INFO("INFO: current UAV Positions as observation...");
    // Retrieve the global node container
    NodeContainer nodes = NodeContainer::GetGlobal();

    // Check if there is at least one node in the simulation
    if (nodes.GetN() > 0)
    {
      // Get the first node (UAV) from the container
      Ptr<Node> uavNode = nodes.Get(0);

      // Check if the UAV node has a mobility model
      if (uavNode->GetObject<MobilityModel>())
      {
        // Retrieve the mobility model of the UAV
        Ptr<MobilityModel> uavMobility = uavNode->GetObject<MobilityModel>();

        // Get the current position of the UAV
        Vector uavPosition = uavMobility->GetPosition();
        NS_LOG_INFO("INFO: current UAV Positions as observation...ok!");
        return uavPosition;
      }
      else
      {
        NS_LOG_INFO("UAV node does not have a mobility model.");
      }
    }
    else
    {
      NS_LOG_INFO("No nodes in the simulation.");
    }

    // Return a default position (0, 0, 0) if there was an error
    return Vector(0.0, 0.0, 0.0);
  }

  std::vector<std::string> ListOfLoS(json config)
  {
    NS_LOG_INFO("INFO: checking the List of LoS...");

    Ptr<BuildingsChannelConditionModel> condModel = CreateObject<BuildingsChannelConditionModel>();

    json defaultValuesUAV = config["ns3Sim"]["defaultValues"]["UAVNodes"];
    json defaultValuesUE = config["ns3Sim"]["defaultValues"]["UENodes"];
    NodeContainer nodes = NodeContainer::GetGlobal();
    std::vector<std::string> LoSList;
    Ptr<ChannelCondition> cond{};

    // uint32_t nLos{};
    Ptr<MobilityModel> ap{};
    Ptr<MobilityModel> ue{};
    for (std::size_t i = 0; i < defaultValuesUAV.size(); ++i)
    {
      ap = nodes.Get(i)->GetObject<ConstantPositionMobilityModel>(
          ConstantPositionMobilityModel::GetTypeId());
      for (std::size_t j = 0; j < defaultValuesUE.size(); ++j)
      {
        ue = nodes.Get(j + 1)->GetObject<ConstantPositionMobilityModel>(
            ConstantPositionMobilityModel::GetTypeId());
        cond = condModel->GetChannelCondition(ap, ue);
        std::cout << "Channel between the AP # " << i << " and the UE #" << j
                  << " is: " << cond->GetLosCondition() << std::endl;
        LoSList.push_back(std::to_string(!cond->GetLosCondition()));
      }
    }
    NS_LOG_INFO("INFO: checking the List of LoS...Ok!");
    return LoSList;
  }

  int NumOfLoS(json config)
  {
    NS_LOG_INFO("INFO: checking the Number of LoS...");
    uint32_t nLos{};
    std::vector<std::string> LoSList = ListOfLoS(config);

    for (const auto &elem : LoSList)
    {
      if (elem == "1")
      {
        ++nLos;
      }
    }
    std::cout << "the number of LoS = " << nLos << std::endl;
    NS_LOG_INFO("INFO: checking the Number of LoS...ok!");
    return nLos;
  }
  std::vector<double> NLoSLoss(json config)
  {
    NS_LOG_INFO("INFO: calculationg the NLoS psth loss for all links...");
    json defaultValuesUAV = config["ns3Sim"]["defaultValues"]["UAVNodes"];
    json defaultValuesUE = config["ns3Sim"]["defaultValues"]["UENodes"];
    NodeContainer nodes = NodeContainer::GetGlobal();
    Ptr<MobilityModel> ap{};
    Ptr<MobilityModel> ue{};

    Ptr<ItuR1411NlosOverRooftopPropagationLossModel> lossModel2 = lossModelNLoS(config);
    std::vector<double> NLoSLoss;
    for (std::size_t i = 0; i < defaultValuesUAV.size(); ++i)
    {
      ap = nodes.Get(i)->GetObject<ConstantPositionMobilityModel>(
          ConstantPositionMobilityModel::GetTypeId());
      for (std::size_t j = 0; j < defaultValuesUE.size(); ++j)
      {
        ue = nodes.Get(j + 1)->GetObject<ConstantPositionMobilityModel>(
            ConstantPositionMobilityModel::GetTypeId());
        double loss = lossModel2->GetLoss(ap, ue);

        NLoSLoss.push_back(loss);
        std::cout << "In NLoS Conditions Loss(AP, UE#" << j << "): " << loss << std::endl;
      }
    }
    NS_LOG_INFO("INFO: calculationg the NLoS psth loss for all links...Ok!");
    return NLoSLoss;
  }

  std::vector<double> LoSLoss(json config)
  {
    NS_LOG_INFO("INFO: calculationg the LoS psth loss for all links...");
    json defaultValuesUAV = config["ns3Sim"]["defaultValues"]["UAVNodes"];
    json defaultValuesUE = config["ns3Sim"]["defaultValues"]["UENodes"];
    NodeContainer nodes = NodeContainer::GetGlobal();
    Ptr<MobilityModel> ap{};
    Ptr<MobilityModel> ue{};

    Ptr<ItuR1411LosPropagationLossModel> lossModel1 = lossModelLoS(config);
    std::vector<double> LoSLoss;
    for (std::size_t i = 0; i < defaultValuesUAV.size(); ++i)
    {
      ap = nodes.Get(i)->GetObject<ConstantPositionMobilityModel>(
          ConstantPositionMobilityModel::GetTypeId());
      for (std::size_t j = 0; j < defaultValuesUE.size(); ++j)
      {
        ue = nodes.Get(j + 1)->GetObject<ConstantPositionMobilityModel>(
            ConstantPositionMobilityModel::GetTypeId());
        double loss = lossModel1->GetLoss(ap, ue);
        LoSLoss.push_back(loss);
        std::cout << "In LoS Condition Loss(AP, UE#" << j << "):" << loss << std::endl;
      }
    }
    NS_LOG_INFO("INFO: calculationg the LoS psth loss for all links...Ok!");
    return LoSLoss;
  }

  Ptr<ItuR1411LosPropagationLossModel> lossModelLoS(json config)
  {
    // NS_LOG_INFO("INFO: LoS propagation loss model...");
    uint64_t frequency = determineWifiChannelFrequencyHz(config);
    Ptr<ItuR1411LosPropagationLossModel> lossModelLoS{CreateObject<ItuR1411LosPropagationLossModel>()};

    lossModelLoS->SetAttribute("Frequency", DoubleValue(frequency));
    // NS_LOG_INFO("INFO: LoS propagation loss model...Ok and Propagation Loss Model is: ItuR1411LosPropagationLossModel !");
    return lossModelLoS;
  }
  Ptr<HybridBuildingsPropagationLossModel> lossModelLoShybrid(json config)
  {
    // NS_LOG_INFO("INFO: LoS propagation loss model...");
    uint64_t frequency = determineWifiChannelFrequencyHz(config);
    Ptr<HybridBuildingsPropagationLossModel> lossModelLoShybrid{CreateObject<HybridBuildingsPropagationLossModel>()};

    lossModelLoShybrid->SetAttribute("Frequency", DoubleValue(frequency));
    lossModelLoShybrid->SetAttribute("CitySize", StringValue("Small"));
    lossModelLoShybrid->SetAttribute("Environment", StringValue("SubUrban"));

    // NS_LOG_INFO("INFO: LoS propagation loss model...Ok and Propagation Loss Model is: HybridBuildingsPropagationLossModel !");
    return lossModelLoShybrid;
  }

  Ptr<YansWifiChannel>
  wifiChannelLoS(json config)
  {
    NS_LOG_INFO("INFO: LoS wifi channel...");
    Ptr<YansWifiChannel> wifiChannel = CreateObject<YansWifiChannel>();
    Ptr<ConstantSpeedPropagationDelayModel> DelayLossModel =
        CreateObject<ConstantSpeedPropagationDelayModel>();
    wifiChannel->SetPropagationDelayModel(DelayLossModel);

    wifiChannel->SetPropagationLossModel(lossModelLoShybrid(config));
    std::clog << "Propagation Loss Model is:" << (typeid(*lossModelLoShybrid).name()) << std::endl;
    NS_LOG_INFO("INFO: LoS wifi channel...Ok!");
    return wifiChannel;
  }

  Ptr<ItuR1411NlosOverRooftopPropagationLossModel> lossModelNLoS(json config)
  {
    // NS_LOG_INFO("INFO: NLoS propagation loss model...");
    uint64_t frequency = determineWifiChannelFrequencyHz(config);
    Ptr<ItuR1411NlosOverRooftopPropagationLossModel> lossModelNLoS{CreateObject<ItuR1411NlosOverRooftopPropagationLossModel>()};
    lossModelNLoS->SetAttribute("Frequency", DoubleValue(frequency));
    lossModelNLoS->SetAttribute("RooftopLevel", DoubleValue(20));
    lossModelNLoS->SetAttribute("Environment", StringValue("SubUrban"));
    lossModelNLoS->SetAttribute("StreetsOrientation", DoubleValue(45));
    lossModelNLoS->SetAttribute("CitySize", StringValue("Small"));
    lossModelNLoS->SetAttribute("BuildingSeparation", DoubleValue(100));
    lossModelNLoS->SetAttribute("BuildingsExtend", DoubleValue(10));
    lossModelNLoS->SetAttribute("StreetsWidth", DoubleValue(200));
    // NS_LOG_INFO("INFO: NLoS propagation loss model...Ok and Propagation Loss Model is: ItuR1411NlosOverRooftopPropagationLossModel !");
    return lossModelNLoS;
  }

  Ptr<YansWifiChannel> wifiChannelNLoS(json config)
  {
    NS_LOG_INFO("INFO: NLoS wifi channel...");
    Ptr<YansWifiChannel> wifiChannel = CreateObject<YansWifiChannel>();
    Ptr<ConstantSpeedPropagationDelayModel> DelayLossModel =
        CreateObject<ConstantSpeedPropagationDelayModel>();
    wifiChannel->SetPropagationDelayModel(DelayLossModel);

    wifiChannel->SetPropagationLossModel(lossModelNLoS(config));
    std::clog << "Propagation Loss Model is: ItuR1411NlosOverRooftopPropagationLossModel" << std::endl;
    NS_LOG_INFO("INFO: NLoS wifi channel...Ok!");
    return wifiChannel;
  }

  bool CheckGameOver(json config)
  {
    // Define game over condition
    bool isGameOver = false;
    // a static variable declared within a function scope will be created and
    //  initialized once for all non-recursive calls to that function.
    float gameover_steps = config["ns3Sim"]["defaultValues"]["simDuration"].get<double>() * 10 - 21;
    static float stepCounter = 0.0;
    stepCounter += 1;
    if (stepCounter >= gameover_steps)
    {
      isGameOver = true;
    }
    return isGameOver;
  }

  uint32_t
  DefineDiscreteActionSpace(json config)
  {

    NS_LOG_INFO("INFO: Defining Action Space...");
    json configVenue = config["Gym"]["venueSize"];

    // Define the number of actions based on the grid size
    int gridSizeX = (configVenue["Xmax"].get<int>() - configVenue["Xmin"].get<int>()) / configVenue["gridResolution"].get<int>();
    int gridSizeY = (configVenue["Ymax"].get<int>() - configVenue["Ymin"].get<int>()) / configVenue["gridResolution"].get<int>();
    int gridSizeZ = (configVenue["Zmax"].get<int>() - configVenue["Zmin"].get<int>()) / configVenue["gridResolution"].get<int>();

    // Calculate grid dimensions
    int numGridPointsX = gridSizeX + 1;
    int numGridPointsY = gridSizeY + 1;
    int numGridPointsZ = gridSizeZ + 1;

    uint32_t numActions = numGridPointsX * numGridPointsY * numGridPointsZ;

    NS_LOG_INFO("INFO: Defining Action Space... Ok!");
    return numActions;
  }

  uint32_t
  DefineDiscreteObservationSpace(json config)
  {
    NS_LOG_INFO("INFO: Defining Observation Space...");
    json configVenue = config["Gym"]["venueSize"];

    int gridSizeX = (configVenue["Xmax"].get<int>() - configVenue["Xmin"].get<int>()) / configVenue["gridResolution"].get<int>();
    int gridSizeY = (configVenue["Ymax"].get<int>() - configVenue["Ymin"].get<int>()) / configVenue["gridResolution"].get<int>();
    int gridSizeZ = (configVenue["Zmax"].get<int>() - configVenue["Zmin"].get<int>()) / configVenue["gridResolution"].get<int>();

    // Calculate grid dimensions
    int numGridPointsX = gridSizeX + 1;
    int numGridPointsY = gridSizeY + 1;
    int numGridPointsZ = gridSizeZ + 1;
    uint32_t numActions = numGridPointsX * numGridPointsY * numGridPointsZ;

    NS_LOG_INFO("INFO: Defining Observation Space... Ok!");
    return numActions;
  }

} // namespace ns3