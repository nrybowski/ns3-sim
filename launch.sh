#! /bin/bash -x

if [[ $# != 1 ]]; then	exit 1; fi

TOPO=${1}

if [[ -f ${TOPO}.out ]]; then rm ${TOPO}.out; fi

npf/npf-compare.py local --test test.npf --var TOPOLOGY=${TOPO} 2>&1
while read -r line
do
	if [[ -d $line ]]
	then
		mv $line output
	fi
done < ${TOPO}.out
