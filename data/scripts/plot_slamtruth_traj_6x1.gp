#!/bin/sh

# This script plots x,y,z,yaw,pitch,roll of slam and mocap wrt time
#
# It uses the log file outputed by the sync_truth ruby macro to be
# used with jafar_irb
#
# Author: croussil

ARGV_FILE=$1
OUTPUT_FILE=plot_slamtruth_traj_6x1

# mocap 04 loop closure
ymin_pos=-149
ymax_pos=250
ymin_angle=-200
ymax_angle=199
tmin=0
tmax=56

# mocap 01 high dyn
# ymin_pos=-29.8
# ymax_pos=30
# ymin_angle=-40
# ymax_angle=39.8
# tmin=0
# tmax=69.85

##

names=("x (cm)" "y (cm)" "z (cm)" "yaw (deg)" "pitch (deg)" "roll (deg)")
coeff_pos=100
coeff_angle=57.2958

globlmargin=0.033
globrmargin=0.012
#globtmargin=0.015
#globbmargin=0.045
globtmargin=0.02
globbmargin=0.07

coeffs=($coeff_pos $coeff_pos $coeff_pos $coeff_angle $coeff_angle $coeff_angle)
ymin=($ymin_pos $ymin_pos $ymin_pos $ymin_angle $ymin_angle $ymin_angle)
ymax=($ymax_pos $ymax_pos $ymax_pos $ymax_angle $ymax_angle $ymax_angle)
functions=(identity identity identity continuous continuous continuous)

plotwidth="((1-$globlmargin-$globrmargin)/3.0)"
plotheight="((1-$globtmargin-$globbmargin)/2.0)"

###############################
## HEADER
script_header=`cat<<EOF


# set terminal postscript eps color "Helvetica" 12 size 14cm,7cm
set terminal postscript eps color "Helvetica" 10 size 14cm,3.5cm
set output '$OUTPUT_FILE.eps'

#set terminal png enhanced font "arial,12" size 1440,720 truecolor
#set terminal png enhanced font "arial,26" size 3000,1500 truecolor
# set terminal png enhanced font "arial,26" size 3000,750 truecolor
# set output '$OUTPUT_FILE.png'

#set term wxt size 1280,640

set grid

set xrange [$tmin:$tmax]

set tics scale 0.33
set border lw 0.5
set grid lw 1.0 lt rgb "black"
set key at graph 1,0.97

#set multiplot layout 2,3 title "Trajectories comparison (wrt time in s)"
set multiplot layout 2,3

k=0
prev_a=0
continuous(a)=((prev_a>pi/2&&a<-pi/2 ? k=k+1 : (prev_a<-pi/2&&a>pi/2 ? k=k-1 : k)), prev_a=a, a+k*2*pi)
identity(a)=a


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
	xlabel='set label "time (s)" at graph 0.90,-0.08';
else
	xlabel='';
fi


###############################
## LOOP
script_loop=`cat<<EOF

set yrange [${ymin[$i]}:${ymax[$i]}]

set lmargin at screen ($lmargin)
set rmargin at screen ($rmargin)
set tmargin at screen ($tmargin)
set bmargin at screen ($bmargin)

set format x ($i<3?"":"%g")
set format y ($i%3?"":"%g")

$xlabel

plot \
	"$ARGV_FILE" using 1:(${functions[$i]}(\$ $((2+$i)))*${coeffs[$i]}) with lines lt 1 lw 1.0 lc rgb "red" title "slam ${names[$i]}", \
	"$ARGV_FILE" using 1:(${functions[$i]}(\$ $((2+12+$i)))*${coeffs[$i]}) with lines lt 1 lw 1.0 lc rgb "black" title "mocap ${names[$i]}"

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


