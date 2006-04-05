# $Id$ #

#
#/** Demo of the plot command. \file demoPlot.tcl  \ingroup jmath */
#

package require jmath

set n 100

set data [jmath::new_mat  $n 3]

for {set i 0} {$i < $n} {incr i} {
    set x [expr "$i/10.0"]

    $data set $i 0 $x
    $data set $i 1 [expr "pow($x,2)"]
    $data set $i 2 [expr "pow($x,3)"]

}

jmath::plot $data "using 1:2 with lines, '' using 1:3 with lines" "demo_plot"
