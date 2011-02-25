#!/bin/sh

# This script takes a log file created by the MTI-genom module and converts
# it to the rtslam format. The columns are not in the same order, and the format
# is tab separated values for MTI-genom log and jblas::vec>> for rtslam.
#
# Author: croussil

awk ' { print "[10]("$1","$9","$10","$11","$6","$7","$8","$12","$13","$14")"; } ' $1 > $2
