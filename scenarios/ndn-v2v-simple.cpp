#include "ns3/core-module.h"

#include "ns3/ns2-mobility-helper.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/ndnSIM/apps/ndn-producer.hpp"
#include "ns3/ndnSIM/apps/ndn-consumer-cbr.hpp"
#include "ns3/ndnSIM/apps/ndn-app.hpp"
#include "ns3/ndnSIM/helper/ndn-app-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include <ns3/ndnSIM/helper/ndn-global-routing-helper.hpp>
#include "ns3/animation-interface.h"

#include <algorithm>
#include <vector>
#include <iostream>
#include <list>
#include <iterator>


namespace ns3{
  NS_LOG_COMPONENT_DEFINE ("V2VSimple");

  int simulationEnd = 100;
  int producerCount = 0;
  int consumerCount = 0;
  int nodeCount = 0;
  std::string traceFile = "File path goes here...";
  std::string animFile = "ndn-v2v-test.xml";

  // For Debugging purposes
  void printPosition(Ptr<const MobilityModel> mobility) {
      Simulator::Schedule(Seconds(1), &printPosition, mobility);
      NS_LOG_INFO("Car "<<  mobility->GetObject<Node>()->GetId() << " is at: " <<mobility->GetPosition());
  }

  void installWave(NodeContainer &c, NetDeviceContainer &devices) {
      // Modulation and wifi channel bit rate
      std::string phyMode("OfdmRate24Mbps");
      // Fix non-unicast data rate to be the same as that of unicast
      Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));

      Wifi80211pHelper waveHelper = Wifi80211pHelper::Default();

      YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
      wifiPhy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

      YansWifiChannelHelper wifiChannel;
      wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
      wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel",
                                     "MaxRange", DoubleValue(19.0));
      wifiChannel.AddPropagationLoss("ns3::NakagamiPropagationLossModel",
                                     "m0", DoubleValue(1.0),
                                     "m1", DoubleValue(1.0),
                                     "m2", DoubleValue(1.0));
      wifiPhy.SetChannel(wifiChannel.Create());


      QosWaveMacHelper qosWaveMacHelper = QosWaveMacHelper::Default();

      // devices = wifi.Install(wifiPhy, wifiMac, c);
      devices = waveHelper.Install(wifiPhy, qosWaveMacHelper, c);
  }

  void installNDN(NodeContainer &c) {
      ndn::StackHelper ndnHelper;
      ndnHelper.SetDefaultRoutes(true);

      ndnHelper.Install(c);
      ndn::StrategyChoiceHelper::InstallAll("/", "/localhost/nfd/strategy/broadcast");
  }

  void installConsumer(NodeContainer &c) {
      ndn::AppHelper helper("ns3::ndn::ConsumerCbr");
      helper.SetAttribute("Frequency", DoubleValue (1.0));
      helper.SetAttribute("Randomize", StringValue("uniform"));
      helper.SetPrefix("/v2v/test");
      helper.Install(c);
  }

  void installProducer(NodeContainer &c) {
      ndn::AppHelper producerHelper("ns3::ndn::Producer");
      producerHelper.SetPrefix("/v2v");
      producerHelper.Install(c.Get(0));
      NS_LOG_INFO("Producer installed on node " << c.Get(0)->GetId());
  }

  void installProdConsHelpers(NodeContainer &c) {
      //TODO: Allow randomly alloced consumers as well as all nodes being both
      // Installs consumer and producer helpers on nodes where relevant
      Ptr<UniformRandomVariable> randomNum = CreateObject<UniformRandomVariable> ();
      NodeContainer producers;
      NodeContainer consumers;
      // std::list <int> producerIds;
      // int producerId;
      // // Randomly generated producer ids
      // for (int i = 0; i < producerCount; i++) {
      //   //gen rand num
      //   producerId = randomNum->GetValue(0, (nodeCount - 1));
      //   //check not in list already
      //   if ((producerIds.empty()) || (std::find(producerIds.begin(), producerIds.end(), producerId) != producerIds.end())) {
      //     producerIds.push_front(producerId);
      //   } else {
      //     i = i - 1;
      //   }
      // }
      // Producers and Consumers assigned based on the list of producerIds above
      for (int i = 0; i < nodeCount; i++) {
        // bool prod = (std::find(producerIds.begin(), producerIds.end(), i)) != (producerIds.end());
        // if(!prod) {
          consumers.Add(c.Get(i));
        // }
        // if (prod) {
          producers.Add(c.Get(i));
        // }
      }
      installConsumer(consumers);
      installProducer(producers);
  }

  void initialiseVanet() {
      Ns2MobilityHelper ns2MobilityHelper = Ns2MobilityHelper(traceFile);
      NodeContainer c;

      c.Create(nodeCount);
      ns2MobilityHelper.Install();

      NetDeviceContainer netDevices;
      installWave(c, netDevices);
      installNDN(c);
      installProdConsHelpers(c);

      for(int i = 0; i < c.GetN(); i++) {
        Simulator::Schedule(Seconds(1), &printPosition, c.Get(i)->GetObject<WaypointMobilityModel>());
      }
  }

  int main (int argc, char *argv[]) {
      CommandLine cmd;
      cmd.Usage("NDN V2V simulator.");
      cmd.AddValue("producer-count", "An int, specifies producers present", producerCount);
      cmd.AddValue("consumer-count", "An int, specifies consumers present", consumerCount);
      cmd.AddValue("num-nodes", "An int, specifies total amount of routers, producers & consumers present", nodeCount);
      cmd.AddValue("simulation-length", "Specify simulation length in seconds", simulationEnd);
      cmd.AddValue("trace-file", "The Path to the .tcl mobility file, likely generated by a 'sumo -c' command. Using an PATH variable is reccommended to avoid a headwreck.", traceFile);
      cmd.AddValue("anim-file", "Specify name of anim file to be outputted (.xml)", animFile);
      cmd.Parse (argc, argv);

      initialiseVanet();
      NS_LOG_UNCOND("\n" + cmd.GetName() + " running...");
      AnimationInterface anim(animFile);
      Simulator::Stop(Seconds(simulationEnd));
      Simulator::Run ();
      NS_LOG_UNCOND("Done.\n");
      return 0;
  }
} // namespace ns3

int main(int argc, char* argv[]) {
    return ns3::main(argc, argv);
}
