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
  config.vm.synced_folder "bird", "/data/bird", type: "nfs", nfs_udp: "false"
  config.vm.synced_folder "helpers", "/data/helpers", type: "nfs", nfs_udp: "false"
  config.vm.synced_folder "myscripts", "/home/ns3dce/dce-linux-dev/source/ns-3-dce/myscripts", type: "nfs", nfs_udp: "false"

  config.vm.provider "libvirt" do |lb|
    lb.memory = "4096"
    lb.cpus = 4
  end

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
    sed -i 's/#net.ipv4.ip_forward=1/net.ipv4.ip_forward=1/g' /etc/sysctl.conf
    sed -i 's/#net.ipv6.conf.forwarding=1/net.ipv6.conf.forwarding=1/g' /etc/sysctl.conf
    sysctl -p
  SHELL
end
