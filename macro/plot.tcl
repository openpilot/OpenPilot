# $Id$ #

#
#/** Provides front-end to gnuplot to plot data in matrix. (limited to one plot !?) \file plot.tcl  \ingroup jmath */
#

namespace eval jmath {

    proc plot {data format {title "plot"}} {
        # call gnuplot to plot data with format
        # data is saved in a temporary file before plotting
        # format should not refer to any data file, example:
        # "using 1:2 with lines, '' using 1:3 with lines"

        # save data in temporary file
        set nRows [$data size1]
        set nCols [$data size2]
        set dataFileName "${title}.dat.tmp"
        set dataFile [open $dataFileName "w"]		
        puts $dataFile "\# $dataFileName - temporary file can safely be deleted"
		
        for {set i 0} {$i < $nRows} {incr i} {
            for {set j 0} {$j < $nCols} {incr j} {
                puts -nonewline $dataFile "[$data get $i $j] "
            }
            puts $dataFile " "
        }
        close $dataFile


        set gp [open "|gnuplot" w]
        puts $gp "set term x11 persist"
        puts $gp "set title '$title'"
        puts -nonewline $gp "plot "
        puts $gp "\"$dataFileName\" $format"
        close $gp
    }

    proc displayMat {mat maxValue} {
        set nRows [$mat size1]
        set nCols [$mat size2]
        set display [open "|display -resize 400x400+256 -" w]
        puts $display "P3"
        puts $display "$nCols $nRows"
        puts $display "255"
        for {set i 0} {$i < $nRows} {incr i} {
            for {set j 0} {$j < $nCols} {incr j} {
                puts -nonewline $display "[_displayMatPixelColor [$mat get $j $i] $maxValue] "
            }
            puts -nonewline $display "\n"
        }
        close $display
    }

    proc _displayMatPixelColor {value maxValue} {
        set c [expr "round(255 - 255*abs($value)/$maxValue)"]
        if "$value>=0" {
            return "$c $c 255"
        } else {
            return "255 $c $c"
        }
    }

}

package provide jmath 0.1
