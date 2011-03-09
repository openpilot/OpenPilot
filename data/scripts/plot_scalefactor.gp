#!/bin/sh

ARGV1=$1
ARGV_TITLE=$2
gnuplot << EOF

set term wxt size 1024,768

# file to plot:
file="$ARGV1"

# This script plots the evolution of the scale factor, computed with the ruby script
#
# Author: croussil


# to have smaller crosses:
set pointsize 0.3333333333333

# to have the same scale along x and y axis:
#set size ratio -1

# to display a grid:
set grid

# to have a different left y axis:
set y2tics auto
set ytics nomirror

# titles
set title "Scale Factor $ARGV_TITLE"
set xlabel "Time (s)"
set ylabel "Scale Factor"


# to define some variables:
t0=0
deltat(t)=((t0<=1)?(t0=t,0):(t-t0))
#t0=1297772014.53331995

# now plot:
plot \
	file using (deltat(\$1)):2 with lines lt rgb "red"       notitle, \
	1 with lines lt rgb "black" lw 2 notitle


# prevent window from closing at the end
#pause -1

EOF
