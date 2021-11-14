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


    TopoHelper topo(ntf, check);
    if (strcmp(failures.c_str(), "") != 0)
	topo.ScheduleFailures(failures);

    NodeContainer *nodes = topo.GetNodes();
    DceApplicationHelper *dce = topo.GetDce();
    ApplicationContainer app;

    dce->SetStackSize(1 << 20);
    dce->SetBinary("udp_server");
    dce->ResetArguments();
    app = dce->Install(nodes->Get(2));
    app.Start(Seconds(30));

    for (int i = 0; i < 6; i++) {
	if (i == 2) continue;
	dce->SetStackSize(1 << 20);
   	dce->SetBinary("udp_client");
    	dce->ResetArguments();
    	app = dce->Install(nodes->Get(i));
    	app.Start(Seconds(85));
    }
    
    for (int i = 0; i < 6; i++) {
	dce->SetStackSize(1 << 20);
    	dce->SetBinary("ip");
    	dce->ResetArguments();
	dce->ParseArguments("r");
    	app = dce->Install(nodes->Get(i));
    	app.Start(Seconds(85));
    }

    for (int i = 0; i < 6; i++) {
	dce->SetStackSize(1 << 20);
    	dce->SetBinary("ip");
    	dce->ResetArguments();
	dce->ParseArguments("r");
    	app = dce->Install(nodes->Get(i));
    	app.Start(Seconds(250));
    }

    topo.Run(runtime);

  return 0;
}
