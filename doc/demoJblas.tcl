# $Id$ #

#
#/** Demo of jblas vector. \file demoJblas.tcl  \ingroup jmath */
#


package require jmath

set v [jmath::new_vec 10]
jmath::setValue $v "(0, 1, 2, 3, 4, 5, 6, 7, 8, 9)"

jmath::print $v

for {set i 0} {$i < [$v size]} {incr i} {
    $v set $i [expr "2*[$v get $i]"]
}

jmath::print $v

