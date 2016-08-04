#!/bin/bash

k=16384
j=2
l=1
while [ $k != 2199023255552 ]
do
	./apps/app_emu $k 8 1024 > results/DRAM/"r6_log_$k"
	#cp /proc/`pidof r6_emu`/status results/"mem_$k"
	echo `pidof r6_emu` 
	k=$(( k * 2 ))
done
