#!/bin/bash

for j in `find ../graph/ -type f`; do
	echo "Working on $j";
	for k in pagerank sssp scc hashmin; do
		for i in `find bin -type f | grep $k | grep -v "_no_"`; do
			echo -n "`./$i $j out.txt | grep Total | rev | cut -d ' ' -f 1 | cut -d '.' -f 2-3 | cut -d 's' -f 2 | rev`"; echo " $i";
		done;
	done; 
	echo "";
done
