set xlabel 'Size of Data in GB'
set ylabel 'Time in milliseconds'
set title 'Scaling Up The Jacobi Iteration'
set xrange [0:10000]
set yrange [1:1000]
plot 'Jacobian_ScaleUP.data' with points title 'Jacobi_Iteration'

