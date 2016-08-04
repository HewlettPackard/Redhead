#!/bin/bash

k=34359738368
j=2048
l=1
while [ $k != 2199023255552 ]
do
	./r6_emu_lfs $k 8 11 2 $j 1 1 2048 0 > results/LFS/"r6_log_$k"
	#cp /proc/`pidof r6_emu`/status results/"mem_$k"
	echo `pidof r6_emu` 
	k=$(( k * 2 ))
	if [ $k = 32768 ]; then
		j=$(( j * 2 ))
	fi
	if [ $k = 65536 ]; then
		j=$(( j * 2 ))
	fi
	if [ $k = 131072 ]; then
		j=$(( j * 2 ))
	fi
	if [ $k = 4194304 ]; then
		j=$(( j * 2 ))
	fi
	if [ $k = 33554432 ]; then
		j=$(( j * 2 ))
	fi
	if [ $k = 268435456 ]; then
		j=$(( j * 2 ))
	fi
	if [ $k = 10737441824 ]; then
		j=$(( j * 2 ))
	fi
	if [ $k = 2147483648 ]; then
		j=$(( j * 2 ))
	fi
	if [ $k = 4294967296 ]; then
        	j=$(( j * 2 ))      
	fi                
	if [ $k = 17179869184 ]; then
                j=$(( j * 2 ))
        fi
        if [ $k = 34359738368 ]; then
                j=$(( j * 2 ))
        fi          
	if [ $k = 68719476736 ]; then
        	j=$(( j * 2 ))       
 	fi                           
 	if [ $k = 137438953472 ]; then
        	j=$(( j * 2 ))       
 	fi                    
	if [ $k = 274877906944 ]; then
                j=$(( j * 2 ))
        fi
        if [ $k = 549755813888 ]; then
                j=$(( j * 2 ))
        fi
        if [ $k = 1099511627776 ]; then
                j=$(( j * 2 ))
        fi       
done
