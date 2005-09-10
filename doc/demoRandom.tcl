# $Id$ #

#
#/** Demo of uniform and normal random distribution. \file demoRandom.tcl  \ingroup jmath */
#

package require jmath

set min [jmath::create_vec 2 "-5 -5"]
set max [jmath::create_vec 2 "5 5"]

set uniform [jmath::new_MultiDimUniformDistribution $min $max]

set mean [jmath::create_vec 2 "0 0"]
set cov  [jmath::create_vec 2 "2 1"]

set normal [jmath::new_MultiDimNormalDistribution $mean $cov]

set nbSamples 5000

set data [kernel::new_DataLog "demoRandom.dat"]

$data writeTime
$data writeComment "plot 'demoRandom.dat' u 1:2, '' u 3:4"
$data writeComment "u_x u_y n_x n_y"

for {set i 0} {$i < $nbSamples} {incr i} {
    set u [$uniform get]
    set n [$normal get]
    $data write "[jmath::vec_get $u 0] [jmath::vec_get $u 1] [jmath::vec_get $n 0] [jmath::vec_get $n 1]"
}

# set gp [open "demoRandom.gnuplot" "w"]
# puts $gp "plot 'demoRandom.dat' u 1:2, '' u 3:4"
# close $gp
