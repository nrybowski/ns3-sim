FROM ns3-base

WORKDIR /opt
USER root
RUN apt-get install -y valgrind zlib1g-dev libssl-dev libffi-dev

# Install latest python
RUN wget https://www.python.org/ftp/python/3.7.0/Python-3.7.0.tar.xz
RUN md5sum Python-3.7.0.tar.xz
RUN tar -xf Python-3.7.0.tar.xz
WORKDIR /opt/Python-3.7.0
RUN ./configure --enable-optimizations
RUN make -j$(nproc) altinstall
RUN pip3.7 install --upgrade pip

WORKDIR /home/ns3dce/dce-linux-dev/source/dce-linux-dev
RUN /bin/bash -c "if [[ ! -f /var/run/bird.ctl ]]; then touch /var/run/bird.ctl; chown ns3dce:ns3dce /var/run/bird.ctl; fi"
COPY script.sh spfs.py requirements.txt ./
RUN chmod +x script.sh && chown ns3dce:ns3dce script.sh
RUN pip3.7 install -r requirements.txt

USER ns3dce
ENTRYPOINT ["./script.sh"]
CMD [""]
