FROM ns3-base

USER root

WORKDIR /opt
RUN apt-get install -y valgrind zlib1g-dev libssl-dev libffi-dev

# Install python3.7
RUN wget https://www.python.org/ftp/python/3.7.0/Python-3.7.0.tar.xz
RUN md5sum Python-3.7.0.tar.xz
RUN tar -xf Python-3.7.0.tar.xz
WORKDIR /opt/Python-3.7.0
RUN ./configure --enable-optimizations
RUN make -j$(nproc) altinstall
RUN pip3.7 install --upgrade pip

# Add ctl socket for bird
RUN touch /var/run/bird.ctl && chown ns3dce:ns3dce /var/run/bird.ctl

USER ns3dce

# Apply patches for bird
WORKDIR /home/ns3dce/dce-linux-dev/source/dce-linux-dev
COPY ns3-bird.patch .
RUN git apply --whitespace=warn < ns3-bird.patch

WORKDIR /home/ns3dce/dce-linux-dev/source/ns-3-dev
COPY ns3-dev.patch .
RUN git apply --whitespace=warn < ns3-dev.patch

# Rebuild ns3 with patches
WORKDIR /home/ns3dce/dce-linux-dev
RUN ./bake/bake.py build -j $(nproc) -vvv

# Install scripts and their requirements
WORKDIR /home/ns3dce/dce-linux-dev/source/dce-linux-dev
COPY requirements.txt .
RUN pip3.7 install -r requirements.txt
#COPY script.sh spfs.py ./
COPY script.sh .

USER root
RUN rm /usr/bin/python3
RUN ln -s /usr/local/bin/python3.7 /usr/bin/python3
USER ns3dce

COPY dce-system.patch .
RUN git apply --whitespace=warn < dce-system.patch

ENTRYPOINT ["./script.sh"]
CMD [""]
