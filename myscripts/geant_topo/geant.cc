#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/dce-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/csma-module.h"

#include <iostream>
#include <fstream>

#include "ns3/NtfParser.h"

using namespace std;
using namespace ns3;

NS_LOG_COMPONENT_DEFINE("GEANT");

static void RunIp (Ptr<Node> node, Time at, std::string str)
{
  DceApplicationHelper process;
  ApplicationContainer apps;
  process.SetBinary ("ip");
  process.SetStackSize (1 << 16);
  process.ResetArguments ();
  process.ParseArguments (str.c_str ());
  apps = process.Install (node);
  apps.Start (at);
}

int main (int argc, char *argv[]) {

    NodeContainer nodes;
    PointToPointHelper pointToPoint;
    //LinuxStackHelper stack;
    //Ipv6DceRoutingHelper ipv6RoutingHelper;
    DceManagerHelper dceManager;
    //NetDeviceContainer devices;
    
    //NtfContent ntf("geant.ntf");

    // create nodes
    nodes.Create(3);
    //nodes.Create(ntf.nNodes());

  dceManager.SetTaskManagerAttribute( "FiberManagerType", StringValue ( "UcontextFiberManager" ) );
  dceManager.SetNetworkStack ("ns3::LinuxSocketFdFactory", "Library", StringValue ("liblinux.so"));
  //stack.SetRoutingHelper(ipv6RoutingHelper);
  //stack.Install(nodes);
  dceManager.Install (nodes);

  // set p2p link attributes
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("1ms"));

  pointToPoint.Install(nodes.Get(0), nodes.Get(1));
  //LinuxStackHelper::RunIp (nodes.Get (0), Seconds (0.2), "-6 a add fd00::2/127 dev sim0");
  RunIp (nodes.Get (0), Seconds (0.2), "-6 a add fd00::2/127 dev sim0");
  //RunIp (nodes.Get (0), Seconds (0.2), "-6 a add 2001:db8:0:1::1/64 dev sim0");
  RunIp (nodes.Get (0), Seconds (0.2), "-6 l set sim0 up");
  RunIp (nodes.Get (0), Seconds (0.2), "-6 l set lo up");
  RunIp (nodes.Get (1), Seconds (0.4), "-6 a add fd00::3/127 dev sim0");
  //RunIp (nodes.Get (1), Seconds (0.4), "-6 a add 2001:db8:0:1::2/64 dev sim0");
  RunIp (nodes.Get (1), Seconds (0.4), "-6 l set sim0 up");
  RunIp (nodes.Get (1), Seconds (0.4), "-6 l set lo up");

  RunIp (nodes.Get (1), Seconds (150), "-6 r");
  RunIp (nodes.Get (0), Seconds (150), "-6 r");

  pointToPoint.Install(nodes.Get(1), nodes.Get(2));
  LinuxStackHelper::RunIp (nodes.Get (2), Seconds (0.2), "link set sim0 up");
  LinuxStackHelper::RunIp (nodes.Get (2), Seconds (0.2), "link set lo up");
  //LinuxStackHelper::RunIp (nodes.Get (2), Seconds (0.2), "a add 192.168.1.1/24 dev sim0");
  LinuxStackHelper::RunIp (nodes.Get (2), Seconds (0.2), "-6 a add fd00::4/127 dev sim0");
  LinuxStackHelper::RunIp (nodes.Get (1), Seconds (0.4), "link set sim1 up");
  LinuxStackHelper::RunIp (nodes.Get (1), Seconds (0.4), "link set lo up");
  //LinuxStackHelper::RunIp (nodes.Get (1), Seconds (0.4), "a add 192.168.1.2/24 dev sim1");
  LinuxStackHelper::RunIp (nodes.Get (1), Seconds (0.4), "-6 a add fd00::5/127 dev sim1");

  /*pointToPoint.Install(nodes.Get(1), nodes.Get(3));
  LinuxStackHelper::RunIp (nodes.Get (3), Seconds (0.2), "link set sim0 up");
  LinuxStackHelper::RunIp (nodes.Get (3), Seconds (0.2), "a add 192.168.2.1/24 dev sim0");
  LinuxStackHelper::RunIp (nodes.Get (1), Seconds (0.4), "link set sim2 up");
  LinuxStackHelper::RunIp (nodes.Get (1), Seconds (0.4), "a add 192.168.2.2/24 dev sim2");

  pointToPoint.Install(nodes.Get(3), nodes.Get(2));
  LinuxStackHelper::RunIp (nodes.Get (3), Seconds (0.2), "link set sim1 up");
  LinuxStackHelper::RunIp (nodes.Get (3), Seconds (0.2), "a add 10.1.2.2/24 dev sim1");
  LinuxStackHelper::RunIp (nodes.Get (2), Seconds (0.2), "link set sim1 up");
  LinuxStackHelper::RunIp (nodes.Get (2), Seconds (0.2), "a add 10.1.2.1/24 dev sim1");*/

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

  /*dce.SetStackSize (1 << 20);
  dce.SetBinary ("bird");
  dce.ResetArguments ();
  dce.AddArguments("-d");
  app = dce.Install (nodes.Get(3));
  app.Start (Seconds (2.5));
  app.Stop (Seconds (180));*/

  RunIp (nodes.Get(0), Seconds (130), "l set dev sim0 down");

  RunIp (nodes.Get (1), Seconds (120), "-6 r");
  RunIp (nodes.Get (0), Seconds (120), "-6 r");

  //RunIp (nodes.Get (1), Seconds (121), "-6 a");
  //RunIp (nodes.Get (0), Seconds (121), "-6 a");

  pointToPoint.EnablePcapAll("geant", true);

  Simulator::Stop (Seconds (200));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
