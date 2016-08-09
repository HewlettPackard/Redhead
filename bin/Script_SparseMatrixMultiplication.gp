set xlabel 'Applications'
set ylabel 'Time in microseconds'
set title 'SparseMatrixMultiplicatoin V2 on /Dev/SHM - Size: ~500MB - Iterations: 100'
set yrange [0:3.792305000000000582e+05]
set style line 1 lc rgb 'red' 
set style line 2 lc rgb '#406090' 
set boxwidth 0.5 
set style fill solid 1.0 border -1 
plot 'SparseMatrixMultiplication.data' every ::0::0 using 1:3:xtic(2) title '~743 microseconds' with boxes ls 1, 'SparseMatrixMultiplication.data' every ::1::1 using 1:3:xtic(2) title '~344755 microseconds' with boxes ls 2, 
