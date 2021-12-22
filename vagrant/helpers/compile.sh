#! /bin/bash

CURDIR=$(pwd)
cd /data/frr
CFLAGS="-fPIC -g -U_FORTIFY_SOURCE" LDFLAGS="-pie -rdynamic" ./configure \
	--prefix=/usr \
	--includedir=\${prefix}/include \
	--bindir=\${prefix}/bin \
	--sbindir=\${prefix}/lib/frr \
	--libdir=\${prefix}/lib/frr \
	--libexecdir=\${prefix}/lib/frr \
	--localstatedir=/var/run/frr \
	--sysconfdir=/etc/frr \
	--with-moduledir=\${prefix}/lib/frr/modules \
	--with-libyang-pluginsdir=\${prefix}/lib/frr/libyang_plugins \
	--enable-configfile-mask=0640 \
	--enable-logfile-mask=0640 \
	--enable-snmp=agentx \
	--enable-multipath=64 \
        --disable-capabilities \
        --disable-user \
        --disable-group \
	#--enable-user=ns3dce \
	#--enable-group=ns3dce \
	#--with-pkg-git-version
	#--enable-vty-group=frrvty \

make -j $(nproc) -C /data/frr
sudo make -j $(nproc) -C /data/frr install

cd "${CURDIR}"
