#!/bin/sh

ARGV1=$1
gnuplot << EOF

set term wxt size 1024,768

# file to plot:
file="$ARGV1"

# This script plots the evolution of estimation of IMU gyro biases (wx,wy,wz)
# with 3-sigma uncertainty.
#
# Author: croussil


# to have smaller crosses:
set pointsize 0.3333333333333

# to have the same scale along x and y axis:
#set size ratio -1

# to display a grid:
#set grid

# titles
set title "Gyro biases"
set xlabel "Time (s)"
set ylabel "Angular velocity (deg/s)"

# to define some variables:
t0=0
deltat(t)=((t0<=1)?(t0=t,0):(t-t0))
#t0=1297772014.53331995

# now plot:
plot \
	file using (deltat(\$1)):21 with points pt 1 lt rgb "red"       title "Bias WX with 3 sigma limits", \
	file using (deltat(\$1)):(\$21-3*\$43) with points pt 0 lt rgb "red"       title "", \
	file using (deltat(\$1)):(\$21+3*\$43) with points pt 0 lt rgb "red"       title "", \
	file using (deltat(\$1)):22 with points pt 1 lt rgb "dark-red"  title "Bias WY with 3 sigma limits", \
	file using (deltat(\$1)):(\$22-3*\$44) with points pt 0 lt rgb "dark-red"       title "", \
	file using (deltat(\$1)):(\$22+3*\$44) with points pt 0 lt rgb "dark-red"       title "", \
	file using (deltat(\$1)):23 with points pt 1 lt rgb "orange"    title "Bias WZ with 3 sigma limits", \
	file using (deltat(\$1)):(\$23-3*\$45) with points pt 0 lt rgb "orange"       title "", \
	file using (deltat(\$1)):(\$23+3*\$45) with points pt 0 lt rgb "orange"       title ""


# prevent window from closing at the end
#pause -1

EOF
