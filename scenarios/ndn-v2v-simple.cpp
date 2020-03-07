#include "ns3/core-module.h"

#include "ns3/ns2-mobility-helper.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/ndnSIM/apps/ndn-producer.hpp"
#include "ns3/random-variable-stream.h"
#include "ns3/ndnSIM/apps/ndn-proactive-producer.hpp"
#include "ns3/ndnSIM/apps/ndn-consumer-cbr.hpp"
#include "ns3/ndnSIM/apps/ndn-consumer.hpp"
#include "ns3/ndnSIM/apps/ndn-app.hpp"
#include "ns3/ndnSIM/helper/ndn-app-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include <ns3/ndnSIM/helper/ndn-global-routing-helper.hpp>
#include "ns3/animation-interface.h"
#include "ns3/ndnSIM-module.h"
#include "model/ndn-l3-protocol.hpp"
#include "ns3/ndnSIM/NFD/daemon/table/pit.hpp"


#include <algorithm>
#include <vector>
#include <iostream>
#include <list>
#include <iterator>
#include <string>


namespace ns3{
  NS_LOG_COMPONENT_DEFINE ("V2VSimple");
  int simulationEnd = 100;
  int nodeCount = 0;
  int contentTrigger_x_start = 0;
  int contentTrigger_x_end = 0;
  int contentTrigger_l_start = 0;
  int contentTrigger_l_end = simulationEnd;
  int contentTrigger_x_speed = 0;
  std::string traceFile = "File path goes here...";
  std::string traceOutput = std::getenv("NS3_TRACE_OUTPUTS");
  std::string animFile = "../results/ndn-v2v-test.xml";

  void installWave(NodeContainer &c, NetDeviceContainer &devices) {
      // Modulation and wifi channel bit rate
      std::string phyMode ("OfdmRate6MbpsBW10MHz");
      // std::string phyMode("OfdmRate24Mbps");
      // Fix non-unicast data rate to be the same as that of unicast
      Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));

      YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
      // wifiPhy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
      wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11);
      wifiPhy.EnablePcap ("wave-simple-80211p", devices);
      YansWifiChannelHelper wifiChannel;
      wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
      wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel",
                                     "MaxRange", DoubleValue(100.0));

      wifiPhy.SetChannel(wifiChannel.Create());
      Wifi80211pHelper waveHelper = Wifi80211pHelper::Default();
      waveHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                        "DataMode",StringValue (phyMode),
                                        "ControlMode",StringValue (phyMode));

      QosWaveMacHelper qosWaveMacHelper = QosWaveMacHelper::Default();
      devices = waveHelper.Install(wifiPhy, qosWaveMacHelper, c);
  }

  void installNDN(NodeContainer &c) {
      ndn::StackHelper ndnHelper;
      ndnHelper.SetDefaultRoutes(true);
      ndnHelper.Install(c);
      ndn::StrategyChoiceHelper::InstallAll("/", "/localhost/nfd/strategy/broadcast");
  }

  void printProgress(double percent) {
    std::cout << percent << "% done" << '\n';
  }

  void progressLogging(){
    double _perc05 = simulationEnd * 0.05;
    double _perc10 = simulationEnd * 0.1;
    double _perc15 = simulationEnd * 0.15;
    double _perc20 = simulationEnd * 0.2;
    double _perc25 = simulationEnd * 0.25;
    double _perc30 = simulationEnd * 0.3;
    double _perc35 = simulationEnd * 0.35;
    double _perc40 = simulationEnd * 0.4;
    double _perc45 = simulationEnd * 0.45;
    double _perc50 = simulationEnd * 0.5;
    double _perc55 = simulationEnd * 0.55;
    double _perc60 = simulationEnd * 0.6;
    double _perc65 = simulationEnd * 0.65;
    double _perc70 = simulationEnd * 0.7;
    double _perc75 = simulationEnd * 0.75;
    double _perc80 = simulationEnd * 0.8;
    double _perc85 = simulationEnd * 0.85;
    double _perc90 = simulationEnd * 0.9;
    double _perc95 = simulationEnd * 0.95;
    double _perc100 = simulationEnd * 1.0;
    Simulator::Schedule(Seconds(_perc05), &printProgress, 5.0);
    Simulator::Schedule(Seconds(_perc10), &printProgress, 10.0);
    Simulator::Schedule(Seconds(_perc15), &printProgress, 15.0);
    Simulator::Schedule(Seconds(_perc20), &printProgress, 20.0);
    Simulator::Schedule(Seconds(_perc25), &printProgress, 25.0);
    Simulator::Schedule(Seconds(_perc30), &printProgress, 30.0);
    Simulator::Schedule(Seconds(_perc35), &printProgress, 35.0);
    Simulator::Schedule(Seconds(_perc40), &printProgress, 40.0);
    Simulator::Schedule(Seconds(_perc45), &printProgress, 45.0);
    Simulator::Schedule(Seconds(_perc50), &printProgress, 50.0);
    Simulator::Schedule(Seconds(_perc55), &printProgress, 55.0);
    Simulator::Schedule(Seconds(_perc60), &printProgress, 60.0);
    Simulator::Schedule(Seconds(_perc65), &printProgress, 65.0);
    Simulator::Schedule(Seconds(_perc70), &printProgress, 70.0);
    Simulator::Schedule(Seconds(_perc75), &printProgress, 75.0);
    Simulator::Schedule(Seconds(_perc80), &printProgress, 80.0);
    Simulator::Schedule(Seconds(_perc85), &printProgress, 85.0);
    Simulator::Schedule(Seconds(_perc90), &printProgress, 90.0);
    Simulator::Schedule(Seconds(_perc95), &printProgress, 95.0);
    Simulator::Schedule(Seconds(_perc100), &printProgress, 100.0);
  }

  void nodeLogging(NodeContainer &c) {
    for (int i = 0; i < nodeCount; i++) {
      std::cout << endl << "node(" << i << "):" << endl;
      auto node = c.Get(i);
      auto& nodePit = node->GetObject<ndn::L3Protocol>()->getForwarder()->getPit();
      nfd::Pit::const_iterator iter;
      for (iter = nodePit.begin(); iter != nodePit.end(); iter++) {
        cout << iter->getName() << endl;
      }
    }
  }

  void installConsumer(NodeContainer &c) {
      ndn::AppHelper helper("ns3::ndn::ConsumerCbr");
      helper.SetAttribute("Frequency", DoubleValue (0.3));
      helper.SetAttribute("Randomize", StringValue("uniform"));
      helper.SetPrefix("/criticalData/test");
      helper.Install(c);
  }

  void installProducer(NodeContainer &c) {
      ndn::AppHelper producerHelper("ns3::ndn::Producer");
      producerHelper.SetPrefix("/criticalData");
      producerHelper.SetAttribute("Freshness", TimeValue(Seconds(20)));
      producerHelper.SetAttribute("SimEnd", UintegerValue(simulationEnd));
      producerHelper.SetAttribute("CTxStart", UintegerValue(contentTrigger_x_start));
      producerHelper.SetAttribute("CTxEnd", UintegerValue(contentTrigger_x_end));
      producerHelper.SetAttribute("CTlStart", UintegerValue(contentTrigger_l_start));
      producerHelper.SetAttribute("CTlEnd", UintegerValue(contentTrigger_l_end));
      producerHelper.SetAttribute("CTxSpeed", UintegerValue(contentTrigger_x_speed));
      // The below attribute needs to be configured based on whether or not it is operating
      // in tandem with PCD or not
      producerHelper.SetAttribute("ProducerActivationPos", UintegerValue(contentTrigger_x_start));
      producerHelper.Install(c);
  }

  void installProactiveProducer(NodeContainer &c) {
      ndn::AppHelper proactiveProducerHelper("ns3::ndn::ProactiveProducer");
      proactiveProducerHelper.SetPrefix("/criticalData/test");
      proactiveProducerHelper.SetAttribute("Freshness", TimeValue(Seconds(20)));
      proactiveProducerHelper.SetAttribute("SimEnd", UintegerValue(simulationEnd));
      proactiveProducerHelper.SetAttribute("CTxStart", UintegerValue(contentTrigger_x_start));
      proactiveProducerHelper.SetAttribute("CTxEnd", UintegerValue(contentTrigger_x_end));
      proactiveProducerHelper.SetAttribute("CTlStart", UintegerValue(contentTrigger_l_start));
      proactiveProducerHelper.SetAttribute("CTlEnd", UintegerValue(contentTrigger_l_end));
      proactiveProducerHelper.SetAttribute("CTxSpeed", UintegerValue(contentTrigger_x_speed));
      proactiveProducerHelper.Install(c);
  }

  void installAppHelpers(NodeContainer &c) {
      NodeContainer producers;
      NodeContainer consumers;
      NodeContainer proactiveProducers;
      // Producers and Consumers assigned based on the list of producerIds above
      for (int i = 0; i < nodeCount; i++) {
          if (i != 5 && i != 0) {
            consumers.Add(c.Get(i));
            // producers.Add(c.Get(i));
          }
          //OK LOL. Just figured out how to get the position,
          //Position based triggering is now a reality!!!!
          c.Get(i)->GetObject<MobilityModel>()->GetPosition().x;
      }
      //Temporary to test functionality of proactiveProducer model
      proactiveProducers.Add(c.Get(1));
      //Temporary for specific scenario testing
      producers.Add(c.Get(1));

      installConsumer(consumers);
      // installProactiveProducer(proactiveProducers);
      installProducer(producers);
  }

  void initialiseVanet() {
      Ns2MobilityHelper ns2MobilityHelper = Ns2MobilityHelper(traceFile);
      NodeContainer c;
      NetDeviceContainer netDevices;

      NS_LOG_UNCOND("Setting up nodes...");
      c.Create(nodeCount);
      NS_LOG_UNCOND("Installing ns2MobilityHelper...");
      ns2MobilityHelper.Install();

      NS_LOG_UNCOND("Installing WAVE on all nodes...");
      installWave(c, netDevices);
      NS_LOG_UNCOND("Installing NDN Stack on all Nodes...");
      installNDN(c);
      NS_LOG_UNCOND("Assigning producers and consumers as needed...");
      installAppHelpers(c);

      // Simulator::Schedule(Seconds(5), &nodeLogging, c);
      NS_LOG_UNCOND("Setup complete.");
  }

  int main (int argc, char *argv[]) {
      CommandLine cmd;
      cmd.Usage("NDN V2V simulator.");
      cmd.AddValue("num-nodes", "An int, specifies total amount of routers, producers & consumers present", nodeCount);
      cmd.AddValue("simulation-length", "Specify simulation length in seconds", simulationEnd);
      cmd.AddValue("content-trigger-x-start", "Specifies where critical content becomes available in the simulation", contentTrigger_x_start);
      cmd.AddValue("content-trigger-x-end", "Specifies where critical content is no longer available in the simulation", contentTrigger_x_end);
      cmd.AddValue("content-trigger-x-speed", "Specifies the speed of the content if it is mobile", contentTrigger_x_speed);
      cmd.AddValue("content-trigger-l-start", "Specifies when critical content is available in the simulation", contentTrigger_l_start);
      cmd.AddValue("content-trigger-l-end", "Specifies when critical content is no longer available in the simulation", contentTrigger_l_end);
      cmd.AddValue("trace-file", "The Path to the .tcl mobility file, likely generated by a 'sumo -c' command.\nUsing an PATH variable is reccommended to avoid a headwreck such as $SUMO_PROJECTS_ROOT", traceFile);
      cmd.AddValue("anim-file", "Specify name of anim file to be outputted (.xml)", animFile);
      cmd.Parse(argc, argv);

      initialiseVanet();

      NS_LOG_UNCOND("\n" + cmd.GetName() + " running...");
      AnimationInterface anim(animFile);
      anim.SetMaxPktsPerTraceFile(50000000);

      progressLogging();

      Simulator::Stop(Seconds(simulationEnd));
      traceOutput = traceOutput + "/l3-rate-trace.txt";
      ndn::L3RateTracer::InstallAll(traceOutput, Seconds(0.05));
      Simulator::Run();
      NS_LOG_UNCOND("Done.\n");
      return 0;
  }
} // namespace ns3

int main(int argc, char* argv[]) {
    return ns3::main(argc, argv);
}
