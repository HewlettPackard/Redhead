set xlabel 'Applications'
set ylabel 'Time in microseconds'
set title 'Memory Bandwidth V2 on /Dev/SHM - Size: ~500MB - Iterations: 100'
set yrange [0:1.627965900000000140e+06]
set style line 1 lc rgb 'red' 
set style line 2 lc rgb '#406090' 
set boxwidth 0.5 
set style fill solid 1.0 border -1 
plot 'MemoryBandwidth.data' every ::0::0 using 1:3:xtic(2) title '~1479969 microseconds' with boxes ls 1, 'MemoryBandwidth.data' every ::1::1 using 1:3:xtic(2) title '~219750 microseconds' with boxes ls 2, 
