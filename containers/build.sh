#! /bin/bash -xe

CURDIR="${PWD}"
cd containers
docker build -t ns3-base -f ns3-base.containerfile .
docker build -t ns3-bird -f ns3-bird.containerfile .
cd "${CURDIR}"
