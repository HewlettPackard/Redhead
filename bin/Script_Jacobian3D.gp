set xlabel 'Applications'
set ylabel 'Time in milliseconds'
set title 'Jacobian3D on /Dev/SHM - Size: ~500MB - Iterations: 100'
set yrange [0:3.649800000000000182e+03]
set style line 1 lc rgb 'red' 
set style line 2 lc rgb '#406090' 
set boxwidth 0.5 
set style fill solid 1.0 border -1 
plot 'Jacobian3D.data' every ::0::0 using 1:3:xtic(2) title '~3318 milliseconds' with boxes ls 1, 'Jacobian3D.data' every ::1::1 using 1:3:xtic(2) title '~0 milliseconds' with boxes ls 2, 
