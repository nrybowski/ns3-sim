FROM ns3-base

WORKDIR /home/ns3dce/dce-linux-dev/source/dce-linux-dev
#USER ns3dce
#COPY ns3-bird.patch .
#RUN git apply --whitespace=warn < ns3-bird.patch

USER root
RUN /bin/bash -c "if [[ ! -f /var/run/bird.ctl ]]; then touch /var/run/bird.ctl; chown ns3dce:ns3dce /var/run/bird.ctl; fi"
COPY script.sh .
RUN chmod +x script.sh && chown ns3dce:ns3dce script.sh
RUN apt-get install -y valgrind

USER ns3dce

ENTRYPOINT ["./script.sh"]
CMD [""]
