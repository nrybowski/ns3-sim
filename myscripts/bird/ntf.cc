#include "ns3/NtfTopoGen.h"

string ntf;
uint32_t runtime = 180;
bool check = true;
string failures;

int main (int argc, char *argv[]) {
    CommandLine cmd;
    cmd.AddValue ("ntf", "Path to the NTF topology", ntf);
    cmd.AddValue ("check", "Enable generated topology verification", check);
    cmd.AddValue ("runtime", "Total simulation duration", runtime);
    cmd.AddValue ("failures", "Path to the NTF failure file", failures);
    cmd.Parse (argc, argv);

    //AnimationInterface anim ("geant-anim.xml");

    TopoHelper topo(ntf, check);
    if (strcmp(failures.c_str(), "") != 0)
	topo.ScheduleFailures(failures);

    NodeContainer *nodes = topo.GetNodes();
    DceApplicationHelper *dce = topo.GetDce();
    ApplicationContainer app;

    dce->SetStackSize(1 << 20);
    dce->SetBinary("udp_server");
    dce->ResetArguments();
    app = dce->Install(nodes->Get(18));
    app.Start(Seconds(30));

    dce->SetStackSize(1 << 20);
    dce->SetBinary("udp_client");
    dce->ResetArguments();
    app = dce->Install(nodes->Get(0));
    app.Start(Seconds(60));

    topo.Run(runtime);

  return 0;
}
