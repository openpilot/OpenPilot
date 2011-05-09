#!/bin/sh


# This script combines title image, legend image, 3D and 2D images and
# encodes the sequence in a movie file.
#
# FIXME:
# - the title image has to have at least one pixel with color, or the encoder
#   will segfault when reaching sequence images. Probably a bug of mencoder.
#
# WARNING:
# - if you modify this script, don't forget to escape dollar symbol inside
#   subscripts.
#
# Author: croussil


nframe_title=$1
first=$2
last=$3
fps=$4


###############################################################################
script_process_images=$(cat<<EOF

# title
for ((i = 0; \$i < $nframe_title; i++)); do
	cat title.png | convert -quality 100 - jpg:-
done;

# sequence
for ((i = $first; \$i <= $last; i++)); do
	num=\$(printf "%06d" \$i)

# 3D/2D/legend from individual images

	convert rendered-2D_2-\$num.png -resize 640x480 ppm:- \
		| convert rendered-3D_\$num.png - +append ppm:- \
			| convert legend.png - -append -quality 100 jpg:-


# 3D/2D/legend from 3D/2D and legend

#	convert \
#		legend.png \
#		rendered_\$num.png \
#		-append \
#		-quality 100 \
#		jpg:-

done;

EOF
)

npass=1
x264_string_mp="-ovc x264 -x264encopts crf=25:pass=\$p"
x264_string="-ovc x264 -x264encopts crf=25"
#webm_string="-ovc lavc -ffourcc VP80 -of lavf -lavfopts format=webm -lavcopts vcodec=libvpx:vqscale=22:vpass=\$p"
webm_string="-ovc lavc -ffourcc VP80 -of lavf -lavfopts format=webm -lavcopts vcodec=libvpx:vqscale=22:vpass=2"
#webm_string="-ovc lavc -ffourcc VP80 -of lavf -lavfopts format=webm -lavcopts vcodec=libvpx:vqscale=22.0"
#webm_string="-ovc lavc -ffourcc VP80 -of lavf -lavfopts format=webm -lavcopts vcodec=libvpx:vbitrate=50"

###############################################################################
script_process_video=$(cat<<EOF

for ((p = 1; \$p <= $npass; p++)); do

mencoder \
	- \
	-demuxer lavf -lavfdopts format=mjpeg \
	$x264_string \
	-vf scale=1280:512 -fps $fps \
	-noskip \
	-o rendered.avi

done

EOF
)


###############################################################################

sh -c "$script_process_images" | sh -c "$script_process_video"


