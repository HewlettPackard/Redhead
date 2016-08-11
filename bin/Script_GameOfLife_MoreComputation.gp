set xlabel 'Applications'
set ylabel 'Time in milliseconds'
set title 'GameOfLife with 100 times more computation on /Dev/SHM - Size: ~500MB - Iterations: 2'
set yrange [0:2.776081000000000349e+05]
set style line 1 lc rgb 'red' 
set style line 2 lc rgb '#406090' 
set boxwidth 0.5 
set style fill solid 1.0 border -1 
plot 'GameOfLife_MoreComputation.data' every ::0::0 using 1:3:xtic(2) title '~149446 milliseconds' with boxes ls 1, 'GameOfLife_MoreComputation.data' every ::1::1 using 1:3:xtic(2) title '~252371 milliseconds' with boxes ls 2, 
