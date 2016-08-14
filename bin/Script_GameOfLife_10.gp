set xlabel 'Applications'
set ylabel 'Time in milliseconds'
set title 'GameOfLife on NVM - Size: 500MB - Iterations: 10'
set yrange [0:1.876435000000000291e+05]
set style line 1 lc rgb 'red' 
set style line 2 lc rgb '#406090' 
set boxwidth 0.5 
set style fill solid 1.0 border -1 
plot 'GameOfLife.data' every ::0::0 using 1:3:xtic(2) title '~69372 milliseconds' with boxes ls 1, 'GameOfLife.data' every ::1::1 using 1:3:xtic(2) title '~170585 milliseconds' with boxes ls 2, 
