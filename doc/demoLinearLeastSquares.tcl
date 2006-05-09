# $Id$ #

# Linear Least Squares demo
# simple line fitting: y = a.x + b

# line parameters
set line_a 2.5
set line_b 11

set sizeDataSet 100000

package require jmath

# random x points in [-100,100]
set random [jmath::new_UniformDistribution -100 100]

# measure gaussian noise
set noise [jmath::new_NormalDistribution 0.0 1.0]

# lls class
set lls [::jmath::new_LinearLeastSquares]
$lls setSize 2 $sizeDataSet

# lls reference to A.x = b
set lls_A [$lls A]
set lls_b [$lls b]

for {set i 0} {$i < $sizeDataSet} {incr i} {
    set x [$random get]
    set y [expr $line_a * $x + $line_b + [$noise get]]
    # set y [expr $line_a * $x + $line_b]

#    puts "y: $y - x: $x"

    $lls_A set $i 0 $x
    $lls_A set $i 1 1.0

    $lls_b set $i $y
}

kernel::tic
$lls solve
puts "elapsed: [kernel::toc] ms"

puts "(a,b) = [jmath::print [$lls x]]"
puts "cov(a,b) = \n [jmath::prettyPrint [$lls xCov]]"
