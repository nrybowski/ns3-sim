#! /bin/bash -e

#CXXFLAGS="-D CHECK" ./waf configure --prefix=/home/ns3dce/dce-linux-dev/build --with-ns3=/home/ns3dce/dce-linux-dev/build --with-elf-loader=/home/ns3dce/dce-linux-dev/build/lib --with-libaspect=/home/ns3dce/dce-linux-dev/build --enable-kernel-stack=/home/ns3dce/dce-linux-dev/source/dce-linux-dev/../net-next-nuse-4.4.0/arch --disable-python dir=/home/ns3dce/dce-linux-dev/source/dce-linux-dev --disable-tests --disable-examples
info() {
    echo -e "[INFO][wrapper] $*"
}

NTF="${1}"
FAILURES="${2}"

info "Hello from NS3 wrapper."
info "Re-building NS3 container if required."

#docker build -t ns3 -f ns3.containerfile . > /dev/null 2>&1
docker build -t ns3-base -f ns3-base.containerfile .
docker build -t ns3 -f ns3.containerfile . 

info "Starting NS3 container. Giving hand to NS3."

docker run \
    --rm\
    -v ${PWD}/output:/data/output\
    -v ${PWD}/bird:/data/bird\
    -v ${PWD}/helpers:/data/helpers\
    -v ${PWD}/myscripts:/home/ns3dce/dce-linux-dev/source/dce-linux-dev/myscripts\
    -v ${PWD}/inputs:/data/inputs\
    -v ${PWD}/my_exe:/data/my_exe\
    -e NS_LOG="NtfTopoHelper=all:BlackholeErrorModel=all"\
    -e DCE_PATH="/data/bird:/data/my_exe:${DCE_PATH}"\
    ns3 "--ntf=${NTF}.ntf --check=true --runtime=300 --failures=${FAILURES}.ntf"
    #ns3 "--ntf=geant.ntf --check=true --runtime=300 --failures=failures.ntf"

info "NS3 finished. Post processing outputs."

CURDIR="${PWD}"
cd output/traces

NODES=$(wc -l <"${NTF}.map")
for i in $(seq 0 $((${NODES}-1)))
do
    mergecap "${NTF}-${i}"-* -w "${NTF}-$i.pcap"
done

cd "${CURDIR}"

info "End of wrapper"
