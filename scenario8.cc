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
  std::string cacheSize = "200";
  std::string Frequency = "100";
  std::string NumberOfContents = "200";


  AnnotatedTopologyReader topologyReader("", 1);
  topologyReader.SetFileName("src/ndnSIM/examples/topologies/testbed.txt");
  topologyReader.Read();

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetOldContentStore("ns3::ndn::cs::Lru", "MaxSize",
                               cacheSize); // ! Attention ! If set to 0, then MaxSize is infinite
  ndnHelper.InstallAll();

  // Set BestRoute strategy
  ndn::StrategyChoiceHelper::InstallAll("/", "/localhost/nfd/strategy/ncc");

  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();

  // Getting containers for the consumer/producer
  Ptr<Node> consumers[3] = {Names::Find<Node>("caida"),Names::Find<Node>("ntnu"),Names::Find<Node>("tongji")};

  Ptr<Node> producers[1] = {Names::Find<Node>("urjc")};

  if (consumers[0] == 0 || producers[0] == 0){
   NS_FATAL_ERROR("Error in topology: node is missing");
 }


  //Consumer application of Movie
  for (int i = 0; i < 3; i++) {
   ndn::AppHelper consumerHelper("ns3::ndn::ConsumerZipfMandelbrot");
   consumerHelper.SetPrefix("/Movie");
    consumerHelper.SetAttribute("Frequency", StringValue(Frequency));        // 100 interests a second
    consumerHelper.SetAttribute("NumberOfContents", StringValue(NumberOfContents));

    ApplicationContainer consumer = consumerHelper.Install(consumers[i]);
    consumer.Start(Seconds(0));
    consumer.Stop(Seconds(20));
  }




    //Movie Producer
  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));

    // install producer that will satisfy Interests in /dst1 namespace
  ndnGlobalRoutingHelper.AddOrigins("/Movie", producers[0]);
  producerHelper.SetPrefix("/Movie");
  ApplicationContainer Producer = producerHelper.Install(producers[0]);


  // }

  // Manually configure FIB routes
  ndn::GlobalRoutingHelper::CalculateRoutes();

  // Schedule simulation time and run the simulation
  Simulator::Stop(Seconds(30.0));

 ndn::L3RateTracer::InstallAll("rate-trace.txt", Seconds(0.5));

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
