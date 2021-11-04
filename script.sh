#! /bin/bash

cp myscripts/bird/test.py .
cp /data/inputs/* .
./waf --run dce-ntf-bird --command-template "%s ${1}"
if [[ ! -d /data/output/traces ]]; then mkdir /data/output/traces; fi
mv -f *.pcap geant.map /data/output/traces > /dev/null 2>&1
if [[ ! -d /data/output/files ]]; then mkdir /data/output/files; else rm -rf /data/output/files/*; fi
mv -f files-* /data/output/files > /dev/null 2>&1
mv -f geant.check /data/output > /dev/null 2>&1
