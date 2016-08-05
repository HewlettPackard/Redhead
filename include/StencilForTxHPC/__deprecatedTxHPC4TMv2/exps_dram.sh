#!/bin/bash

jm=0
jer="_j"
rs="_rs"
ws=32
while [ $jm != 2 ]
do
l=0
	while [ $l != 2 ]
	do
		fm=0
		while [ $fm != 2 ]
		do
		if [ $l = 0 ]; then
			if [ $fm = 0 ]; then
				fn="dram_nf_ns"
			else
				fn="dram_ns"
			fi
		fi
		if [ $l = 1 ]; then
			if [ $fm = 0 ]; then
				fn="dram_nf_s"
			else
				fn="dram_s"
			fi
		fi
		if [ $jm = 0 ]; then
			fn=$fn$rs 
			ws=32
		else
			fn=$fn$jer
			ws=11 
		fi
		./arch_probe/arch_probe 0 $jm 1 $fm $l
		make r6_emu_dram
		mkdir newresults/"$fn"
		k=4096
		j=2
		while [ $k != 17592186044416 ]
		do
			./r6_emu_dram $k 8 $ws 2 $j 1 1 1024 0 > newresults/"$fn"/"r6_log_$k"
			#cp /proc/`pidof r6_emu`/status results/"mem_$k"
			echo `pidof r6_emu` 
			k=$(( k * 2 ))
			if [ $k = 32768 ]; then
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
			if [ $k = 68719476736 ]; then
        			j=$(( j * 2 ))       
 			fi                           
			if [ $k = 274877906944 ]; then
                		j=$(( j * 2 ))
        		fi
        		if [ $k = 1099511627776 ]; then
                		j=$(( j * 2 ))
        		fi 
			if [ $k = 2199023255552 ]; then
        			j=$(( j * 2 ))         
			fi                     
			if [ $k = 4398046511104 ]; then
        			j=$(( j * 2 ))         
			fi                     
			if [ $k = 8796093022208 ]; then
        			j=$(( j * 2 ))         
			fi                                                  
		done
		make clean
		fm=$(( fm + 1 ))
	done
	l=$(( l + 1 ))
done
jm=$(( jm + 1 ))
done
