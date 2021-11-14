#! /bin/bash

if [[ ! -d env ]]
then
    virtualenv env
    source env/bin/activate
    pip3 install -r requirements.txt
fi

source env/bin/activate
python3 spfs.py
deactivate
cat udp.ping
