#ifndef NTF_TOPO_GEN_H
#define NTF_TOPO_GEN_H

#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/dce-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

#include <memory> // unique_ptr
#include <iostream>
#include <iomanip>
#include <fstream>

#include "ns3/NtfParser.h"

#include "ns3/netanim-module.h"

#include "blackhole-error-model.h"

using namespace std;
using namespace ns3;

/** @file */

/**
 * Simulate link failure by filtering out all packets received on PointToPointChannel devices.
 *
 * @param src The source NetDevice of the link to fail.
 * @param dst The dsetination NetDevice of the link to fail.
 */
void LinkFailure(Ptr<PointToPointNetDevice> src, Ptr<PointToPointNetDevice> dst);

class TopoHelper {
    private:
	// NS3 objects
	NodeContainer nodes;
	PointToPointHelper p2p;
	DceManagerHelper dceManager;
	DceApplicationHelper dce;
	ApplicationContainer app;

	// NTF topology related fields
        map<tuple<uint32_t, uint32_t>, uint32_t> metrics; /**< Mapping between links and metrics: <src_id, dst_id> = metric */
	string ntf_file, /**< Path to the NTF topology file. */
	       filename; /**< Name derived from the NTF file. @see ntf_file. */
	NtfContent *ntf;

	bool check; /**< Indicate whether the generated topology must be verified. */
	unsigned int four; /**< IP address counter. @see ConfigureIface. */
	unsigned int three; /**< IP address counter. @see ConfigureIface. */

	// Methods
	string ConfigureIface(Ptr<Node> node, unsigned int id);
	void TopoGen(void);
	void ConfigureBird(uint32_t spt_delay, bool ecmp);
	vector<pair<Ptr<PointToPointNetDevice>, Ptr<PointToPointNetDevice>>> LinkCallback(uint32_t src_id, uint32_t dst_id,
		uint32_t delay_value, void (*callback)(Ptr<PointToPointNetDevice>, Ptr<PointToPointNetDevice>, uint32_t),
		uint32_t callback_delay, uint32_t callback_duration);

    public:
	TopoHelper(string ntf_file, bool check, uint32_t spt_delay, bool ecmp);
	~TopoHelper();
	void MakeLinkFail(uint32_t src_id, uint32_t dst_id, uint32_t delay_value, uint32_t delay, uint32_t duration);
	void MakeLinkFail(uint32_t src_id, uint32_t dst_id, uint32_t delay_value, uint32_t delay);
	void Run(uint32_t runtime, bool pcap);
	void ScheduleFailures(string fail_path);
	NodeContainer *GetNodes(void);
	ApplicationContainer *GetApp(void);
	DceApplicationHelper *GetDce(void);
};

#endif //NTF_TOPO_GEN_H
