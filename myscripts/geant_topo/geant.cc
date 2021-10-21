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

#include "ns3/netanim-module.h"

using namespace std;
using namespace ns3;

NS_LOG_COMPONENT_DEFINE("GEANT");

static unsigned int a = 1; // unique addr counter

void ConfigureIface(Ptr<Node> node, unsigned int id) {
    LinuxStackHelper::RunIp (node, Seconds (0.2), "a add 10.0.0." + to_string(node->GetId()+id+a) + "/24 dev sim" + to_string(id));
    LinuxStackHelper::RunIp (node, Seconds (0.2), "l set dev sim" + to_string(id) + " up");
    a++;
}

unsigned int FindIface(map<string, unsigned int> *nifaces, string id) {
    auto oiface = nifaces->find(id);
    if (oiface == nifaces->end()) {
	nifaces->insert(pair<string, unsigned int>(id, 0));
	return 0;
    }
    nifaces->erase(id);
    nifaces->insert(pair<string, unsigned int>(id, oiface->second+1));
    return oiface->second;
}

int main (int argc, char *argv[]) {
    NodeContainer nodes;
    PointToPointHelper pointToPoint;
    DceManagerHelper dceManager;
    dceManager.SetTaskManagerAttribute( "FiberManagerType", StringValue ( "UcontextFiberManager" ) );
    dceManager.SetNetworkStack ("ns3::LinuxSocketFdFactory", "Library", StringValue ("liblinux.so"));

    NtfContent ntf("geant.ntf");
    NS_LOG_FUNCTION(ntf.nNodes());
    nodes.Create(ntf.nNodes());
    dceManager.Install (nodes);

    map<string, unsigned int> nifaces; // number of iface per node

    // Create the topology
    for (vector<Link>::iterator links = ntf.getLinks(); links != ntf.getLastLink(); links++) {
	Link l = *links;
	int n1_id = ntf.getNodeId(l.origin), n2_id = ntf.getNodeId(l.end);
	Ptr<Node> n1 = nodes.Get(n1_id), n2 = nodes.Get(n2_id);
	pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
	pointToPoint.SetChannelAttribute ("Delay", StringValue (to_string(l.delay) + "ms"));
	pointToPoint.Install(n1, n2);

	ConfigureIface(n1, FindIface(&nifaces, l.origin));
	ConfigureIface(n2, FindIface(&nifaces, l.end));
    }
#include "ns3/netanim-module.h"
    DceApplicationHelper dce;
    ApplicationContainer app;

    // Here, all the ifaces of each node is configured
    // We need to generate the bird config
    for (auto node = ntf.getNodes(); node != ntf.getLastNode(); node++) {
	string node_name = node->first;
	int node_id = node->second;
	unsigned int nif = nifaces.find(node_name)->second;

	string conf_dir = "files-" + to_string(node_id) + "/etc";
	mkdir(conf_dir.c_str(), S_IRWXU | S_IRWXG);

	ofstream config(conf_dir + "/bird.conf");
	config << "log \"/var/log/bird.log\" all;" << endl;
	config << "router id " << node_id+1 << ";" <<endl;
	config << "protocol device {}" << endl;
	config << "protocol direct {\n\tdisabled;\n\tipv4;\n\tipv6;\n}" <<endl;
	config << "protocol kernel {\n\tipv4 {\n\t\texport all;\n\t};\n}" << endl;
	config << "protocol static {\n\tipv4;\n}" <<endl;
	config << "protocol ospf v2 {\n\tarea 0 {" <<endl;
	for (unsigned int i=0; i<=nif; i++) {
	    config << "\t\tinterface \"sim" << i <<"\" {" << endl;
	    //config << "\t\t\ttype broadcast;" <<endl;
	    config << "\t\t\ttype ptp;" <<endl;
	    config << "\t\t\tcost " << 10 << ";" <<endl;
	    config << "\t\t\thello 5;\n\t\t};" <<endl;
	}
	config << "\t};\n}" << endl;
	config.close();

	LinuxStackHelper::RunIp (nodes.Get(node_id), Seconds (0.2), "l set dev lo up");

	dce.SetStackSize (1 << 20);
	dce.SetBinary ("bird");
	dce.ResetArguments ();
	dce.ParseArguments("-d -c /etc/bird.conf");
	app = dce.Install (nodes.Get(node_id));
	app.Start (Seconds (0.5*node_id));
	//app.Stop (Seconds (3540));
    }

    // TODO: put callback for failure scenario

  /*RunIp (nodes.Get(0), Seconds (130), "l set dev sim0 down");
  RunIp (nodes.Get (1), Seconds (120), "-6 r");
  RunIp (nodes.Get (0), Seconds (120), "-6 r");*/

  //RunIp (nodes.Get (1), Seconds (121), "-6 a");
  //RunIp (nodes.Get (0), Seconds (121), "-6 a");*/

    AnimationInterface anim ("geant-anim.xml");
  pointToPoint.EnablePcapAll("geant", true);

  Simulator::Stop (Seconds (180));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
