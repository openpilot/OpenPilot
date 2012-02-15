#!/bin/sh

ARGV1=$1
gnuplot << EOF

set term wxt size 1024,768

# file to plot:
file="$ARGV1"

# This script plots the evolution of estimation of IMU accelero biases (ax,ay,az,g)
# with 3-sigma uncertainty.
#
# Author: croussil


# to have smaller crosses:
set pointsize 0.3333333333333

# to have the same scale along x and y axis:
#set size ratio -1

# to display a grid:
#set grid

# to have a different left y axis:
set y2tics auto
set ytics nomirror

# titles
set title "Accelero biases and g"
set xlabel "Time (s)"
set ylabel "Acceleration (m/s2)"
set y2label "Angle (deg)"

# to define some functions:
sign(x)=(x<0?-1:1)
norm3(x,y,z)=sqrt(x*x+y*y+z*z)
pitch(x,y,z) = sign(x)*atan2(abs(x), -z)*180/pi
roll(x,y,z) = sign(y)*atan2(abs(y), -z)*180/pi

# to define some variables:
t0=0
deltat(t)=((t0<=1)?(t0=t,0):(t-t0))
#t0=1297772014.53331995

# now plot:
plot \
	file using (deltat(\$1)):18 with points pt 1 lt rgb "red"       title "Bias AX with 3 sigma limits", \
	file using (deltat(\$1)):(\$18-3*\$40) with points pt 0 lt rgb "red"       title "", \
	file using (deltat(\$1)):(\$18+3*\$40) with points pt 0 lt rgb "red"       title "", \
	file using (deltat(\$1)):19 with points pt 1 lt rgb "dark-red"  title "Bias AY with 3 sigma limits", \
	file using (deltat(\$1)):(\$19-3*\$41) with points pt 0 lt rgb "dark-red"       title "", \
	file using (deltat(\$1)):(\$19+3*\$41) with points pt 0 lt rgb "dark-red"       title "", \
	file using (deltat(\$1)):20 with points pt 1 lt rgb "orange"    title "Bias AZ with 3 sigma limits", \
	file using (deltat(\$1)):(\$20-3*\$42) with points pt 0 lt rgb "orange"       title "", \
	file using (deltat(\$1)):(\$20+3*\$42) with points pt 0 lt rgb "orange"       title "", \
	file using (deltat(\$1)):(norm3(\$24,\$25,\$26)-9.805) with points pt 1 lt rgb "blue"          title "Norm g - 9.805 with 3 sigma limits", \
	file using (deltat(\$1)):(norm3(\$24,\$25,\$26)-9.805-3*norm3(\$46,\$47,\$48)) with points pt 0 lt rgb "blue"          title "", \
	file using (deltat(\$1)):(norm3(\$24,\$25,\$26)-9.805+3*norm3(\$46,\$47,\$48)) with points pt 0 lt rgb "blue"          title "", \
	file using (deltat(\$1)):(pitch(\$24,\$25,\$26)) axes x1y2 with points pt 1 lt rgb "green"     title "Pitch g", \
	file using (deltat(\$1)):(roll(\$24,\$25,\$26)) axes x1y2 with points pt 1 lt rgb "dark-green" title "Roll g"


# prevent window from closing at the end
#pause -1

EOF
