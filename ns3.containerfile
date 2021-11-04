FROM docker.io/ubuntu:16.04 AS ns3-base

RUN apt-get update && apt-get install --no-install-recommends -y mercurial libexpat1-dev git-core build-essential bison \
    flex libssl-dev libdb-dev libpcap-dev libc6-dbg libsysfs-dev gawk indent \
    pkg-config autoconf automake sudo ccache libsaxonb-java openjdk-8-jre-headless python-setuptools \
    bc wget python-pip software-properties-common apt-transport-https ca-certificates cmake python3-dev htop \
    python-pygraphviz python-pygoocanvas g++-multilib qt5-default && \
    rm -rf /var/lib/apt/lists/*

RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test && apt-get update && \
      apt-get install -y g++-4.9

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.9 10 && \
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

RUN useradd -ms /bin/bash ns3dce && adduser ns3dce sudo && echo -n 'ns3dce:ns3dce' | chpasswd

RUN pip install --upgrade pip==9.0.3
RUN pip install setuptools-scm==5.0.2

RUN chown ns3dce:ns3dce -R /home/ns3dce

RUN su - ns3dce -c '\
      cd /home/ns3dce &&\
      [[ ! -d dce-linux-dev ]] && mkdir -p /home/ns3dce/dce-linux-dev ;\
      cd /home/ns3dce/dce-linux-dev &&\
      export PATH=/usr/lib/ccache:${PATH} &&\
      [[ ! -d bake ]] && git clone https://github.com/thehajime/bake ;\
    '
WORKDIR /home/ns3dce/dce-linux-dev
COPY bakeconf.xml.patch bake/
RUN chown ns3dce:ns3dce /home/ns3dce/dce-linux-dev/bake/bakeconf*

RUN  su - ns3dce -c "\
      export PATH=/usr/lib/ccache:${PATH} &&\
      cd /home/ns3dce/dce-linux-dev/bake &&\
      diff bakeconf.xml bakeconf.xml.test ;\
      git apply --whitespace=warn < bakeconf.xml.patch ;\
      cd .. &&\
      ./bake/bake.py configure -e dce-linux-dev &&\
      ./bake/bake.py download &&\
      ./bake/bake.py build -j $(nproc) -vvv\
    "
RUN sed -i -e \
      's/%sudo\s\+ALL=(ALL:ALL)\s\+ALL/%sudo ALL=NOPASSWD:ALL/g' \
      /etc/sudoers

USER ns3dce

WORKDIR /home/ns3dce/dce-linux-dev/source/dce-linux-dev
COPY ns3-bird.patch .
RUN git apply --whitespace=warn < ns3-bird.patch

WORKDIR /home/ns3dce/dce-linux-dev/source/ns-3-dev
COPY ns3-dev.patch .
RUN git apply --whitespace=warn < ns3-dev.patch

WORKDIR /home/ns3dce/dce-linux-dev
RUN ./bake/bake.py build -j $(nproc) -vvv

USER root
RUN /bin/bash -c "if [[ ! -f /var/run/bird.ctl ]]; then touch /var/run/bird.ctl; chown ns3dce:ns3dce /var/run/bird.ctl; fi"

FROM ns3-base AS ns3-ntf

WORKDIR /home/ns3dce/dce-linux-dev/source/dce-linux-dev
COPY script.sh .
USER root
RUN chmod +x script.sh && chown ns3dce:ns3dce script.sh
USER ns3dce
ENTRYPOINT ["./script.sh"]
CMD [""]
