#include "ns3/NtfTopoGen.h"

string ntf;
uint32_t runtime = 180;
bool check = false;

int main (int argc, char *argv[]) {
    CommandLine cmd;
    cmd.AddValue ("ntf", "Path to the NTF topology", ntf);
    cmd.AddValue ("check", "Enable generated topology verification", check);
    cmd.AddValue ("runtime", "Total simulation duration", runtime);
    cmd.Parse (argc, argv);

    //AnimationInterface anim ("geant-anim.xml");

    TopoHelper topo(ntf, check);
    topo.Run(runtime);

  return 0;
}
