#include "NtfTopoGen.h"

NS_LOG_COMPONENT_DEFINE("NtfTopoHelper");

void LinkFailure(Ptr<PointToPointNetDevice> src, Ptr<PointToPointNetDevice> dst, uint32_t end) {
    Ptr<BlackholeErrorModel> em = CreateObject<BlackholeErrorModel>();
    if (end != 0) {
	//NS_LOG_FUNCTION(Seconds(end));
	//Simulator::Schedule(Seconds(end), MakeEvent(BlackholeErrorModel::Disable, em));
	Simulator::Schedule(Seconds(end), &Disable, em, Seconds(end), 1);
    }
    src->SetReceiveErrorModel(PointerValue(em));
    dst->SetReceiveErrorModel(PointerValue(em));
    /*dce.SetStackSize (1 << 20);
    dce.SetBinary ("ip");
    dce.ResetEnvironment();
    dce.ResetArguments ();
    dce.ParseArguments("l set dev sim"+src->GetIfIndex()+" down");
    app = dce.Install (src->GetNode(node_id));
    app.Start (Seconds (5+0.5*node_id));*/
}

void LinkFailure(Ptr<PointToPointNetDevice> src, Ptr<PointToPointNetDevice> dst) {
    /*Ptr<BlackholeErrorModel> em = CreateObject<BlackholeErrorModel>();
    src->SetReceiveErrorModel(PointerValue(em));
    dst->SetReceiveErrorModel(PointerValue(em));*/
    LinkFailure(src, dst, 0);
}

TopoHelper::TopoHelper(string ntf_file, bool check) {
    a = 1;
    this->ntf_file = ntf_file;
    this->check = check;

    // unsecure if file do not exist ? TODO: refactor
    ntf = new NtfContent(ntf_file);
    
    // unsecure. TODO: refactor
    filename = ntf_file.substr(0, ntf_file.find("."));

    dceManager.SetTaskManagerAttribute( "FiberManagerType", StringValue ( "UcontextFiberManager" ) );
    dceManager.SetNetworkStack ("ns3::LinuxSocketFdFactory", "Library", StringValue ("liblinux.so"));

    // Generate the topology on basis of an NTF file.
    TopoGen();

    // TODO: switch case on daemon kind
    // Configure and schedule BIRD daemon on each topology's node.
    ConfigureBird();

    NS_LOG_FUNCTION("Topology is ready to use");
}

TopoHelper::~TopoHelper() {
    //NS_LOG_FUNCTION("called");
    delete ntf;
}

/**
 * Configure node's iface with specific ip.
 */
string TopoHelper::ConfigureIface(Ptr<Node> node, unsigned int id) {
    string ip = "10.0.0." + to_string(a++) + "/32";
    //NS_LOG_FUNCTION(node->GetId() << id << ip);
    LinuxStackHelper::RunIp (node, Seconds (1), "a add " + ip + " dev sim" + to_string(id));
    LinuxStackHelper::RunIp (node, Seconds (1), "l set dev sim" + to_string(id) + " up");
    return ip;
}

/**
 * Find links such that
 *
 *     src_id --[delay]-> dst_id
 *
 * and schedule link failure at event.
 *
 * @param src_id The source node id.
 * @param dst_id The destination node id.
 * @param delay_value The link delay.
 * @param delay Delay at which the callback will be scheduled
 * @param callback A callback to execute on both PointToPointNetDevice of a PointToPointChannel.
 *
 * @return A vector containing pairs of PointToPointNetDevice matching the parameters.
 */
vector<pair<Ptr<PointToPointNetDevice>, Ptr<PointToPointNetDevice>>> TopoHelper::LinkCallback(uint32_t src_id,
	uint32_t dst_id, uint32_t delay_value, void (*callback)(Ptr<PointToPointNetDevice>, Ptr<PointToPointNetDevice>, uint32_t),
	uint32_t callback_delay, uint32_t callback_duration) {


    Ptr<Node> src_node = nodes.Get(src_id);

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
	//Simulator::Schedule(callback_delay, callback, src_p2p, dst_p2p);
	//uint32_t end = callback_duration == 0 ? 0 : callback_duration + callback_delay;
	Simulator::Schedule(Seconds(callback_delay), MakeEvent(callback, src_p2p, dst_p2p, callback_duration));
	dce.SetStackSize (1 << 20);
    	dce.SetBinary ("ip");
	//dce.SetBinary("tc");

    	dce.ResetEnvironment();
    	dce.ResetArguments ();
    	dce.ParseArguments("l set dev sim"+to_string(src_p2p->GetIfIndex())+" down");
	//dce.ParseArguments("qdisc add dev sim"+to_string(src_p2p->GetIfIndex())+" netem rate 0");
    	app = dce.Install (src_nd->GetNode());
    	app.Start (MilliSeconds (callback_delay*1000+15));
    	//app.Start (Seconds (callback_delay));

	dce.ResetEnvironment();
    	dce.ResetArguments ();
    	dce.ParseArguments("l set dev sim"+to_string(dst_p2p->GetIfIndex())+" down");
	//dce.ParseArguments("qdisc add dev sim"+to_string(dst_p2p->GetIfIndex())+" netem rate 0");
    	app = dce.Install (dst_nd->GetNode());
    	app.Start (MilliSeconds (callback_delay*1000+18));

    }

    return ret;
}

/*void TopoHelper::MakeLinkFail(uint32_t src_id, uint32_t dst_id, uint32_t delay_value, uint32_t delay) {
    //LinkCallback(src_id, dst_id, delay_value, delay, LinkFailure);
    LinkCallback(src_id, dst_id, delay_value, LinkFailure, delay);
}*/

void TopoHelper::MakeLinkFail(uint32_t src_id, uint32_t dst_id, uint32_t delay_value, uint32_t delay, uint32_t duration) {
    NS_LOG_FUNCTION("Scheduling link (" << src_id << "->" << dst_id << ") failure at " << delay);
    
    LinkCallback(src_id, dst_id, delay_value, LinkFailure, delay, duration);
}

void TopoHelper::TopoGen(void) {

    NS_LOG_FUNCTION("Generating topology defined in <" + ntf_file + ">");

    if (check)
	// Dump the mapping of node's names and node's ids
	ntf->dumpNodes(filename + ".map");

    nodes.Create(ntf->nNodes());
    dceManager.Install (nodes);
    //Ptr<Node> sink_node = nodes.Get(nodes.GetN()-1);

    // Create the topology
    for (vector<Link>::iterator links = ntf->getLinks(); links != ntf->getLastLink(); links++) {
	Link l = *links;
	// From NTF parser to NS3
	int n1_id = ntf->getNodeId(l.origin),
	    n2_id = ntf->getNodeId(l.end);

	// Get both ends of the link
	Ptr<Node> n1 = nodes.Get(n1_id),
		  n2 = nodes.Get(n2_id);

	// Set link's data rate and delay
	p2p.SetDeviceAttribute ("DataRate", StringValue ("100Gbps"));
	p2p.SetChannelAttribute ("Delay", StringValue (to_string(l.delay) + "ms"));

	// Install NetDevices on nodes and channel between them
	p2p.Install(n1, n2);
	
	// Current iface_id is the last one added
	uint32_t n1_iface_id = n1->GetNDevices()-1,
		 n2_iface_id = n2->GetNDevices()-1;

	// Map link metrics such that `<node_id, iface_id> = metric` for BIRD config file
	metrics.insert(pair<tuple<uint32_t, uint32_t>, uint32_t>(make_tuple(n1_id, n1_iface_id), l.f_metric));
	metrics.insert(pair<tuple<uint32_t, uint32_t>, uint32_t>(make_tuple(n2_id, n2_iface_id), l.r_metric));

	// Configure newly created ifaces
	//string ip1 = ConfigureIface(n1, n1_iface_id),
	//       ip2 = ConfigureIface(n2, n2_iface_id);
	string ip1 = "10.0.0." + to_string(a++) + "/32",
	       ip2 = "10.0.0." + to_string(a++) + "/32";
	LinuxStackHelper::RunIp (n1, Seconds (1), "a add " + ip1 + " peer " + ip2 + " dev sim" + to_string(n1_iface_id));
	//LinuxStackHelper::RunIp (n1, Seconds (1), "a add " + ip1 + " dev sim" + to_string(n1_iface_id));
	LinuxStackHelper::RunIp (n1, Seconds (1), "l set dev sim" + to_string(n1_iface_id) + " up");
	LinuxStackHelper::RunIp (n2, Seconds (1), "a add " + ip2 + " peer " + ip1 + " dev sim" + to_string(n2_iface_id));
	//LinuxStackHelper::RunIp (n2, Seconds (1), "a add " + ip2 + " dev sim" + to_string(n2_iface_id));
	LinuxStackHelper::RunIp (n2, Seconds (1), "l set dev sim" + to_string(n2_iface_id) + " up");
	NS_LOG_FUNCTION(n1->GetId() << n2->GetId() << ip1 << ip2);
    }

    /*Ptr<Node> node;
    for (uint32_t i=0; i<nodes.GetN(); i++) {
	node = nodes.Get(i);
	p2p.SetDeviceAttribute ("DataRate", StringValue ("100Gbps"));
	p2p.SetChannelAttribute ("Delay", StringValue("5ms"));
	p2p.Install(node, sink_node);
	uint32_t node_iface_id = node->GetNDevices()-1,
		 sink_iface_id = sink_node->GetNDevices()-1;
	LinuxStackHelper::RunIp (node, Seconds (1), "l set dev sim" + to_string(node_iface_id) + " up");
	LinuxStackHelper::RunIp (node, Seconds (1), "a add dev sim" + to_string(node_iface_id) + " 10.0.0." + to_string(a++) + "/32");
	LinuxStackHelper::RunIp (sink_node, Seconds (1), "l set dev sim" + to_string(sink_iface_id) + " up");
	LinuxStackHelper::RunIp (sink_node, Seconds (1), "a add dev sim" + to_string(sink_iface_id) + " 10.0.0." + to_string(a++) + "/32");
	
	//LinuxStackHelper::RunIp (node, Seconds (1), "l add stub type dummy"); // not supported by NS3
	//LinuxStackHelper::RunIp (node, Seconds (1), "l set dev stub up");
	//string stub_ip = "10.0.0." + to_string(a++) + "/32";
	//LinuxStackHelper::RunIp (node, Seconds (1), "a add dev stub " + stub_ip);*/
	/*for (uint32_t j=0; j<100; j++) {
		LinuxStackHelper::RunIp (node, Seconds (1), "r add 10." + to_string(i) +"."+to_string(j)+".0/24 via " + stub_ip);
	}*/
    //}
}

DceApplicationHelper *TopoHelper::GetDce(void) {
    return &dce;
}

ApplicationContainer *TopoHelper::GetApp(void) {
    return &app;
}

NodeContainer *TopoHelper::GetNodes(void) {
    return &nodes;
}

void TopoHelper::ConfigureBird(void) {

    NS_LOG_FUNCTION("Configuring BIRD daemons");
    Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();

    // Here, each node's ifaces are configured
    // We need to generate the bird config
    for (auto node = nodes.Begin(); node != nodes.End(); node++) {
	uint32_t node_id = (*node)->GetId(),
		 n_ifaces = (*node)->GetNDevices();

	// Directories and config file paths.
	string base_dir = "files-" + to_string(node_id),
	       conf_dir = base_dir + "/etc",
	       dev_dir = base_dir + "/dev";

	// Create required directories / files.
	mkdir(conf_dir.c_str(), S_IRWXU | S_IRWXG);
	mkdir(dev_dir.c_str(), S_IRWXU | S_IRWXG);
	ofstream devnull(dev_dir + "/null");
	devnull.close();

	string lo_addr = "10.0.1." + to_string(node_id+1);

	// OSPF config
	
	// Generic BIRD config
	// @see https://bird.network.cz/doc/bird-3.html for more details
	ofstream config(conf_dir + "/bird.conf");
	config << "log \"/var/log/bird.log\" all;" <<endl;
	config << "debug protocols all;" <<endl;
	config << "debug latency on;" <<endl;
	config << "router id " << lo_addr << ";" <<endl;
	config << "protocol device {}" <<endl;
	config << "protocol direct {\n\tdisabled;\n\tipv4;\n\tipv6;\n}" <<endl;
	config << "protocol kernel {\n\tipv4 {\n\t\texport filter {\n\t\t\tkrt_prefsrc=" << lo_addr << ";\n\t\t\taccept;\n\t\t};\n\t};\n}" <<endl;
	config << "protocol static {\n\tipv4;\n" <<endl;
	/*for (uint32_t j=0; j<100; j++) {
		config << "\t\troute 10." + to_string(node_id+1) + "." + to_string(j)+ ".0/24 via sim" + to_string(n_ifaces-1) +";" << endl;
	}*/
	config << "}\n" <<endl;

	// OSPF config
	map<tuple<uint32_t, uint32_t>, uint32_t>::iterator metric_data;
	uint32_t metric;
	//config << "protocol ospf v2 {\n\ttick 50000;\n\tarea 0 {" <<endl;
	config << "protocol ospf v2 {\n\tecmp no;\n\ttick 100000;\n\tarea 0 {" <<endl;
	for (uint32_t i=0; i<n_ifaces; i++) {
	    metric_data = metrics.find(make_tuple(node_id, i));
	    // 0 will cause invalid BIRD config and segfault during NS3 run
	    metric = metric_data == metrics.end() ? 0 : metric_data->second;
	    config << "\t\tinterface \"sim" << i <<"\" {" << endl;
	    config << "\t\t\ttype ptp;" <<endl;
	    config << "\t\t\tcost " << metric << ";" <<endl;
	    config << "\t\t\thello 5;\n\t\t};" <<endl;
	}
	config << "\t\tinterface \"lo\" {" << endl;
	config << "\t\t\ttype ptp;" <<endl;
	//config << "\t\t\tstub yes;" <<endl;
	config << "\t\t\thello 5;\n\t\t};" <<endl;

	/*config << "\t\tinterface \"sim\""+to_string(n_ifaces-1)+" {" << endl;
	config << "\t\t\tstub yes;\n\t\t};" <<endl;*/

	config << "\t};\n}" << endl;

	// End of BIRD config.
	config.close();

	// Set lo iface up.
	LinuxStackHelper::RunIp (*node, Seconds (1), "a add " + lo_addr +"/32 dev lo");
	LinuxStackHelper::RunIp (*node, Seconds (1), "l set dev lo up");

	// Configure BIRD daemon and schedule its start.
	dce.SetStackSize (1 << 20);
	dce.SetBinary ("bird");
	dce.ResetEnvironment();
	dce.ResetArguments ();
	dce.ParseArguments("-c /etc/bird.conf -s /var/run/bird.ctl");
	app = dce.Install (*node);
	app.Start (MilliSeconds (5000+(500*node_id)+rand->GetInteger()%100));
    }

    if (check) {
	NS_LOG_FUNCTION("Verifying generated topology");

	string out_file = filename + ".check";
	ofstream out(out_file);

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
		//NS_LOG_FUNCTION((*node)->GetId() <<  nd0->GetIfIndex() << nd1->GetNode()->GetId() << nd1->GetIfIndex() << chan->GetDelay().GetMilliSeconds());
	    }
	}
	out.close();

	string cmd = "python3 test.py " + out_file + " " + filename + ".map " + ntf_file;
	int ret = system(cmd.c_str());
	NS_ABORT_MSG_IF(!WIFEXITED(ret), "Some error occured during topology tests. Abort.");
	NS_ABORT_MSG_IF(WEXITSTATUS(ret) != 0, "Invalid generated topo! Abort.");
	NS_LOG_FUNCTION("Topology verified.");
    }
}

void TopoHelper::ScheduleFailures(string fail_path) {
    NtfContent failures(fail_path);
    failures.dumpLinks();
    for (vector<Link>::iterator links = failures.getLinks(); links != failures.getLastLink(); links++)
	MakeLinkFail(ntf->getNodeId((*links).origin), ntf->getNodeId((*links).end), (*links).delay,
		(*links).failure_start, (*links).failure_duration);
}

void TopoHelper::Run(uint32_t runtime, bool pcap) {
    if (pcap)
	p2p.EnablePcapAll(filename, true);
    Simulator::Stop (Seconds (runtime));
    //AnimationInterface anim (filename + ".xml");
    NS_LOG_FUNCTION("Starting simulation.");
    Simulator::Run ();
    Simulator::Destroy ();
    NS_LOG_FUNCTION("Simulation ended.");
}
