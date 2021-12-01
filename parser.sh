#! /bin/bash

LOG=$(mktemp)

for i in $(seq 0 5)
do
    python3 parser.py output/files/files-${i}/var/log/udp_ping.log >> "${LOG}"
done

cat "${LOG}"

RAW=$(cat "${LOG}" | grep -E "^[0-9]+ [0-9]+" | sed -E "s/^[0-9]+ //g")
echo -e "\n${RAW}"

TOP=$(echo "${RAW}" | sed -E "s/ [0-9.]+ [0-9]*$//g" | sort -n -r | head -n 1)
echo -e "\ntop: ${TOP}"

BOTTOM=$(echo "${RAW}" | sed -E "s/^[0-9.]+ //g;s/ [0-9]+$//g" | sort -n | head -n 1)
echo -e "bottom: ${BOTTOM}"
GAP=$(echo "(${TOP} - ${BOTTOM})*1000" | bc)
echo "real gap: ${GAP}"


rm "${LOG}"
