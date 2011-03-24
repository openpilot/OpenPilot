#!/bin/sh

# This script plots the 3D trajectory of a gps
#
# Author: croussil

ARGV1=$1
gnuplot << EOF

set term wxt size 1024,768

# file to plot:
file="$ARGV1"

# to have smaller crosses:
#set pointsize 0.3333333333333

# to have the same scale along x and y axis:
#set size square
set view equal xyz

# to display a grid:
set grid

# to have a different left y axis:
#set y2tics auto
#set ytics nomirror

# titles
set title "3D trajectory"
set xlabel "x (m)"
set ylabel "y (m)"
set zlabel "z (m)"

# to define some functions:
#norm3(x,y,z)=sqrt(x*x+y*y+z*z)
#pitch(x,y,z) = atan2(-x, z)*180/pi
#roll(x,y,z) = atan2(y, z)*180/pi

# to define some variables:
#t0=1281469305.9

# now plot:
splot \
	file using 3:2:4 with lines lt rgb "blue" notitle

# prevent window from closing at the end
#pause -1

EOF

