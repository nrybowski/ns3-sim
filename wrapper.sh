#! /bin/bash -e

#CXXFLAGS="-D CHECK" ./waf configure --prefix=/home/ns3dce/dce-linux-dev/build --with-ns3=/home/ns3dce/dce-linux-dev/build --with-elf-loader=/home/ns3dce/dce-linux-dev/build/lib --with-libaspect=/home/ns3dce/dce-linux-dev/build --enable-kernel-stack=/home/ns3dce/dce-linux-dev/source/dce-linux-dev/../net-next-nuse-4.4.0/arch --disable-python dir=/home/ns3dce/dce-linux-dev/source/dce-linux-dev --disable-tests --disable-examples
info() {
    echo -e "[INFO][wrapper] $*"
}

usage () {
    echo -e "Usage: ${0} -n|--ntf <input> -r|--runtime <input> [ OPTIONS ]
    where OPTIONS := { 
	-f|--failures <input> - NTF file specifying the link failures to simulate
	-s|--single - Simulate the failure of each specified link one at a time. Used with -f.
	-p|--pcap - Register PCAP traces for each node
    }"
}

NTF=""
SINGLE=0
RUNTIME=0
PCAP=0

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
	-rt|--runtime)
	    RUNTIME=${2}
	    shift 2
	    ;;
	-p|--pcap)
	    PCAP=1
	    shift
	    ;;
	*)
	    usage
	    exit 1
	    shift
	    ;;
	esac
done

[[ "${NTF}" == "" || ${RUNTIME} == 0 ]] && usage && exit 1;

run_ns3() {
    local ntf="${1}"
    local failures="${2}"

    info "Starting NS3 container. Giving hand to NS3."
    local cli="--ntf=${ntf}.ntf --check=true --runtime=${RUNTIME}"
    [[ ${failures} != "" ]] && cli="${cli} --failures=${failures}.ntf"
    [[ ${PCAP} -eq 1 ]] && cli="${cli} --pcap=true"

    docker run \
    	--rm\
	-v ${PWD}/output:/data/output\
	-v ${PWD}/bird:/data/bird\
	-v ${PWD}/myscripts:/home/ns3dce/dce-linux-dev/source/dce-linux-dev/myscripts\
	-v ${PWD}/inputs:/data/inputs\
	-v ${PWD}/udp_ping:/data/my_exe\
	-e BIRD_SPT_DELAY=${SPT_DELAY}\
	-e NS_LOG="NtfTopoHelper=all:BlackholeErrorModel=all"\
	-e DCE_PATH="/data/bird:/data/my_exe:${DCE_PATH}"\
	ns3-bird "${cli}"
	#ns3-bird "--ntf=${ntf}.ntf --check=true --runtime=${RUNTIME} $([[ ${failures} != "" ]] && echo "--failures=${failures}.ntf") $([[ ${PCAP} -eq 1 ]] && echo "--pcap=true")"
    info "NS3 finished."
}

save_output() {
    local dest=${1}

    if [[ -d "${dest}" ]]
    then
	info "${dest} already exists. Do you want to replace its content? [y/N]"
	read erase
	if [[ "${erase}" == "y" ]]
	then
	    rm -r "${dest}"/*
	    mv output/* "${dest}"
	else
	    info "Skip save."
	fi
    else
	mkdir -pv "${dest}"
	mv output/* "${dest}"
    fi
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
    save_output "saves/${FAILURES}-single/${count}" 
    rm "${tmp}"
}

info "Hello from NS3 wrapper."
info "Re-building NS3 containers."
./containers/build.sh

if [[ ${SINGLE} -eq 1 && "${FAILURES}" != "" ]]
then
    COUNT=0
    while read LINE <&4
    do
	simulate_line "${LINE}" "${COUNT}"
	((COUNT=COUNT+1))
    done 4< "inputs/${FAILURES}.ntf"
else
    run_ns3 "${NTF}" "${FAILURES}"
    save_output "saves/${FAILURES}" 
    #post_process "${NTF}"
fi

info "End of wrapper"
