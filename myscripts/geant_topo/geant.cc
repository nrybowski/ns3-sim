#include "ns3/NtfTopoGen.h"

int main (int argc, char *argv[]) {

    //AnimationInterface anim ("geant-anim.xml");

    TopoHelper topo("geant.ntf", true);
    topo.Run(180);

  return 0;
}
