#!/bin/sh


# This script encodes a sequence of raw images in a movie file,
# adding frame number
#
# WARNING:
# - if you modify this script, don't forget to escape dollar symbol inside
#   subscripts.
#
# Author: croussil

if [ $# -eq 6 ]; then
	pattern=$1
	ext=$2
	first=$3
	last=$4
	fps=$5
	output=$6
else
	pattern_l=$1
	pattern_r=$2
	ext=$3
	first=$4
	last=$5
	fps=$6
	output=$7
fi;

quality=26


###############################################################################
script_process_images_mono=$(cat<<EOF

# sequence
for ((i = $first; \$i <= $last; i++)); do
	filename=\$(printf "$pattern" \$i)

	convert \
		\$filename.$ext \
		-pointsize 20 -fill red \
		-draw "text 5,20 \"\$i\" " \
		-draw "text 5,40 \"\$(cat \${filename}.time)\" " \
		-quality 100 \
		jpg:-

done;

EOF
)


script_process_images_stereo=$(cat<<EOF

# sequence
for ((i = $first; \$i <= $last; i++)); do
	fname_l=\$(printf "$pattern_l" \$i)
	fname_r=\$(printf "$pattern_r" \$i)

	convert \$fname_l.$ext \$fname_r.$ext +append ppm:- | \
	convert - \
		-pointsize 20 -fill red \
		-draw "text  5,20 \"\$i\" " \
		-draw "text  5,40 \"\$(cat \${fname_l}.time)\" " \
		-draw "text 645,20 \"\$i\" " \
		-draw "text 645,40 \"\$(cat \${fname_r}.time)\" " \
		-quality 100 \
		jpg:-

done;

EOF
)  


if [ $# -eq 6 ]; then
	script_process_images=$script_process_images_mono
else
	script_process_images=$script_process_images_stereo
fi;


###############################################################################
script_process_video=$(cat<<EOF

mencoder \
	- \
	-demuxer lavf -lavfdopts format=mjpeg \
	-ovc x264 -x264encopts crf=$quality \
	-fps $fps \
	-noskip \
	-o $output

EOF
)


###############################################################################

sh -c "$script_process_images" | sh -c "$script_process_video"


