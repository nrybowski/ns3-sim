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
    LinuxStackHelper::RunIp (node, Seconds (0.2), "a add 10.0.0." + to_string(node->GetId()+id+a++) + "/24 dev sim" + to_string(id));
    LinuxStackHelper::RunIp (node, Seconds (0.2), "l set dev sim" + to_string(id) + " up");
}

int main (int argc, char *argv[]) {
    NodeContainer nodes;
    PointToPointHelper pointToPoint;
    DceManagerHelper dceManager;
    dceManager.SetTaskManagerAttribute( "FiberManagerType", StringValue ( "UcontextFiberManager" ) );
    dceManager.SetNetworkStack ("ns3::LinuxSocketFdFactory", "Library", StringValue ("liblinux.so"));

    NtfContent ntf("geant.ntf");
    NS_LOG_FUNCTION(ntf.nNodes());
    ntf.dumpLinks();
    nodes.Create(ntf.nNodes());
    dceManager.Install (nodes);

    map<tuple<uint32_t, uint32_t>, uint32_t> metrics;

    // Create the topology
    for (vector<Link>::iterator links = ntf.getLinks(); links != ntf.getLastLink(); links++) {
	Link l = *links;
	// From NTF parser to NS3
	int n1_id = ntf.getNodeId(l.origin), n2_id = ntf.getNodeId(l.end);

	Ptr<Node> n1 = nodes.Get(n1_id), n2 = nodes.Get(n2_id);
	pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
	pointToPoint.SetChannelAttribute ("Delay", StringValue (to_string(l.delay) + "ms"));
	// Install NetDevices on nodes and channel between them
	pointToPoint.Install(n1, n2);
	
	// Current iface_id is the last one added
	uint32_t n1_iface_id = n1->GetNDevices()-1, n2_iface_id = n2->GetNDevices()-1;

	// map <node_id, iface_id> = delay for BIRD config file
	metrics.insert(pair<tuple<uint32_t, uint32_t>, uint32_t>(make_tuple(n1_id, n1_iface_id), l.f_metric));
	metrics.insert(pair<tuple<uint32_t, uint32_t>, uint32_t>(make_tuple(n2_id, n2_iface_id), l.r_metric));

	ConfigureIface(n1, n1_iface_id);
	ConfigureIface(n2, n1_iface_id);

    }
    
    DceApplicationHelper dce;
    ApplicationContainer app;

    // Here, all the ifaces of each node is configured
    // We need to generate the bird config
    for (auto node = nodes.Begin(); node != nodes.End(); node++) {
	uint32_t node_id = (*node)->GetId(),
		 n_ifaces = (*node)->GetNDevices();

	string conf_dir = "files-" + to_string(node_id) + "/etc";
	mkdir(conf_dir.c_str(), S_IRWXU | S_IRWXG);

	ofstream config(conf_dir + "/bird.conf");
	
	// Generic BIRD config
	config << "log \"/var/log/bird.log\" all;" <<endl;
	config << "router id " << node_id+1 << ";" <<endl;
	config << "protocol device {}" <<endl;
	config << "protocol direct {\n\tdisabled;\n\tipv4;\n\tipv6;\n}" <<endl;
	config << "protocol kernel {\n\tipv4 {\n\t\texport all;\n\t};\n}" <<endl;
	config << "protocol static {\n\tipv4;\n}" <<endl;

	// OSPF config
	map<tuple<uint32_t, uint32_t>, uint32_t>::iterator metric_data;
	uint32_t metric;
	config << "protocol ospf v2 {\n\tarea 0 {" <<endl;
	for (uint32_t i=0; i<n_ifaces; i++) {
	    metric_data = metrics.find(make_tuple(node_id, i));
	    // 0 will cause invalid BIRD config and segfault during NS3 run
	    metric = metric_data == metrics.end() ? 0 : metric_data->second;
	    config << "\t\tinterface \"sim" << i <<"\" {" << endl;
	    config << "\t\t\ttype ptp;" <<endl;
	    config << "\t\t\tcost " << metric << ";" <<endl;
	    config << "\t\t\thello 5;\n\t\t};" <<endl;
	}
	config << "\t};\n}" << endl;

	config.close();

	LinuxStackHelper::RunIp (*node, Seconds (0.2), "l set dev lo up");

	dce.SetStackSize (1 << 20);
	dce.SetBinary ("bird");
	dce.ResetArguments ();
	dce.ParseArguments("-d -c /etc/bird.conf");
	app = dce.Install (*node);
	app.Start (Seconds (0.5*node_id));
    }

    // TODO: put callback for failure scenario

    //AnimationInterface anim ("geant-anim.xml");
    pointToPoint.EnablePcapAll("geant", true);

    Simulator::Stop (Seconds (180));
    Simulator::Run ();
    Simulator::Destroy ();

  return 0;
}
