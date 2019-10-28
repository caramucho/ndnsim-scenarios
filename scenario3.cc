/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

// ndn-congestion-alt-topo-plugin.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"

 namespace ns3 {

/**
 *
 *   /------\ 0                                                 0 /------\
 *   |  c1  |<-----+                                       +----->|  p1  |
 *   \------/       \                                     /       \------/
 *                   \              /-----\              /
 *   /------\ 0       \         +==>| r12 |<==+         /       0 /------\
 *   |  c2  |<--+      \       /    \-----/    \       /      +-->|  p2  |
 *   \------/    \      \     |                 |     /      /    \------/
 *                \      |    |   1Mbps links   |    |      /
 *                 \  1  v0   v5               1v   2v  3  /
 *                  +->/------\                 /------\<-+
 *                    2|  r1  |<===============>|  r2  |4
 *                  +->\------/4               0\------/<-+
 *                 /    3^                           ^5    \
 *                /      |                           |      \
 *   /------\ 0  /      /                             \      \  0 /------\
 *   |  c3  |<--+      /                               \      +-->|  p3  |
 *   \------/         /                                 \         \------/
 *                   /     "All consumer-router and"     \
 *   /------\ 0     /      "router-producer links are"    \    0 /------\
 *   |  c4  |<-----+       "10Mbps"                        +---->|  p4  |
 *   \------/                                                    \------/
 *
 *   "Numbers near nodes denote face IDs. Face ID is assigned based on the order of link"
 *   "definitions in the topology file"
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-congestion-alt-topo-plugin
 */

 int
 main(int argc, char* argv[])
 {
  CommandLine cmd;
  cmd.Parse(argc, argv);

  AnnotatedTopologyReader topologyReader("", 1);
  topologyReader.SetFileName("src/ndnSIM/examples/topologies/testbed.txt");
  topologyReader.Read();

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetOldContentStore("ns3::ndn::cs::Lru", "MaxSize",
                               "200"); // ! Attention ! If set to 0, then MaxSize is infinite
  ndnHelper.InstallAll();

  // Set BestRoute strategy
  ndn::StrategyChoiceHelper::InstallAll("/", "/localhost/nfd/strategy/ncc");

  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();

  // Getting containers for the consumer/producer
  Ptr<Node> consumers[3] = {Names::Find<Node>("caida"),Names::Find<Node>("ntnu"),Names::Find<Node>("tongji")};

  Ptr<Node> producers[2] = {Names::Find<Node>("urjc"),Names::Find<Node>("verisign")};

  if (consumers[0] == 0 || producers[0] == 0){
   NS_FATAL_ERROR("Error in topology: node is missing");
 }

  // if (consumers[0] == 0 || consumers[1] == 0 || consumers[2] == 0 || consumers[3] == 0
  //     || producers[0] == 0 || producers[1] == 0 || producers[2] == 0 || producers[3] == 0) {
  //   NS_FATAL_ERROR("Error in topology: one nodes c1, c2, c3, c4, p1, p2, p3, or p4 is missing");
  // }
 
    // std::string prefix = "/" + Names::FindName(producers[0]);

  //Consumer application of Movie

   ndn::AppHelper consumerHelper("ns3::ndn::ConsumerZipfMandelbrot");
   consumerHelper.SetPrefix("/Movie");
    consumerHelper.SetAttribute("Frequency", StringValue("100"));        // 100 interests a second
    consumerHelper.SetAttribute("NumberOfContents", StringValue("200"));

    ApplicationContainer consumer0 = consumerHelper.Install(consumers[0]);
    consumer0.Start(Seconds(10));      
    consumer0.Stop(Seconds(20)); 

    ApplicationContainer consumer1 = consumerHelper.Install(consumers[1]);
    consumer1.Start(Seconds(0));      
    consumer1.Stop(Seconds(10)); 

    consumerHelper.SetAttribute("Frequency", StringValue("50"));
    ApplicationContainer consumer2 = consumerHelper.Install(consumers[2]);
    consumer2.Start(Seconds(0));      
    consumer2.Stop(Seconds(20)); 
 
    consumerHelper.SetPrefix("/Music");
    consumerHelper.SetAttribute("Frequency", StringValue("100"));        // 100 interests a second

    ApplicationContainer consumer3 = consumerHelper.Install(consumers[0]);
    consumer3.Start(Seconds(0));      
    consumer3.Stop(Seconds(10)); 

    ApplicationContainer consumer4 = consumerHelper.Install(consumers[1]);
    consumer4.Start(Seconds(10));      
    consumer4.Stop(Seconds(20)); 

    consumerHelper.SetAttribute("Frequency", StringValue("50"));
    ApplicationContainer consumer5 = consumerHelper.Install(consumers[2]);
    consumer5.Start(Seconds(0));      
    consumer5.Stop(Seconds(20)); 
  


    //Movie Producer
  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));

    // install producer that will satisfy Interests in /dst1 namespace
  ndnGlobalRoutingHelper.AddOrigins("/Movie", producers[0]);
  producerHelper.SetPrefix("/Movie");
  ApplicationContainer MovieProducer = producerHelper.Install(producers[0]);
    // when Start/Stop time is not specified, the application is running throughout the simulation

    //Music Producer
  //ndn::AppHelper producerHelper("ns3::ndn::Producer");
  //producerHelper.SetAttribute("PayloadSize", StringValue("1024"));

    // install producer that will satisfy Interests in /dst1 namespace
  ndnGlobalRoutingHelper.AddOrigins("/Music", producers[1]);
  producerHelper.SetPrefix("/Music");
  ApplicationContainer MusicProducer = producerHelper.Install(producers[1]);

  // }

  // Manually configure FIB routes
  ndn::GlobalRoutingHelper::CalculateRoutes();
  // ndn::FibHelper::AddRoute("c1", "/data", "n1", 1); // link to n1
  // ndn::FibHelper::AddRoute("c2", "/data", "n1", 1); // link to n1
  // ndn::FibHelper::AddRoute("c3", "/data", "n1", 1); // link to n1
  // ndn::FibHelper::AddRoute("c4", "/data", "n1", 1); // link to n1

  // ndn::FibHelper::AddRoute("n1", "/data", "n2", 1);  // link to n2
  // ndn::FibHelper::AddRoute("n1", "/data", "n12", 2); // link to n12

  // ndn::FibHelper::AddRoute("n12", "/data", "n2", 1); // link to n2

  // ndn::FibHelper::AddRoute("n2", "/data/p1", "p1", 1); // link to p1
  // ndn::FibHelper::AddRoute("n2", "/data/p2", "p2", 1); // link to p2
  // ndn::FibHelper::AddRoute("n2", "/data/p3", "p3", 1); // link to p3
  // ndn::FibHelper::AddRoute("n2", "/data/p4", "p4", 1); // link to p4

  // Schedule simulation time and run the simulation
  Simulator::Stop(Seconds(30.0));
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
