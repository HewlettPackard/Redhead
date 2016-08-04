#!/bin/bash

./r6_emu 1048576 8 32 2 16 1 1 1024 0 > results/log_before_crash &
for i in `seq 1 10`;
do
	j=0
done

kill `pidof r6_emu`
./r6_emu 1048576 8 32 2 16 1 1 1024 1 > results/log_after_crash

