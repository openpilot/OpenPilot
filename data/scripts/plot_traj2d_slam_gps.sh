#!/bin/sh

# This script plots the 2D trajectory of slam and gps
#
# You need to add to the GPS log, a first line with the format:
# "# x y z yaw pitch roll dx dy dz" (don't forget the sharp and space at the beginning)
# The 6 first values correspond to the transform to apply to the GPS curve to align
# it with the odometry / slam, and the 3 last values correspond to the translation 
# between the slam robot's origin and the GPS antenna. For instance:
#"# 377100.991034 4824447.002581 141.045482 0.5 0.0 0.0 -0.87 0.12 0.37" 
#
# Author: croussil

ARGV_SLAM=$1
ARGV_GPS=$2
ARGV_TITLE=$3

ARGV_COR=(`head $2 -n 1`)

gnuplot << EOF

set term wxt size 1024,768

# to have smaller crosses:
#set pointsize 0.3333333333333

# to have the same scale along x and y axis:
set size ratio -1
#set view equal xyz

# to display a grid:
set grid

# to have a different left y axis:
#set y2tics auto
#set ytics nomirror

# titles
set title "2D trajectories comparison $ARGV_TITLE"
set xlabel "x (m)"
set ylabel "y (m)"

# to define some functions:
theta=${ARGV_COR[4]}
dx=${ARGV_COR[1]}
dy=${ARGV_COR[2]}

rotation_x(x,y)=(x-dx)*cos(theta)-(y-dy)*sin(theta)
rotation_y(x,y)=(x-dx)*sin(theta)+(y-dy)*cos(theta)

transfo_x(t)=${ARGV_COR[7]}*cos(t)-${ARGV_COR[8]}*sin(t)-${ARGV_COR[7]}
transfo_y(t)=${ARGV_COR[7]}*sin(t)+${ARGV_COR[8]}*cos(t)-${ARGV_COR[8]}

# now plot:
plot \
	"$ARGV_GPS" using (rotation_x(\$14,\$13)):(rotation_y(\$14,\$13)) with lines lt rgb "blue" title "GPS", \
	"$ARGV_SLAM" using (\$2+transfo_x(\$9)):(\$3+transfo_y(\$9)) with lines lt rgb "red" title "SLAM"

# prevent window from closing at the end
#pause -1

EOF

