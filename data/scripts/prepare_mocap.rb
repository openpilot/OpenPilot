#!/usr/bin/ruby

# This script preprocesses motion capture files so that they can be read by
# the Octave script convert_mocap_2_pose.m, with two operations:
# - putting a leading # on the first 6 lines to show that they are comments
# - removing all lines with missing data (which contains two consecutive tabs)
#
# Author: croussil


in_filename=ARGV[0]
out_filename=ARGV[1]

in_file = File.open(in_filename)
out_file = File.open(out_filename,"w")

for i in 1..6
	in_line = in_file.gets
	out_line = "#" + in_line
	out_file.print(out_line)
end

while (in_line = in_file.gets)
	if not in_line.include? "\t\t"
		out_file.print(in_line)
	end
end

in_file.close
out_file.close

