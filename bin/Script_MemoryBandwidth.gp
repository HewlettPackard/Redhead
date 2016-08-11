set xlabel 'Applications'
set ylabel 'Time in microseconds'
set title 'Memory Bandwidth on /Dev/SHM - Size: ~500MB - Iterations: 1000'
set yrange [0:1.673404700000000186e+06]
set style line 1 lc rgb 'red' 
set style line 2 lc rgb '#406090' 
set boxwidth 0.5 
set style fill solid 1.0 border -1 
plot 'MemoryBandwidth.data' every ::0::0 using 1:3:xtic(2) title '~347996 microseconds' with boxes ls 1, 'MemoryBandwidth.data' every ::1::1 using 1:3:xtic(2) title '~1521277 microseconds' with boxes ls 2, 
