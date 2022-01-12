#include "ns3/NtfTopoGen.h"

string ntf;
uint32_t runtime = 180;
bool check = true;
string failures;
bool pcap = false;
uint32_t spt_delay = 100;

int main (int argc, char *argv[]) {
    CommandLine cmd;
    cmd.AddValue ("ntf", "Path to the NTF topology", ntf);
    cmd.AddValue ("check", "Enable generated topology verification", check);
    cmd.AddValue ("runtime", "Total simulation duration", runtime);
    cmd.AddValue ("failures", "Path to the NTF failure file", failures);
    cmd.AddValue ("pcap", "Register PCAP outputs", pcap);
    cmd.AddValue ("spt_delay", "SPT computation delay [ms]", spt_delay);
    cmd.Parse (argc, argv);
    spt_delay *= 1000;

    TopoHelper topo(ntf, check, spt_delay);
    if (strcmp(failures.c_str(), "") != 0)
	topo.ScheduleFailures(failures);

    NodeContainer *nodes = topo.GetNodes();
    DceApplicationHelper *dce = topo.GetDce();
    ApplicationContainer app;

    /*string spfs_cmd = "python3.7 spfs.py " + ntf;
    int ret = system(spfs_cmd.c_str());
    NS_ABORT_MSG_IF(!WIFEXITED(ret), "Some error occured during SPFs computation");
    NS_ABORT_MSG_IF(WEXITSTATUS(ret) != 0, "Invalid SPFs computation");

    // Open leaves file
    string spfs_in = "udp.ping";
    FILE *spfs = fopen(spfs_in.c_str(), "r");
    if (!spfs) {
	    cout << "Can't open " << spfs_in << " file." << endl;
	    exit(1);
    }

    size_t len;
    char *buf = NULL, *token, *in;
    int iteration = 0;
    while (getline(&buf, &len, spfs) != -1) {
	printf("%i %s", iteration, buf);
	for (in = buf; ;in = NULL) {
	    token = strtok(in, ",");
	    if (token == NULL)
		break;
	    printf("%s\n", token);
	    dce->SetStackSize(1 << 20);
	    dce->SetBinary("udp_client");
	    dce->ResetArguments();
	    dce->ParseArguments(token);
	    app = dce->Install(nodes->Get(iteration));
	    app.Start(Seconds(59));
	}
	iteration++;
    }

    free(buf);
    fclose(spfs);*/

    /*dce->SetStackSize(1 << 20);
    dce->SetBinary("ip");
    dce->ResetArguments();
    dce->ParseArguments("r");
    app = dce->Install(nodes->Get(13));
    app.Start(MilliSeconds(160000));

    dce->SetStackSize(1 << 20);
    dce->SetBinary("ip");
    dce->ResetArguments();
    dce->ParseArguments("r");
    app = dce->Install(nodes->Get(13));
    app.Start(MilliSeconds(160500));*/

    for (int i = 0; i < nodes->GetN(); i++) {
	// Launch UDP server on each node
	//dce->SetStackSize(1 << 20);
    	//dce->SetBinary("udp_server");
    	//dce->ResetArguments();
    	//app = dce->Install(nodes->Get(i));
    	//app.Start(Seconds(30));

	dce->SetStackSize(1 << 20);
    	dce->SetBinary("ip");
    	dce->ResetArguments();
	dce->ParseArguments("r");
    	app = dce->Install(nodes->Get(i));
    	app.Start(Seconds(60));
    }
    
    /*for (int i = 0; i < 6; i++) {
	dce->SetStackSize(1 << 20);
    	dce->SetBinary("ip");
    	dce->ResetArguments();
	dce->ParseArguments("r");
    	app = dce->Install(nodes->Get(i));
    	app.Start(Seconds(55));
    }

    for (int i = 0; i < 6; i++) {
	dce->SetStackSize(1 << 20);
    	dce->SetBinary("ip");
    	dce->ResetArguments();
	dce->ParseArguments("r");
    	app = dce->Install(nodes->Get(i));
    	app.Start(Seconds(250));
    }*/

	/*dce->SetStackSize(1 << 20);
    	dce->SetBinary("birdcl");
    	dce->ResetArguments();
	dce->ParseArguments("-s /var/run/bird.ctl show protocols all");
    	app = dce->Install(nodes->Get(1));
    	app.Start(Seconds(85));*/


    topo.Run(runtime, pcap);

  return 0;
}
