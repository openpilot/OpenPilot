set term wxt size 1024,768

# file to plot:
file="data/tmp/2011-02-15_inertial/rtslam.log"

# This script plots the evolution of estimation of IMU biases (ax,ay,az,g)
# with 3-sigma uncertainty.
#
# Author: croussil


# to have smaller crosses:
set pointsize 0.3333333333333

# to have the same scale along x and y axis:
#set size ratio -1

# to display a grid:
#set grid

# to have a different left y axis:
set y2tics auto
set ytics nomirror

# titles
set title "Accelero biases and g"
set xlabel "Time (s)"
set ylabel "Acceleration (m/s2)"
set y2label "Angle (deg)"

# to define some functions:
norm3(x,y,z)=sqrt(x*x+y*y+z*z)
pitch(x,y,z) = atan2(-x, z)*180/pi
roll(x,y,z) = atan2(y, z)*180/pi

# to define some variables:
t0=1281469305.9

# now plot:
plot \
	file using ($1-t0):15 with points pt 1 lt rgb "red"       title "Bias AX", \
	file using ($1-t0):($15-3*$37) with points pt 0 lt rgb "red"       title "", \
	file using ($1-t0):($15+3*$37) with points pt 0 lt rgb "red"       title "", \
	file using ($1-t0):16 with points pt 1 lt rgb "dark-red"  title "Bias AY", \
	file using ($1-t0):($16-3*$38) with points pt 0 lt rgb "dark-red"       title "", \
	file using ($1-t0):($16+3*$38) with points pt 0 lt rgb "dark-red"       title "", \
	file using ($1-t0):17 with points pt 1 lt rgb "orange"    title "Bias AZ", \
	file using ($1-t0):($17-3*$39) with points pt 0 lt rgb "orange"       title "", \
	file using ($1-t0):($17+3*$39) with points pt 0 lt rgb "orange"       title "", \
	file using ($1-t0):(norm3($21,$22,$23)-9.805) with points pt 1 lt rgb "blue"          title "Norm g - 9.805", \
	file using ($1-t0):(norm3($21,$22,$23)-9.805-3*norm3($43,$44,$45)) with points pt 0 lt rgb "blue"          title "", \
	file using ($1-t0):(norm3($21,$22,$23)-9.805+3*norm3($43,$44,$45)) with points pt 0 lt rgb "blue"          title "", \
	file using ($1-t0):(pitch($21,$22,$23)) axes x1y2 with points pt 1 lt rgb "green"     title "Pitch g", \
	file using ($1-t0):(roll($21,$22,$23)) axes x1y2 with points pt 1 lt rgb "dark-green" title "Roll g"


# prevent window from closing at the end
#pause -1
