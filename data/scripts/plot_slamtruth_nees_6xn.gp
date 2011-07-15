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
OUTPUT_FILE=plot_slamtruth_nees_6xn

# mocap 01 high dyn
ymin_pos=0.11
ymax_pos=100
ymin_angle=0.1
ymax_angle=90
tmin=0
tmax=69.8

## http://www.fourmilab.ch/rpkp/experiments/analysis/chiCalc.html
## 99.72% (1D 3 sigma) = 0.00135 | 0.99865
## n=25
#chi_min_1xn=
#chi_max_1xn=
## n=50
#chi_min_1xn=
#chi_max_1xn=
## n=100
chi_min_1xn=0.673275
chi_max_1xn=1.401694

chi_min_1x1=0
chi_max_1x1=7.8794

##

names=("x" "y" "z" "yaw" "pitch" "roll")
coeff_pos=1
coeff_angle=1

globlmargin=0.03
globrmargin=0.012
globtmargin=0.015
globbmargin=0.035

coeffs=($coeff_pos $coeff_pos $coeff_pos $coeff_angle $coeff_angle $coeff_angle)
ymin=($ymin_pos $ymin_pos $ymin_pos $ymin_angle $ymin_angle $ymin_angle)
ymax=($ymax_pos $ymax_pos $ymax_pos $ymax_angle $ymax_angle $ymax_angle)

plotwidth="((1-$globlmargin-$globrmargin)/3.0)"
plotheight="((1-$globtmargin-$globbmargin)/2.0)"

###############################
## HEADER
script_header=`cat<<EOF

set terminal postscript eps color "Helvetica" 10 size 14cm,7cm dashlength 2.0
set output '$OUTPUT_FILE.eps'

#set terminal png enhanced font "arial,12" size 1440,720 truecolor
#set terminal png enhanced font "arial,26" size 3000,1500 truecolor
#set output '$OUTPUT_FILE.png'

# set term wxt size 1440,720

set grid

set xrange [$tmin:$tmax]

set tics scale 0.33
set border lw 0.5
set grid lw 1.0 lt rgb "black"
set key at graph 1,0.985

#set multiplot layout 2,3 title "Trajectory error (wrt time in s)"
set multiplot layout 2,3

file(n) = sprintf("$ARGV_FILEPATTERN",n)

EOF
`
##
###############################

script=$script_header

for ((i = 0; $i < 6; i++)); do

lmargin="$globlmargin+($i%3)*$plotwidth"
rmargin="$globlmargin+($i%3+1)*$plotwidth"
tmargin="$globbmargin+($i<3?1:0)*$plotheight"
bmargin="$globbmargin+(($i<3?1:0)+1)*$plotheight"

if [[ $i == 5 ]]; then
	xlabel='set label "time (s)" at graph 0.90,-0.04';
else
	xlabel='';
fi

###############################
## LOOP
script_loop=`cat<<EOF

set yrange [${ymin[$i]}:${ymax[$i]}]
set logscale y

set lmargin at screen ($lmargin)
set rmargin at screen ($rmargin)
set tmargin at screen ($tmargin)
set bmargin at screen ($bmargin)

set format x ($i<3?"":"%g")
set format y ($i%3?"":"%g")

$xlabel

plot \
	for [j=$ARGV_FILENUMMIN:$(($ARGV_FILENUMMAX-1))] file(j) using 1:((\$ $((2+36+$i)))*${coeffs[$i]}) with points pt 0 lc rgb "#D0D0FF" title "", \
	file($ARGV_FILENUMMAX) using 1:((\$ $((2+36+$i)))*${coeffs[$i]}) with points pt 0 lc rgb "#D0D0FF" title "NEES ${names[$i]}", \
	"$ARGV_FILEAVERAGE" using 1:((\$ $((2+36+$i)))*${coeffs[$i]}) with lines lt 1 lw 1 lc rgb "black" title "Average NEES", \
	$chi_min_1x1 with lines lt 3 lw 1 lc rgb "dark-red" title sprintf("NEES 99%% confid. area (%.2f:%.2f)", $chi_min_1x1, $chi_max_1x1), \
	$chi_max_1x1 with lines lt 3 lw 1 lc rgb "dark-red" title "", \
	$chi_min_1xn with lines lt 1 lw 1 lc rgb "red" title sprintf("Average NEES 99%% confid. area (%.2f:%.2f)", $chi_min_1xn, $chi_max_1xn), \
	$chi_max_1xn with lines lt 1 lw 1 lc rgb "red" title ""

EOF
`
##
###############################

script="$script$script_loop"
done

###############################
## FOOTER
script_footer=`cat<<EOF

unset multiplot

# prevent window from closing at the end
#pause -1

set output
!convert -density 600x600 -flatten -depth 8 -colorspace RGB $OUTPUT_FILE.eps ppm:- | convert - $OUTPUT_FILE.png
!epstopdf --outfile=$OUTPUT_FILE.pdf $OUTPUT_FILE.eps
# !evince $OUTPUT_FILE.pdf
quit

EOF
`
##
###############################

script="$script$script_footer"

gnuplot<<EOF
$(echo "$script")
EOF


