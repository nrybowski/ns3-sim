#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/dce-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/csma-module.h"

#include <iostream>
#include <fstream>
#include <thread>

#include "ns3/NtfParser.h"

#include "ns3/netanim-module.h"

#include "blackhole-error-model.h"


using namespace std;
using namespace ns3;

NS_LOG_COMPONENT_DEFINE("GEANT");

static unsigned int a = 2; // unique addr counter

bool LsaProcessDelay(Ptr<NetDevice> dev, Ptr<const Packet> pkt, uint16_t mode, const Address &sender) {
    //cout << WallClockSynchronizer::SleepWait(10000000) << endl;
    //cout << Simulator::SleepWait(10000000) << endl;
    //Simulator::ScheduleNow();
    std::this_thread::sleep_for (std::chrono::nanoseconds (10000000));
    return true;
}

void ConfigureIface(Ptr<Node> node, unsigned int id) {
    //LinuxStackHelper::RunIp (node, Seconds (1), "a add 10.0.0." + to_string(node->GetId()+id+a++) + "/24 dev sim" + to_string(id));
    LinuxStackHelper::RunIp (node, Seconds (1), "a add 10.0.0." + to_string(a++) + "/32 dev sim" + to_string(id));
    LinuxStackHelper::RunIp (node, Seconds (1), "l set dev sim" + to_string(id) + " up");
}

//void handler(Ptr<PointToPointNetDevice> d, PointerValue p) {
void handler(Ptr<PointToPointNetDevice> d, Ptr<ErrorModel> p) {
    //d->SetAttribute("DataRate", DataRateValue(DataRate(0)));
    //d->SetAttribute("ReceiverErrorModel", p);
    d->SetReceiveErrorModel(p);
}

void MakeLinkFail(Ptr<PointToPointNetDevice> src, Ptr<PointToPointNetDevice> dst) {
    Ptr<BlackholeErrorModel> em = CreateObject<BlackholeErrorModel>();
    src->SetReceiveErrorModel(PointerValue(em));
    dst->SetReceiveErrorModel(PointerValue(em));
}

vector<pair<Ptr<PointToPointNetDevice>, Ptr<PointToPointNetDevice>>> FindLinks(NodeContainer *nodes, uint32_t src_id, uint32_t dst_id, uint32_t delay_value, Time event) {
    Ptr<Node> src_node = nodes->Get(src_id);

    Ptr<NetDevice> src_nd, dst_nd;
    Ptr<PointToPointNetDevice> src_p2p, dst_p2p;
    Ptr<PointToPointChannel> chan;

    vector<pair<Ptr<PointToPointNetDevice>, Ptr<PointToPointNetDevice>>> ret;
    Time delay = Time::FromInteger(delay_value, Time::Unit::MS), chan_delay;

    for (uint32_t i=0; i<src_node->GetNDevices(); i++) {
	src_nd = src_node->GetDevice(i);
	src_p2p = DynamicCast<PointToPointNetDevice>(src_nd);
	chan = DynamicCast<PointToPointChannel>(src_p2p->GetChannel());
	//NS_ABORT_MSG_IF(chan->GetNDevices() > 2, "Too many devices on a P2P link");
	dst_nd = chan->GetDevice(0) == src_nd ? chan->GetDevice(1) : src_nd;
	// TODO : cannot get channel delay
	//if (dst_nd->GetNode()->GetId() != dst_id || delay.Compare(&(chan->GetDelay())) == 0) continue;
	if (dst_nd->GetNode()->GetId() != dst_id) continue;
	dst_p2p = DynamicCast<PointToPointNetDevice>(dst_nd);
	ret.push_back(pair<Ptr<PointToPointNetDevice>, Ptr<PointToPointNetDevice>>(src_p2p, dst_p2p));
	Simulator::Schedule(event, &MakeLinkFail, src_p2p, dst_p2p);
    }

    return ret;
}

int main (int argc, char *argv[]) {
    NodeContainer nodes;
    PointToPointHelper pointToPoint;
    DceManagerHelper dceManager;
    dceManager.SetTaskManagerAttribute( "FiberManagerType", StringValue ( "UcontextFiberManager" ) );
    dceManager.SetNetworkStack ("ns3::LinuxSocketFdFactory", "Library", StringValue ("liblinux.so"));

    NtfContent ntf("geant.ntf");
#ifdef CHECK
    NS_LOG_FUNCTION(ntf.nNodes());
    ntf.dumpNodes("geant.map");
#endif
    nodes.Create(ntf.nNodes());
    dceManager.Install (nodes);

    map<tuple<uint32_t, uint32_t>, uint32_t> metrics;

    // Create the topology
    for (vector<Link>::iterator links = ntf.getLinks(); links != ntf.getLastLink(); links++) {
	Link l = *links;
	// From NTF parser to NS3
	int n1_id = ntf.getNodeId(l.origin), n2_id = ntf.getNodeId(l.end);

	Ptr<Node> n1 = nodes.Get(n1_id), n2 = nodes.Get(n2_id);
	//pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("200Kbps"));
	pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("100Gbps"));
	pointToPoint.SetChannelAttribute ("Delay", StringValue (to_string(l.delay) + "ms"));
	// Install NetDevices on nodes and channel between them
	pointToPoint.Install(n1, n2);
	
	// Current iface_id is the last one added
	uint32_t n1_iface_id = n1->GetNDevices()-1, n2_iface_id = n2->GetNDevices()-1;

	// map <node_id, iface_id> = delay for BIRD config file
	metrics.insert(pair<tuple<uint32_t, uint32_t>, uint32_t>(make_tuple(n1_id, n1_iface_id), l.f_metric));
	metrics.insert(pair<tuple<uint32_t, uint32_t>, uint32_t>(make_tuple(n2_id, n2_iface_id), l.r_metric));

	ConfigureIface(n1, n1_iface_id);
	ConfigureIface(n2, n2_iface_id);
    }
    
    DceApplicationHelper dce;
    ApplicationContainer app;

    // Here, all the ifaces of each node is configured
    // We need to generate the bird config
    for (auto node = nodes.Begin(); node != nodes.End(); node++) {
	uint32_t node_id = (*node)->GetId(),
		 n_ifaces = (*node)->GetNDevices();

	string base_dir = "files-" + to_string(node_id), conf_dir = base_dir + "/etc", dev_dir = base_dir + "/dev";
	mkdir(conf_dir.c_str(), S_IRWXU | S_IRWXG);
	mkdir(dev_dir.c_str(), S_IRWXU | S_IRWXG);
	ofstream devnull(dev_dir + "/null");
	devnull.close();

	ofstream config(conf_dir + "/bird.conf");
	
	// Generic BIRD config
	config << "log \"/var/log/bird.log\" all;" <<endl;
	config << "debug protocols all;" <<endl;
	config << "debug latency on;" <<endl;
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

	LinuxStackHelper::RunIp (*node, Seconds (1), "l set dev lo up");

	dce.SetStackSize (1 << 20);
	dce.SetBinary ("bird");
	dce.ResetEnvironment();
	dce.ResetArguments ();
	dce.ParseArguments("-c /etc/bird.conf -s /var/run/bird.ctl");
	app = dce.Install (*node);
	app.Start (Seconds (5+0.5*node_id));
    }

#ifdef CHECK
    NS_LOG_FUNCTION("check");

    ofstream out("geant.check");

    Ptr<NetDevice> nd0, nd1;
    Ptr<PointToPointNetDevice> p2p_nd0;
    Ptr<PointToPointChannel> chan;

    // For all nodes
    for (auto node = nodes.Begin(); node != nodes.End(); node++) {
	// Iterate on all ifaces of current node
	for (uint32_t i=0; i<(*node)->GetNDevices(); i++) {
	    nd0 = (*node)->GetDevice(i); // current node iface
	    p2p_nd0 = DynamicCast<PointToPointNetDevice>(nd0); // get current p2p node iface
	    chan = DynamicCast<PointToPointChannel>(p2p_nd0->GetChannel()); // get p2p channel of current p2p iface
	    //nd1 = chan->GetDevice(0) == nd0 ? chan->GetDevice(1) : nd0; // get remote p2p iface
	    if (nd0 == chan->GetDevice(0))
		nd1 = chan->GetDevice(1);
	    else if (nd0 == chan->GetDevice(1))
		nd1 = chan->GetDevice(0);

	    out << (*node)->GetId() << "," << nd0->GetIfIndex() << "," << nd1->GetNode()->GetId() << "," << nd1->GetIfIndex() << "," << chan->GetDelay().GetMilliSeconds() << endl;
	    NS_LOG_FUNCTION((*node)->GetId() <<  nd0->GetIfIndex() << nd1->GetNode()->GetId() << nd1->GetIfIndex() << chan->GetDelay().GetMilliSeconds());
	}
    }
    out.close();
#endif

    // TODO: put callback for failure scenario

    //AnimationInterface anim ("geant-anim.xml");
    pointToPoint.EnablePcapAll("geant", true);

    auto ret = FindLinks(&nodes, 0, 1, 9, Seconds(120));
    /*for (auto it = ret.begin(); it != ret.end(); it++) {
	NS_LOG_FUNCTION(*it);
    	NS_LOG_FUNCTION(DynamicCast<NetDevice>(it->first)->GetNode()->GetId() << DynamicCast<NetDevice>(it->second)->GetNode()->GetId());
    }*/
    //nodes.Get(0)->GetDevice(0)->SetReceiveCallback(MakeCallback(&LsaProcessDelay));

    Simulator::Stop (Seconds (180));
    Simulator::Run ();
    Simulator::Destroy ();

  return 0;
}
