// ===========================================================================
//
//   Topology
//
//         node 0                 node 1
//   +----------------+    +----------------+
//   |    ns-3 TCP    |    |    ns-3 TCP    |
//   +----------------+    +----------------+
//   |    10.1.1.1    |    |    10.1.1.2    |
//   +----------------+    +----------------+
//   | point-to-point |    | point-to-point |
//   +----------------+    +----------------+
//           |                     |
//           +---------------------+
//                100 Mbps, 1 ms
//
// This experience do bittorent !
//
//  Node 0 : server
//
// ===========================================================================

#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/dce-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/trace-helper.h"
#include <sys/stat.h>
#include <sys/types.h>

#include <iostream>
#include <fstream>

using namespace std;
using namespace ns3;

int
main (int argc, char *argv[])
{
  mkdir ("files-0",0744);
  mkdir ("files-0/var/",0744);
  mkdir ("files-0/var/tmp",0744);
  mkdir ("files-0/var/frr",0744);
  mkdir ("files-0/var/run",0744);
  mkdir ("files-0/usr/", 0744);
  mkdir ("files-0/usr/local/", 0744);
  mkdir ("files-0/usr/local/etc", 0744);
  mkdir ("files-0/etc", 0744);
  mkdir ("files-0/local", 0744);

  mkdir ("files-1",0744);
  mkdir ("files-1/var/",0744);
  mkdir ("files-1/var/tmp",0744);
  mkdir ("files-1/var/frr",0744);
  mkdir ("files-1/var/run",0744);
  mkdir ("files-1/usr/", 0744);
  mkdir ("files-1/usr/local/", 0744);
  mkdir ("files-1/usr/local/etc", 0744);
  mkdir ("files-1/etc", 0744);
  mkdir ("files-1/local", 0744);

  ofstream conf;
  conf.open("files-0/etc/zebra.conf");
  conf << "hostname test" << std::endl;
  conf.close();
  
  NodeContainer nodes;
  nodes.Create (2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("1ms"));

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

  DceManagerHelper dceManager;
  dceManager.SetNetworkStack ("ns3::LinuxSocketFdFactory", "Library", StringValue ("liblinux.so"));
  LinuxStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  // setup ip routes
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  dceManager.Install (nodes);


  DceApplicationHelper dce;
  ApplicationContainer apps;

  dce.SetStackSize (1 << 20);

  dce.SetBinary ("zebra");
  dce.ResetArguments ();
  dce.ResetEnvironment ();
  //dce.AddArguments ("-i", "/usr/local/etc/zebra.pid");
  dce.AddArguments("-d");
  dce.AddArguments ("-f", "/etc/zebra.conf");
  apps = dce.Install (nodes.Get (0));
  apps.Start (Seconds (1));

  dce.SetBinary ("zebra");
  dce.ResetArguments ();
  dce.ResetEnvironment ();
  dce.AddArguments("-d");
  dce.AddArguments ("-f", "/etc/zebra.conf");
  apps = dce.Install (nodes.Get (1));
  apps.Start (Seconds (3));
  
  apps.Stop(Seconds(15));

  Simulator::Stop (Seconds (20.0));
  Simulator::Run ();

  Simulator::Destroy ();

  //NS_LOG_INFO("End")

  return 0;
}
