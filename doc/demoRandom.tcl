# $Id$ #

#
#/** Demo of uniform and normal random distribution. \file demoRandom.tcl  \ingroup jmath */
#

package require jmath

set min [jmath::new_vec 2]
jmath::setValue $min "(-5, -5)"

set max [jmath::new_vec 2]
jmath::setValue $max "(5, 5)"

set uniform [jmath::new_MultiDimUniformDistribution $min $max]

set mean [jmath::new_vec 2]
jmath::setValue $mean "(0, 0)"

set cov [jmath::new_vec 2]
jmath::setValue $cov "(2, 1)"

set normal [jmath::new_MultiDimNormalDistribution $mean $cov]

set nbSamples 5000

set data [kernel::new_DataLogger "demoRandom.dat"]

$data writeCurrentDate
$data writeComment "plot 'demoRandom.dat' u 1:2, '' u 3:4"
$data writeComment "u_x u_y n_x n_y"

for {set i 0} {$i < $nbSamples} {incr i} {
    set u [$uniform get]
    set n [$normal get]
    $data writeData "[jmath::vec_get $n 0] [jmath::vec_get $n 1] \n"
}

# set gp [open "demoRandom.gnuplot" "w"]
# puts $gp "plot 'demoRandom.dat' u 1:2, '' u 3:4"
# close $gp
