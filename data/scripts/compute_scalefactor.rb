#!/usr/bin/ruby

# This script plots the scale factor of a trajectory compared to another one
# with time. For example a mono slam without prediction compared to ground truth.
# The two files must have the same number or lines (except for comments width leading #)
# and be synchronized.
#
# The two files are processed in parallel, starting with the second one (truth).
# When the ground truth has moved more than min_dist, the scale factor is computed.
#
# Author: croussil


slam_filename=ARGV[0]
slam_col=Integer(ARGV[1])
truth_filename=ARGV[2]
truth_col=Integer(ARGV[3])
res_filename=ARGV[4]
min_dist=0.5

def dist(x1, y1, x2, y2)
	return Math::sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1))
end


slam_file = File.open(slam_filename)
truth_file = File.open(truth_filename)
res_file = File.open(res_filename,"w")

first=1
last_slam_x=0.0
last_slam_y=0.0
last_truth_x=0.0
last_truth_y=0.0
last_time=0.0

while (slam_line = slam_file.gets)
	if slam_line[0] == 35 or slam_line[0] == 0 or slam_line[0] == 10 then next end
	slam_vec = slam_line.split(' ')
	if slam_vec.length <= slam_col+1 then next end
	
	while (truth_line = truth_file.gets)
		if truth_line[0] == 35 or truth_line[0] == 0 or truth_line[0] == 10 then next end
		truth_vec = truth_line.split(' ')
		if truth_vec.length <= truth_col+1 then next end
		break
	end
	
	if (first == 1)
		last_slam_x = slam_vec[slam_col].to_f
		last_slam_y = slam_vec[slam_col+1].to_f
		last_truth_x = truth_vec[truth_col].to_f
		last_truth_y = truth_vec[truth_col+1].to_f
		last_time = slam_vec[0].to_f
		first = 0
		next
	end
	
	truth_dist = dist(last_truth_x, last_truth_y, truth_vec[truth_col].to_f, truth_vec[truth_col+1].to_f)
	if (truth_dist > min_dist)
		slam_dist = dist(last_slam_x, last_slam_y, slam_vec[slam_col].to_f, slam_vec[slam_col+1].to_f)
		scale_factor = slam_dist/truth_dist;
		avetime = (slam_vec[0].to_f + last_time) / 2.0
		res_file.print("#{avetime} #{scale_factor}\n")
		
		#print "at time slam #{slam_vec[0].to_f} truth #{truth_vec[0].to_f}\n"
		
		print "between time #{last_time} and #{slam_vec[0].to_f}: sf of #{scale_factor} ; truth moved of #{truth_dist} (#{last_truth_x},#{last_truth_y} to #{truth_vec[truth_col].to_f},#{truth_vec[truth_col+1].to_f}) and slam moved of #{slam_dist} (#{last_slam_x},#{last_slam_y} to #{slam_vec[slam_col].to_f},#{slam_vec[slam_col+1].to_f})\n"
		
		last_slam_x = slam_vec[slam_col].to_f
		last_slam_y = slam_vec[slam_col+1].to_f
		last_truth_x = truth_vec[truth_col].to_f
		last_truth_y = truth_vec[truth_col+1].to_f
		last_time = slam_vec[0].to_f
	end
end

slam_file.close
truth_file.close
res_file.close

