set xlabel 'Applications'
set ylabel 'Time in milliseconds'
set title 'Jacobian2D on /Dev/SHM - Size: 500MB - Iterations: 100'
set yrange [0:2.499519000000000233e+05]
set style line 1 lc rgb 'red' 
set style line 2 lc rgb '#406090' 
set boxwidth 0.5 
set style fill solid 1.0 border -1 
plot 'Jacobian2D.data' every ::0::0 using 1:3:xtic(2) title '~39369 milliseconds' with boxes ls 1, 'Jacobian2D.data' every ::1::1 using 1:3:xtic(2) title '~227229 milliseconds' with boxes ls 2, 
