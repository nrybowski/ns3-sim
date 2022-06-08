# ns-3 sim

This repository contains the artefacts of the WNS3'22 paper ["Evaluating OSPF Convergence with ns-3 DCE"](https://doi.org/10.1145/3532577.3532597).

The [preprint is available here](http://hdl.handle.net/2078.1/260468).

## About

Link-state routing protocols are based on (i) a distributed database, the LSDB, and (ii) a common algorithm allowing a consensus in the choice of the routes.
Evaluating such Internal Gateway Protocols (IGPs) (e.g. OSPF or ISIS) is a quite difficult task.
One could deploy a physical testbed but it is costly for large topologies.
One could also emulate the protocols in virtualised networks but the outcome of the experiments will be hardware dependent, e.g. if all the network's nodes run on a same server.
Another solution is simulation.

In this work, we leveraged the [ns-3](https://www.nsnam.org/) discrete-time simulator, its [Direct Code Execution (DCE) extension](https://www.nsnam.org/about/projects/direct-code-execution/) and the [BIRD routing protocols framework](https://bird.network.cz/?index) to simulate the behavior of OSPF on medium-sized topologies, under various failure scenarios.

## How to use?

### Dependencies
Running this project require some dependencies:
- docker
- the rust ecosystem with cargo

### Wrapper
The whole project can be launched with the rust wrapper.
> cargo run --manifest-path=wrapper/Cargo.toml -- -n `<topology file>`
  
The topology file **must** be located in the `inputs` folder and called `<topology name>.ntf`.

The supported options are available with the `-h` flag, i.e.:
> cargo run --manifest-path=wrapper/Cargo.toml -- -h

Here is a summary of the currently supported options:
> -e, --ecmp <ecmp>            Enable ECMP.
>
> -f, --failures <failures>    NTF file specifying the link failures to simulate.
>
> -n, --ntf <ntf>              Topology name.
>
> -p, --pcap <pcap>            Enable PCAP logging.
>
> -r, --runtime <runtime>      Simulation duration in seconds. [default: 300]
>
> -s, --single <single>        Simulate the failure of each specified link one at a time. Used with -f
>
>     --spt <spt>              [default: 100]
>
>     --udp <udp>              Enable udp ping between nodes.

### Content

Soon TM.
