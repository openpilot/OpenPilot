#!/bin/sh

# This script plots x,y,z,yaw,pitch,roll of slam and mocap wrt time
#
# It uses the log file outputed by the sync_truth ruby macro to be
# used with jafar_irb
#
# Author: croussil

ARGV_FILE=$1

# ymin_pos=-100
# ymax_pos=250
# ymin_angle=-200
# ymax_angle=200
# tmin=0
# tmax=56

ymin_pos=-30
ymax_pos=30
ymin_angle=-40
ymax_angle=40
tmin=0
tmax=70

##

names=("x (cm)" "y (cm)" "z (cm)" "yaw (deg)" "pitch (deg)" "roll (deg)")
coeff_pos=100
coeff_angle=57.2958

globlmargin=0.033
globrmargin=0.012
globtmargin=0.015
globbmargin=0.045

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
# set output 'plot_slamtruth_traj_6x1.eps'

#set terminal png enhanced font "arial,12" size 1440,720 truecolor
set terminal png enhanced font "arial,26" size 3000,1500 truecolor
set output 'plot_slamtruth_traj_6x1.png'

#set term wxt size 1280,640

set grid

set xrange [$tmin:$tmax]

set tics scale 0.5
set border lw 0.5

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

plot \
	"$ARGV_FILE" using 1:(${functions[$i]}(\$ $((2+$i)))*${coeffs[$i]}) with lines lt 1 lw 1 lc rgb "blue" title "slam ${names[$i]}", \
	"$ARGV_FILE" using 1:(${functions[$i]}(\$ $((2+12+$i)))*${coeffs[$i]}) with lines lt 2 lw 1 lc rgb "red" title "mocap ${names[$i]}"

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


# set output
# !epstopdf --outfile=plot_slamtruth_traj_6x1.pdf plot_slamtruth_traj_6x1.eps
# !evince plot_slamtruth_traj_6x1.pdf
# quit

EOF
`
##
###############################

script="$script$script_footer"

gnuplot<<EOF
$(echo "$script")
EOF


