#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/dce-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/constant-position-mobility-model.h"
#include "misc-tools.h"

#include <iostream>
#include <fstream>

using namespace std;
using namespace ns3;

// ===========================================================================
//
//         node 0                 node 1
//   +----------------+    +----------------+
//   |                |    |                |
//   +----------------+    +----------------+
//   |    10.1.1.1    |    |    10.1.1.2    |
//   +----------------+    +----------------+
//   | point-to-point |    | point-to-point |
//   +----------------+    +----------------+
//           |                     |
//           +---------------------+
//                5 Mbps, 1 ms
//
// Just a ping !
// wget http://www.skbuff.net/iputils/iputils-s20101006.tar.bz2
//
//recompile iputils: edit Makefile: replace "CFLAGS=" with "CFLAGS+=" and run:
//                   "make CFLAGS=-fPIC LDFLAGS=-pie ping"
// ===========================================================================
int main (int argc, char *argv[])
{
  uint32_t systemId = 0;
  uint32_t systemCount = 1;

  bool useKernel = 1;
  CommandLine cmd;
  cmd.AddValue ("kernel", "Use kernel linux IP stack.", useKernel);
  cmd.Parse (argc, argv);

  ofstream conf;
  conf.open("files-0/etc/zebra.conf");
  conf << "hostname node0" << std::endl;
  conf << "password zebra" << std::endl;
  conf << "log stdout" << std::endl;
  conf << "debug zebra kernel" << std::endl;
  conf << "debug zebra events" << std::endl;
  conf << "debug zebra packet" << std::endl;
  conf.close();
  conf.open("files-1/etc/zebra.conf");
  conf << "hostname node1" << std::endl;
  conf << "password zebra" << std::endl;
  conf << "log stdout" << std::endl;
  conf << "debug zebra kernel" << std::endl;
  conf << "debug zebra events" << std::endl;
  conf << "debug zebra packet" << std::endl;
  conf.close();

  NodeContainer nodes;
  Ptr<Node> node1 = CreateObject<Node> (0 % systemCount); // Create node1 with system id 0
  Ptr<Node> node2 = CreateObject<Node> (1 % systemCount); // Create node2 with system id 1
  nodes.Add (node1);
  nodes.Add (node2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("1ms"));

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

  if (!useKernel)
    {
      InternetStackHelper stack;
      stack.Install (nodes);

      Ipv4AddressHelper address;
      address.SetBase ("10.1.1.0", "255.255.255.252");
      Ipv4InterfaceContainer interfaces = address.Assign (devices);

      // setup ip routes
      Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    }

  DceManagerHelper dceManager;
  dceManager.SetTaskManagerAttribute( "FiberManagerType", StringValue ( "UcontextFiberManager" ) );

  if (useKernel)
    {
      dceManager.SetNetworkStack ("ns3::LinuxSocketFdFactory", "Library", StringValue ("liblinux.so"));

      AddAddress (nodes.Get (0), Seconds (0.1), "sim0", "10.1.1.1/8");
      //RunIp (nodes.Get (0), Seconds (0.2), "link set sim0 up arp off");
      RunIp (nodes.Get (0), Seconds (0.2), "link set sim0 up");

      AddAddress (nodes.Get (1), Seconds (0.3), "sim0","10.1.1.2/8");
      //RunIp (nodes.Get (1), Seconds (0.4), "link set sim0 up arp off");
      RunIp (nodes.Get (1), Seconds (0.4), "link set sim0 up");

    }
  dceManager.Install (nodes);

  DceApplicationHelper dce1, dce2;
  ApplicationContainer app1, app2;


  //if ( systemId == node1->GetSystemId () )
  //  {
      dce1.SetStackSize (1 << 20);
      // Launch ping on node 0
      dce1.SetBinary ("zebra");
      dce1.ResetArguments ();
      //dce1.ResetEnvironment ();
      dce1.AddArguments ("--log=stdout");
      dce1.AddArguments ("--log-level=debug");
      dce1.AddArguments ("-f", "/etc/zebra.conf");
      //dce.AddArguments ("-d");

      app1 = dce1.Install (node1);
      app1.Start (Seconds (1.0));
  /*  }
  if ( systemId == node2->GetSystemId ())
    {*/
      // Launch ping on node 1
      dce2.SetStackSize (1 << 20);
      dce2.SetBinary ("zebra");
      dce2.ResetArguments ();
      //dce.ResetEnvironment ();
      dce2.AddArguments ("--log=stdout");
      dce2.AddArguments ("--log-level=debug");
      dce2.AddArguments ("-f", "/etc/zebra.conf");

      app2 = dce2.Install (node2);
      app2.Start (Seconds (1.5));
    //}

  app1.Stop(Seconds(20.0)); // sigsev
  app2.Stop(Seconds(20.0));

  Simulator::Stop (Seconds (40.0)); // sigabrt
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
