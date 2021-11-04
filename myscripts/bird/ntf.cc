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
    topo.Run(runtime);

  return 0;
}
