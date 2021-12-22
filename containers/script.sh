#! /bin/bash

info () {
    echo -e "[INFO][container] $*"
}

cp myscripts/bird/*.py .
ls
cp /data/inputs/* .

info "Re-compiling bird."
CURDIR="${PWD}"
if [[ ! -f /data/bird/Makefile ]]
then
       cd /data/bird
       autoreconf
       CFLAGS="-fPIC -D NS3" LDFLAGS="-rdynamic -pie" ./configure --disable-client
       cd "${CURDIR}"
fi
make -C /data/bird -j $(nproc)
rm /data/my_exe/udp_client /data/my_exe/udp_server
make -C /data/my_exe -j $(nproc)

info "Running NS3."
time ./waf --run dce-ntf-bird --command-template "%s ${1}"

info "Moving pcap traces."
if [[ ! -d /data/output/traces ]]; then mkdir /data/output/traces; fi
mv -f *.pcap *.map /data/output/traces > /dev/null 2>&1

info "Moving node files."
if [[ ! -d /data/output/files ]]; then mkdir /data/output/files; else rm -rf /data/output/files/*; fi
mv -f files-* /data/output/files > /dev/null 2>&1

#info ""
mv -f *.check *.xml /data/output > /dev/null 2>&1

info "Leaving container."
