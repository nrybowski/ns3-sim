# -*- mode: ruby -*-
# vi: set ft=ruby :

# All Vagrant configuration is done below. The "2" in Vagrant.configure
# configures the configuration version (we support older styles for
# backwards compatibility). Please don't change it unless you know what
# you're doing.
Vagrant.configure("2") do |config|
  # The most common configuration options are documented and commented below.
  # For a complete reference, please see the online documentation at
  # https://docs.vagrantup.com.

  # Every Vagrant development environment requires a box. You can search for
  # boxes at https://vagrantcloud.com/search.
  config.vm.box = "generic/ubuntu1604"
  config.vm.synced_folder "frr", "/data/frr", type: "nfs", nfs_udp: "false"
  config.vm.synced_folder "bird", "/data/bird", type: "nfs", nfs_udp: "false"
  config.vm.synced_folder "libyang", "/data/libyang", type: "nfs", nfs_udp: "false"
  config.vm.synced_folder "libcap", "/data/libcap", type: "nfs", nfs_udp: "false"
  config.vm.synced_folder "helpers", "/data/helpers", type: "nfs", nfs_udp: "false"
  config.vm.synced_folder "myscripts", "/home/ns3dce/dce-linux-dev/source/ns-3-dce/myscripts", type: "nfs", nfs_udp: "false"

  config.vm.provider "libvirt" do |lb|
    lb.memory = "4096"
    lb.cpus = 4
  end

  # Enable provisioning with a shell script. Additional provisioners such as
  # Ansible, Chef, Docker, Puppet and Salt are also available. Please see the
  # documentation for more information about their specific syntax and use.
  #config.vm.provision "shell", inline: <<-SHELL
  #  apt-get update && apt-get upgrade
  #  apt-get install -y g++ python3 git tar python3-pip cmake mercurial \
  #    python3-gi-cairo gir1.2-gtk-3.0 libc6 libc6-dbg libpcap-dev \
  #    python3-gi python-gi python3-pygraphviz python-pygraphviz htop \
  #    python-gi-cairo g++-multilib qt5-default
  #  su - vagrant
  #  pip3 install distro
  #  if [[ ! -d ns-allinone-3.34 ]]
  #  then
  #    wget https://www.nsnam.org/release/ns-allinone-3.34.tar.bz2
  #    tar xjf ns-allinone-3.34.tar.bz2
  #  fi
  #  cd ns-allinone-3.34
  #  ./build.py --enable-examples --enable-tests
  #  export PATH=$PATH:`pwd`/build/bin:`pwd`/build/bin_dce
  #  export PYTHONPATH=`pwd`/build/lib${PYTHONPATH:+:$PYTHONPATH}
  #  export LD_LIBRARY_PATH=`pwd`/build/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
  #  export DCE_PATH=`pwd`/build/bin_dce:`pwd`/build/sbin
  #  cd bake
  #  ./bake.py configure -e dce-ns3-dev
  #  ./bake.py check
  #  ./bake.py show
  #  ./bake.py download
  #  su - vagrant -- ./bake.py build
  #SHELL
  #
  config.vm.provision "file", source: "bakeconf.xml.patch", destination: "/home/vagrant/bakeconf.xml.patch"
  config.vm.provision "file", source: "bakeconf.xml.test", destination: "/home/vagrant/bakeconf.xml.test"


  config.vm.provision "shell", inline: <<-SHELL
    # NS3-DCE install
    apt-get update && apt-get install --no-install-recommends -y mercurial libexpat1-dev git-core build-essential bison \
    flex libssl-dev libdb-dev libpcap-dev libc6-dbg libsysfs-dev gawk indent \
    pkg-config autoconf automake sudo ccache libsaxonb-java openjdk-8-jre-headless python-setuptools \
    bc wget python-pip software-properties-common apt-transport-https ca-certificates && \
    rm -rf /var/lib/apt/lists/*

    # install g++-4.9
    add-apt-repository -y ppa:ubuntu-toolchain-r/test && apt-get update && \
      apt-get install -y g++-4.9

    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.9 10 && \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.9 20 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.9 10 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.9 20 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.9 10 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.9 20 && \
    rm /usr/bin/cpp && \
    update-alternatives --install /usr/bin/cpp cpp /usr/bin/cpp-4.9 10 && \
    update-alternatives --install /usr/bin/cpp cpp /usr/bin/cpp-4.9 20 && \
    update-alternatives --install /usr/bin/cc cc /usr/bin/gcc 30 && \
    update-alternatives --set cc /usr/bin/gcc && \
    update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 30 && \
    update-alternatives --set c++ /usr/bin/g++

    useradd -ms /bin/bash ns3dce && adduser ns3dce sudo && echo -n 'ns3dce:ns3dce' | chpasswd

    # Enable passwordless sudo for users under the "sudo" group
    sed -i.bkp -e \
      's/%sudo\s\+ALL=(ALL\(:ALL\)\?)\s\+ALL/%sudo ALL=NOPASSWD:ALL/g' \
      /etc/sudoers

    pip install --upgrade pip==9.0.3
    pip install setuptools-scm==5.0.2

    su - ns3dce -c "
      cd /home/ns3dce &&\
      [[ ! -d dce-linux-dev ]] && mkdir -p /home/ns3dce/dce-linux-dev ;\
      cd /home/ns3dce/dce-linux-dev &&\
      export PATH=/usr/lib/ccache:${PATH} &&\
      [[ ! -d bake ]] && git clone https://github.com/thehajime/bake ;\
      echo done # Quick fix to ignore retunr value of condition
    "

    mv /home/vagrant/bakeconf* /home/ns3dce/dce-linux-dev/bake
    chown ns3dce:ns3dce /home/ns3dce/dce-linux-dev/bake/bakeconf*

    su - ns3dce -c "
      export PATH=/usr/lib/ccache:${PATH} &&\
      cd /home/ns3dce/dce-linux-dev/bake &&\
      diff bakeconf.xml bakeconf.xml.test ;\
      git apply --whitespace=warn < bakeconf.xml.patch ;\
      cd .. &&\
      #./bake/bake.py configure -e dce-linux-dev &&\
      #./bake/bake.py download &&\
      #./bake/bake.py build -j $(nproc)
    "
    
    # FRR install
    groupadd -r -g 92 frr && groupadd -r -g 85 frrvty
    #adduser --system --ingroup frr --home /var/run/frr/ \
    #  --gecos "FRR suite" --shell /sbin/nologin frr
    adduser --system --ingroup frr --home /var/run/frr/ \
      --gecos "FRR suite" --shell /bin/bash frr
    usermod -a -G frrvty frr
    usermod -aG sudo frr

    apt-get update
    apt-get install -y \
       git autoconf automake libtool make libreadline-dev texinfo \
       pkg-config libpam0g-dev libjson-c-dev bison flex python3-pytest \
       libc-ares-dev python3-dev python-ipaddress python3-sphinx \
       install-info build-essential libsnmp-dev perl libcap-dev \
       libelf-dev cmake libpcre2-dev

    su - ns3dce -c "
      [[ ! -d libyang ]] && git clone https://github.com/CESNET/libyang.git ;\
      cd libyang &&\
      git checkout v2.0.88 &&\
      [[ ! -d build ]] && mkdir build ;\
      cd build &&\
      cmake -D CMAKE_INSTALL_PREFIX:PATH=/usr \
        -D CMAKE_BUILD_TYPE:String="Release" .. &&\
      make -j $(nproc)
    "

    #make -C /var/run/frr/libyang/build install
    make -C /home/ns3dce/libyang/build install

    su - ns3dce -c "
      [[ ! -d frr ]] && git clone https://github.com/frrouting/frr.git frr;
      cd frr &&\
      ./bootstrap.sh &&\
      ./configure \
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
          --enable-user=frr \
          --enable-group=frr \
          --enable-vty-group=frrvty \
          --with-pkg-git-version \
          --with-pkg-extra-version=-MyOwnFRRVersion &&\
      make -j $(nproc)
      #sudo make install &&\
      #sudo install -m 775 -o frr -g frr -d /var/log/frr &&\
      #sudo install -m 775 -o frr -g frrvty -d /etc/frr &&\
      #sudo install -m 640 -o frr -g frrvty tools/etc/frr/vtysh.conf /etc/frr/vtysh.conf &&\
      #sudo install -m 640 -o frr -g frr tools/etc/frr/frr.conf /etc/frr/frr.conf &&\
      #sudo install -m 640 -o frr -g frr tools/etc/frr/daemons.conf /etc/frr/daemons.conf &&\
      #sudo install -m 640 -o frr -g frr tools/etc/frr/daemons /etc/frr/daemons &&\
    "

    #cd /var/run/frr/frr
    cd /home/ns3dce/frr
    make install
    install -m 775 -o frr -g frr -d /var/log/frr
    install -m 775 -o frr -g frrvty -d /etc/frr
    install -m 640 -o frr -g frrvty tools/etc/frr/vtysh.conf /etc/frr/vtysh.conf
    install -m 640 -o frr -g frr tools/etc/frr/frr.conf /etc/frr/frr.conf
    install -m 640 -o frr -g frr tools/etc/frr/daemons.conf /etc/frr/daemons.conf
    install -m 640 -o frr -g frr tools/etc/frr/daemons /etc/frr/daemons
    install -m 644 tools/frr.service /etc/systemd/system/frr.service
    systemctl enable frr

    sed -i 's/#net.ipv4.ip_forward=1/net.ipv4.ip_forward=1/g' /etc/sysctl.conf
    sed -i 's/#net.ipv6.conf.forwarding=1/net.ipv6.conf.forwarding=1/g' /etc/sysctl.conf
    sysctl -p

  SHELL
end
