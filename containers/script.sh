#! /bin/bash

info () {
    echo -e "[INFO][container] $*"
}

cp myscripts/bird/*.py .
cp /data/inputs/* .

info "Re-compiling bird."
CURDIR="${PWD}"
if [[ ! -f /data/bird/Makefile ]]
then
       cd /data/bird
       autoreconf
       CFLAGS="-fPIC -D NS3 \
	       -D LSP_PD_MIN=2000\
	       -D LSP_PD_MAX=4000\
	       -D SPT_PD_MIN=2000\
	       -D SPT_PD_MAX=4000\
	       -D P_PD_MIN=100\
	       -D P_PD_MAX=110\
       " LDFLAGS="-rdynamic -pie" ./configure --disable-client
       cd "${CURDIR}"
fi
make -C /data/bird -j $(nproc)
rm /data/my_exe/udp_client /data/my_exe/udp_server
make -C /data/my_exe -j $(nproc)

info "Running NS3."
time ./waf --disable-tests --disable-examples --run dce-ntf-bird --command-template "%s ${1}"

info "Moving pcap traces."
if [[ ! -d /data/output/traces ]]; then mkdir /data/output/traces; fi
mv -f *.pcap *.map /data/output/traces > /dev/null 2>&1

info "Moving node files."
if [[ ! -d /data/output/files ]]; then mkdir /data/output/files; else rm -rf /data/output/files/*; fi
mv -f files-* /data/output/files > /dev/null 2>&1

#info ""
mv -f *.check *.xml /data/output > /dev/null 2>&1

info "Leaving container."
