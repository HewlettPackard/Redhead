set xlabel 'Applications'
set ylabel 'Time in milliseconds'
set title 'GameOfLife on /Dev/SHM - Size: ~500MB - Iterations: 100'
set yrange [0:1.749353100000000093e+06]
set style line 1 lc rgb 'red' 
set style line 2 lc rgb '#406090' 
set boxwidth 0.5 
set style fill solid 1.0 border -1 
plot 'GameOfLife.data' every ::0::0 using 1:3:xtic(2) title '~716513 milliseconds' with boxes ls 1, 'GameOfLife.data' every ::1::1 using 1:3:xtic(2) title '~1590321 milliseconds' with boxes ls 2, 
