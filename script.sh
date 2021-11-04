#! /bin/bash

info () {
    echo -e "[INFO][container] $*"
}

cp myscripts/bird/test.py .
cp /data/inputs/* .

info "Re-compiling bird."
make -C /data/bird -j $(nproc)

info "Running NS3."
./waf --run dce-ntf-bird --command-template "%s ${1}"

info "Moving pcap traces."
if [[ ! -d /data/output/traces ]]; then mkdir /data/output/traces; fi
mv -f *.pcap geant.map /data/output/traces > /dev/null 2>&1

info "Moving node files."
if [[ ! -d /data/output/files ]]; then mkdir /data/output/files; else rm -rf /data/output/files/*; fi
mv -f files-* /data/output/files > /dev/null 2>&1

#info ""
mv -f geant.check /data/output > /dev/null 2>&1

info "Leaving container."
