#!/bin/bash

# Function which runs a regex against the test file and records count and time.
run_regex() {
	start_time=$(date +%s.%N)
	count=`cat mtent12.txt | ./xees/$1 $2 | wc -l`
	end_time=$(date +%s.%N)
	time_diff=$(echo "$end_time - $start_time" | bc)
	printf "%10s | %10s | %10s\n" $1 $count $time_diff
}

# Make sure the test file is extracted first.
if [ ! -e "mtent12.txt" ]
then
	tar -xvzf mtent12.tar.gz
fi

# Make sure the xees exist.
if [ ! -e "xees/dre2" ]; then
	make
fi

# Iterate through regular expressions.
for re in `cat regexes.txt`
do
	echo Regex: $re;
	echo =================================================
	printf "%10s | %10s | %10s\n" "Program" "Matched" "Time"
	echo =================================================

	# Iterate through various regex programs.
	for xee in `ls xees`
	do
		run_regex $xee $re
		echo -------------------------------------------------
	done
	echo
	echo
done
