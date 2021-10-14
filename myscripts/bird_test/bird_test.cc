#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/dce-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/csma-module.h"

#include <iostream>
#include <fstream>

using namespace std;
using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Bird");

// ===========================================================================
//
//         node 0                 node 1
//   +----------------+    +----------------+----------------+    +----------------+
//   |                |    |                |                |    |                |
//   +----------------+    +----------------+----------------+    +----------------+
//   |    10.1.1.1    |    |    10.1.1.2    |   192.168.1.2  |    |   192.168.1.1  |
//   +----------------+    +----------------+----------------+    +----------------+
//   | point-to-point |    | point-to-point | point-to-point |    | point-to-point |
//   +----------------+    +----------------+----------------+    +----------------+
//           |                     |                 |                     |
//           +---------------------+                 +---------------------+
//                5 Mbps, 1 ms
//
// ===========================================================================
int main (int argc, char *argv[])
{
  NodeContainer nodes;
  PointToPointHelper pointToPoint;
  LinuxStackHelper stack;
  Ipv4DceRoutingHelper ipv4RoutingHelper;
  DceManagerHelper dceManager;
  NetDeviceContainer devices;

  // create nodes
  nodes.Create(4);

  dceManager.SetTaskManagerAttribute( "FiberManagerType", StringValue ( "UcontextFiberManager" ) );
  dceManager.SetNetworkStack ("ns3::LinuxSocketFdFactory", "Library", StringValue ("liblinux.so"));
  stack.SetRoutingHelper(ipv4RoutingHelper);
  stack.Install(nodes);
  dceManager.Install (nodes);

  // set p2p link attributes
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("1ms"));

  devices = pointToPoint.Install(nodes.Get(0), nodes.Get(1));
  LinuxStackHelper::RunIp (nodes.Get (0), Seconds (0.2), "link set sim0 up");
  LinuxStackHelper::RunIp (nodes.Get (0), Seconds (0.2), "a add 10.1.1.1/24 dev sim0");
  LinuxStackHelper::RunIp (nodes.Get (1), Seconds (0.4), "link set sim0 up");
  LinuxStackHelper::RunIp (nodes.Get (1), Seconds (0.4), "a add 10.1.1.2/24 dev sim0");

  pointToPoint.Install(nodes.Get(1), nodes.Get(2));
  LinuxStackHelper::RunIp (nodes.Get (2), Seconds (0.2), "link set sim0 up");
  LinuxStackHelper::RunIp (nodes.Get (2), Seconds (0.2), "a add 192.168.1.1/24 dev sim0");
  LinuxStackHelper::RunIp (nodes.Get (1), Seconds (0.4), "link set sim1 up");
  LinuxStackHelper::RunIp (nodes.Get (1), Seconds (0.4), "a add 192.168.1.2/24 dev sim1");

  pointToPoint.Install(nodes.Get(1), nodes.Get(3));
  LinuxStackHelper::RunIp (nodes.Get (3), Seconds (0.2), "link set sim0 up");
  LinuxStackHelper::RunIp (nodes.Get (3), Seconds (0.2), "a add 192.168.2.1/24 dev sim0");
  LinuxStackHelper::RunIp (nodes.Get (1), Seconds (0.4), "link set sim2 up");
  LinuxStackHelper::RunIp (nodes.Get (1), Seconds (0.4), "a add 192.168.2.2/24 dev sim2");

  pointToPoint.Install(nodes.Get(3), nodes.Get(2));
  LinuxStackHelper::RunIp (nodes.Get (3), Seconds (0.2), "link set sim1 up");
  LinuxStackHelper::RunIp (nodes.Get (3), Seconds (0.2), "a add 10.1.2.2/24 dev sim1");
  LinuxStackHelper::RunIp (nodes.Get (2), Seconds (0.2), "link set sim1 up");
  LinuxStackHelper::RunIp (nodes.Get (2), Seconds (0.2), "a add 10.1.2.1/24 dev sim1");

  DceApplicationHelper dce;
  ApplicationContainer app;

  dce.SetStackSize (1 << 20);
  dce.SetBinary ("bird");
  dce.ResetArguments ();
  dce.AddArguments("-d");
  app = dce.Install (nodes.Get(0));
  app.Start (Seconds (1.0));
  app.Stop (Seconds (180));
      
  dce.SetStackSize (1 << 20);
  dce.SetBinary ("bird");
  dce.ResetArguments ();
  dce.AddArguments("-d");
  app = dce.Install (nodes.Get(1));
  app.Start (Seconds (2.0));
  app.Stop (Seconds (180));

  dce.SetStackSize (1 << 20);
  dce.SetBinary ("bird");
  dce.ResetArguments ();
  dce.AddArguments("-d");
  app = dce.Install (nodes.Get(2));
  app.Start (Seconds (1.5));
  app.Stop (Seconds (180));

  dce.SetStackSize (1 << 20);
  dce.SetBinary ("bird");
  dce.ResetArguments ();
  dce.AddArguments("-d");
  app = dce.Install (nodes.Get(3));
  app.Start (Seconds (2.5));
  app.Stop (Seconds (180));

  LinuxStackHelper::RunIp (nodes.Get (1), Seconds (120), "r");
  LinuxStackHelper::RunIp (nodes.Get (3), Seconds (120), "l set dev sim0 down");
  LinuxStackHelper::RunIp (nodes.Get (1), Seconds (180), "r");

  // output
  pointToPoint.EnablePcapAll("bird_test", true);
  /*Ptr<Node> node;
  node = nodes.Get(1);
  for (uint32_t i=0; i<node->GetNDevices(); i++) {
  	pointToPoint.EnablePcap("bird_test", 1, i, true);
  }*/
  //Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("routes1.log", std::ios::out);
  //ipv4RoutingHelper.PrintRoutingTableAllEvery (Seconds (10), routingStream);

  Simulator::Stop (Seconds (200));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
