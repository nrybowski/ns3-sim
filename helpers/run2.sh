#! /bin/bash


#CXXFLAGS="-D CHECK" ./waf configure --prefix=/home/ns3dce/dce-linux-dev/build --with-ns3=/home/ns3dce/dce-linux-dev/build --with-elf-loader=/home/ns3dce/dce-linux-dev/build/lib --with-libaspect=/home/ns3dce/dce-linux-dev/build --enable-kernel-stack=/home/ns3dce/dce-linux-dev/source/dce-linux-dev/../net-next-nuse-4.4.0/arch --disable-python dir=/home/ns3dce/dce-linux-dev/source/dce-linux-dev --disable-tests --disable-examples

vagrant ssh -c '
    if [[ ! -f /var/run/bird.ctl ]]; then touch /var/run/bird.ctl; chown ns3dce:ns3dce /var/run/bird.ctl; fi &&\
    sudo su - ns3dce -c "
	export NS_LOG=\"GEANT=all\" &&\
	export DCE_PATH=\"/data/bird:${DCE_PATH}\" &&\
	CURDIR=\"${PWD}\" &&\
	cd /home/ns3dce/dce-linux-dev/source/dce-linux-dev &&\
	./waf --run dce-bird-geant &&\
	if [[ ! -d /data/output/traces ]]; then mkdir /data/output/traces; else rm -r /data/output/traces/*; fi &&\
	mv -f *.pcap geant.map /data/output/traces > /dev/null 2>&1 &&\
	if [[ ! -d /data/output/files ]]; then mkdir /data/output/files; else rm -r /data/output/files/*; fi &&\
	mv files-* /data/output/files/ > /dev/null 2>&1 &&\
	mv -f geant.check geant-anim.xml /data/output > /dev/null 2>&1 &&\
	cd \"${CURDIR}\" \
    "
'


CURDIR="${PWD}"
cd ../output/traces

NODES=$(wc -l <geant.map)
for i in $(seq 0 $((${NODES}-1)))
do
    mergecap geant-"${i}"-* -w "geant-$i.pcap"
done

cd "${CURDIR}"
