#!/bin/sh


# This script encodes a sequence of raw images in a movie file,
# adding frame number
#
# WARNING:
# - if you modify this script, don't forget to escape dollar symbol inside
#   subscripts.
#
# Author: croussil

pattern=$1
first=$2
last=$3
fps=$4


###############################################################################
script_process_images=$(cat<<EOF

# sequence
for ((i = $first; \$i <= $last; i++)); do
	num=\$(printf "$pattern" \$i)
	index=\$((\$i-$first))

	convert \
		rendered_\$num.png \
		-pointsize 20 -fill red -draw 'text 10,30 "\$index" ' \
		-quality 100 \
		jpg:-

done;

EOF
)


###############################################################################
script_process_video=$(cat<<EOF

mencoder \
	- \
	-demuxer lavf -lavfdopts format=mjpeg \
	-ovc x264 -x264encopts crf=22.0 \
	-fps $fps \
	-noskip \
	-o rendered.avi

EOF
)


###############################################################################

sh -c "$script_process_images" | sh -c "$script_process_video"


