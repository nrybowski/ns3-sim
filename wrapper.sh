#! /bin/bash -e

#CXXFLAGS="-D CHECK" ./waf configure --prefix=/home/ns3dce/dce-linux-dev/build --with-ns3=/home/ns3dce/dce-linux-dev/build --with-elf-loader=/home/ns3dce/dce-linux-dev/build/lib --with-libaspect=/home/ns3dce/dce-linux-dev/build --enable-kernel-stack=/home/ns3dce/dce-linux-dev/source/dce-linux-dev/../net-next-nuse-4.4.0/arch --disable-python dir=/home/ns3dce/dce-linux-dev/source/dce-linux-dev --disable-tests --disable-examples
info() {
    echo -e "[INFO][wrapper] $*"
}

usage () {
    echo -e "Usage: ${0} -n|--ntf <input> [ OPTIONS ]
    where OPTIONS := { 
	-f|--failures <input>
	-s|--single
    }"
}

NTF=""
SINGLE=0

# Parse CLI options
while [[ ${#} -gt 0 ]]
do
	case "$1" in
	    -n|--ntf)
	    NTF="${2}"
	    shift 2
	    ;;
	    -f|--failures)
	    FAILURES="${2}"
	    shift 2
	    ;;
	    -s|--single)
	    SINGLE=1
	    shift
	    ;;
	    *)
	    usage
	    exit 1
	    shift
	    ;;
	esac
done

[[ "${NTF}" == "" ]] && usage && exit 1;

run_ns3() {
    local ntf="${1}"
    local failures="${2}"

    info "Starting NS3 container. Giving hand to NS3."
    docker run \
    	--rm\
	-v ${PWD}/output:/data/output\
	-v ${PWD}/bird:/data/bird\
	-v ${PWD}/myscripts:/home/ns3dce/dce-linux-dev/source/dce-linux-dev/myscripts\
	-v ${PWD}/inputs:/data/inputs\
	-v ${PWD}/udp_ping:/data/my_exe\
	-e NS_LOG="NtfTopoHelper=all:BlackholeErrorModel=all"\
	-e DCE_PATH="/data/bird:/data/my_exe:${DCE_PATH}"\
	ns3-bird "--ntf=${ntf}.ntf --check=true --runtime=65 $([[ ${failures} != "" ]] && echo "--failures=${failures}.ntf")"
    info "NS3 finished."
}

post_process() {
    info "Post processing outputs."
    CURDIR="${PWD}"
    cd output/traces

    NODES=$(wc -l <"${NTF}.map")
    for i in $(seq 0 $((${NODES}-1)))
    do
	mergecap "${NTF}-${i}"-* -w "${NTF}-$i.pcap"
    done
    cd "${CURDIR}"
}

simulate_line() {
    local line="${1}"
    local tmp=$(mktemp -p inputs/ tmp.XXXXXX.ntf)
    local count=${2}

    echo "${line}" > "${tmp}"
    run_ns3 "${NTF}" $(echo "${tmp}" | sed -E "s/^inputs\///g;s/\.ntf//g")
    local dest="saves/${FAILURES}-single/${count}" 
    mkdir -pv "${dest}"
    mv output/* "${dest}"
    rm "${tmp}"
}

info "Hello from NS3 wrapper."
info "Re-building NS3 containers."
./containers/build.sh

if [[ ${SINGLE} -eq 1 && "${FAILURES}" != "" ]]
then
    COUNT=0
    while read line
    do
	simulate_line "${line}" "${COUNT}"
	((COUNT=COUNT+1))
    done < "inputs/${FAILURES}.ntf"
else
    run_ns3 "${NTF}" "${FAILURES}"
    DEST="saves/${FAILURES}" 
    mkdir -pv "${DEST}"
    mv output/* "${DEST}"

    #post_process "${NTF}"
fi

info "End of wrapper"
