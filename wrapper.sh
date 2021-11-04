#! /bin/bash -e

#CXXFLAGS="-D CHECK" ./waf configure --prefix=/home/ns3dce/dce-linux-dev/build --with-ns3=/home/ns3dce/dce-linux-dev/build --with-elf-loader=/home/ns3dce/dce-linux-dev/build/lib --with-libaspect=/home/ns3dce/dce-linux-dev/build --enable-kernel-stack=/home/ns3dce/dce-linux-dev/source/dce-linux-dev/../net-next-nuse-4.4.0/arch --disable-python dir=/home/ns3dce/dce-linux-dev/source/dce-linux-dev --disable-tests --disable-examples
info() {
    echo -e "[INFO][wrapper] $*"
}

info "Hello from NS3 wrapper."
info "Re-building NS3 container if required."

docker build -t ns3 -f ns3.containerfile . > /dev/null 2>&1

info "Starting NS3 container. Giving hand to NS3."

docker run \
    --rm\
    -v ${PWD}/output:/data/output\
    -v ${PWD}/bird:/data/bird\
    -v ${PWD}/helpers:/data/helpers\
    -v ${PWD}/myscripts:/home/ns3dce/dce-linux-dev/source/dce-linux-dev/myscripts\
    -v ${PWD}/inputs:/data/inputs\
    -e NS_LOG="NtfTopoHelper=all"\
    -e DCE_PATH="/data/bird:${DCE_PATH}"\
    ns3 "--ntf=geant.ntf --check=true --runtime=180 --failures=failures.ntf"

info "NS3 finished. Post processing outputs."

CURDIR="${PWD}"
cd output/traces

NODES=$(wc -l <geant.map)
for i in $(seq 0 $((${NODES}-1)))
do
    mergecap geant-"${i}"-* -w "geant-$i.pcap"
done

cd "${CURDIR}"

info "End of wrapper"
