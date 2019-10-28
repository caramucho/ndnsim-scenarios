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


 int
 main(int argc, char* argv[])
 {
  std::string cacheSize = "50";
  std::string Frequency = "300";
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
  ndn::StrategyChoiceHelper::InstallAll("/", "/localhost/nfd/strategy/multipath");

  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();

  // Getting containers for the consumer/producer
  Ptr<Node> producers[1] = {Names::Find<Node>("ntnu")};

  Ptr<Node> consumers[1] = {Names::Find<Node>("urjc")};

  if (consumers[0] == 0 || producers[0] == 0){
   NS_FATAL_ERROR("Error in topology: node is missing");
 }


  //Consumer application of Movie

   ndn::AppHelper consumerHelper("ns3::ndn::ConsumerZipfMandelbrot");
   consumerHelper.SetPrefix("/Movie");
    consumerHelper.SetAttribute("Frequency", StringValue(Frequency));        // 100 interests a second
    consumerHelper.SetAttribute("NumberOfContents", StringValue(NumberOfContents));

    ApplicationContainer consumer = consumerHelper.Install(consumers[0]);
    consumer.Start(Seconds(0));
    consumer.Stop(Seconds(20));





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
  // ndn::FibHelper::AddRoute("urjc", "/Movie", "lip6", 2);
  // ndn::FibHelper::AddRoute("urjc", "/Movie", "orange", 3);
  // ndn::FibHelper::AddRoute("urjc", "/Movie", "padua", 1);
  // ndn::FibHelper::AddRoute("urjc", "/Movie", "basel", 4);

  // ndn::FibHelper::AddRoute("lip6", "/Movie", "systemx", 1);
  // ndn::FibHelper::AddRoute("lip6", "/Movie", "orange", 2);
  // ndn::FibHelper::AddRoute("lip6", "/Movie", "ntnu", 3);

  // ndn::FibHelper::AddRoute("orange", "/Movie", "basel", 1);
  // ndn::FibHelper::AddRoute("orange", "/Movie", "systemx", 2);

  // ndn::FibHelper::AddRoute("systemx", "/Movie", "ntnu", 1);

  // ndn::FibHelper::AddRoute("basel", "/Movie", "ntnu", 1);

  // ndn::FibHelper::AddRoute("padua", "/Movie", "basel", 1);
  // ndn::FibHelper::AddRoute("padua", "/Movie", "orange", 2);


  // Schedule simulation time and run the simulation
  Simulator::Stop(Seconds(21.0));

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
