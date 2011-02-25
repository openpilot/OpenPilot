#!/bin/sh

# this script takes the output files of replay dump and make a video with it
#
# Author: croussil

first=$1
last=$2
fps=$3
 
for ((i = $first; $i <= $last; i++)); do
	num=`printf "%06d" $i`;
	echo converting frame $num

	convert \
		rendered-3D_$num.png\
		rendered-2D_1-$num.png \
		+append \
		rendered_$num.png;

done

mencoder \
	-ovc x264 -x264encopts crf=22.0 \
	-vf scale=1280:480 \
	"mf://rendered_*.png" -mf fps=$fps \
	-noskip \
	-o rendered.avi

