#! /bin/bash

FAILURES_NTF="${1}"
FAILURES=""

compute() {
	local logs=$(echo -e "${1}")
	local top=$(echo "${logs}" | sed -E "s/ [0-9.]+ [0-9]*$//g" | sort -n -r | head -n 1)
	#echo -e "top: ${top}"

	local bottom=$(echo "${logs}" | sed -E "s/^[0-9.]+ //g;s/ [0-9]+$//g" | sort -n | head -n 1)
	#echo -e "bottom: ${bottom}"

	local gap=$(echo "(${top} - ${bottom})*1000" | bc)
	#echo "real gap: ${gap}"
	echo "${gap}"
}

while read line
do
	if [[ "${line:0:1}" != "#" ]]
	then
		FAILURES="${FAILURES}$(echo "${line}" | sed -E "s/^[a-zA-Z0-9]+ [a-zA-Z0-9]+ [0-9]+ [0-9]+ //g") "
	fi
done < "${FAILURES_NTF}"

echo "failures: ${FAILURES}"
FAILURES=( ${FAILURES} )

LOG=$(mktemp)
for i in $(seq 0 5)
do
    python3 parser.py output/files/files-${i}/var/log/udp_ping.log >> "${LOG}"
done
cat "${LOG}"

RAW=$(cat "${LOG}" | grep -E "^[0-9]+ [0-9]+" | sed -E "s/^[0-9]+ //g")
echo -e "\n${RAW}\n"

FAIL=0
ACC=""
declare -A LOGS
while read -r line
do
	FAIL_TIMESTAMP="${FAILURES[$FAIL]}"
	CURRENT=$(echo "${line}" | sed -E "s/^[0-9.]+ //g;s/ [0-9]+$//g")
	if [[ $(echo "${CURRENT} <= ${FAIL_TIMESTAMP}" | bc) -eq 1 ]]
	then
		ACC="${ACC}${line}\n"
	else
		LOGS["${FAIL_TIMESTAMP}"]="${ACC}"
		ACC="${line}\n"
		((FAIL++))
	fi
done < <(printf '%s\n' "${RAW}" | sort -n -k 2)
LOGS["${FAILURES[(($FAIL))]}"]="${ACC}"

RETURN=""
for key in "${!LOGS[@]}"
do
	echo -e "$key:\n${LOGS[$key]}\n"
	RETURN="${RETURN}$key:$(compute "${LOGS[$key]}") "
done

echo "${RETURN}"

rm "${LOG}"
