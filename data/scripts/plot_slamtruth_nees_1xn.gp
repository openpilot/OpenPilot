#!/bin/sh

# This script plots x,y,z,yaw,pitch,roll error and uncertainty of slam wrt mocap
#
# It uses the log file outputed by the sync_truth ruby macro to be
# used with jafar_irb
#
# Author: croussil

ARGV_FILEAVERAGE=$1
ARGV_FILEPATTERN=$2
ARGV_FILENUMMIN=$3
ARGV_FILENUMMAX=$4

ymin=0
ymax=50
tmin=0
tmax=70

## http://www.fourmilab.ch/rpkp/experiments/analysis/chiCalc.html
## 3 sigma = 0.00135 | 0.99865
## n=25
#chi_min_6xn=4.13294
#chi_max_6xn=8.29295
## n=50
#chi_min_6xn=4.63634
#chi_max_6xn=7.5768
## n=100
chi_min_6xn=5.013892
chi_max_6xn=7.092723

chi_min_6x1=0.4233
chi_max_6x1=21.7390


gnuplot<<EOF

# set terminal postscript eps color "Helvetica" 12 size 14cm,7cm
# set output 'plot_slamtruth_nees_1xn.eps'

set terminal png enhanced font "arial,12" size 1440,720
set output 'plot_slamtruth_nees_1xn.png'

# set term wxt size 1440,720

set grid

set xrange [$tmin:$tmax]

set tics scale 0.5
set border lw 0.5

file(n) = sprintf("$ARGV_FILEPATTERN",n)

set yrange [$ymin:$ymax]

plot \
	for [j=$ARGV_FILENUMMIN:$(($ARGV_FILENUMMAX-1))] file(j) using 1:((\$ $((2+42)))) with points pt 0 lc rgb "#D0D0FF" title "", \
	file($ARGV_FILENUMMAX) using 1:((\$ $((2+42)))) with points pt 0 lc rgb "#D0D0FF" title "6D NEES ${names[$i]}", \
	"$ARGV_FILEAVERAGE" using 1:((\$ $((2+42)))) with lines lt 1 lw 1 lc rgb "blue" title "Average 6D NEES", \
	$chi_min_6x1 with lines lt 2 lw 1 lc rgb "dark-red" title sprintf("6D NEES 3 sigma limits (%.2f:%.2f)", $chi_min_6x1, $chi_max_6x1), \
	$chi_max_6x1 with lines lt 2 lw 1 lc rgb "dark-red" title "", \
	$chi_min_6xn with lines lt 1 lw 1 lc rgb "red" title sprintf("Average 6D NEES 3 sigma limits (%.2f:%.2f)", $chi_min_6xn, $chi_max_6xn), \
	$chi_max_6xn with lines lt 1 lw 1 lc rgb "red" title ""

# prevent window from closing at the end
#pause -1

# set output
# !epstopdf --outfile=plot_slamtruth_nees_1xn.pdf plot_slamtruth_nees_1xn.eps
# !evince plot_slamtruth_nees_1xn.pdf
# quit

EOF

