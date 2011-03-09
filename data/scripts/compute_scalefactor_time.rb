#!/usr/bin/ruby

# This script plots the scale factor of a trajectory compared to another one
# with time. For example a mono slam without prediction compared to ground truth.
# The two files must have timestamps at first column
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
min_dist=0.2

def dist(x1, y1, x2, y2)
	return Math::sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1))
end

def check_valid_line(line, col)
	if line[0] == 35 or line[0] == 0 or line[0] == 10 then return [],false end
	vec = line.split(' ')
	if vec.length <= col+1 then return [],false end
	return vec,true
end


slam_file = File.open(slam_filename)
truth_file = File.open(truth_filename)
res_file = File.open(res_filename,"w")

slam_line=""
truth_line=""
slam_vec=[]
truth_vec=[]

last_slam_x=0.0
last_slam_y=0.0

last_truth_x=0.0
last_truth_y=0.0
last_truth_time=0.0

prev_slam_x=0.0
prev_slam_y=0.0
prev_slam_time=0.0

# find first slam line
while (slam_line = slam_file.gets)
	slam_vec,ok = check_valid_line(slam_line, slam_col)
	if (not ok) then next end;
	
	last_slam_x = slam_vec[slam_col].to_f
	last_slam_y = slam_vec[slam_col+1].to_f

	prev_slam_x = last_slam_x
	prev_slam_y = last_slam_y
	prev_slam_time = slam_vec[0].to_f
	
	break
end
print "first slam #{slam_vec[0].to_f} #{slam_vec[slam_col].to_f} #{slam_vec[slam_col+1].to_f}\n"

# find first truth line after first slam line
while (truth_line = truth_file.gets)
	truth_vec,ok = check_valid_line(truth_line, truth_col)
	if (not ok) then next end;

	if (truth_vec[0].to_f >= prev_slam_time)
		last_truth_x = truth_vec[truth_col].to_f
		last_truth_y = truth_vec[truth_col+1].to_f
		last_truth_time = truth_vec[0].to_f
		
		break;
	end
end
print "first truth #{truth_vec[0].to_f} #{truth_vec[truth_col].to_f} #{truth_vec[truth_col+1].to_f}\n"

# find first slam line after truth line
while (slam_line = slam_file.gets)
	slam_vec,ok = check_valid_line(slam_line, slam_col)
	if (not ok) then next end;
	
	if (slam_vec[0].to_f >= last_truth_time)
		time_coeff = (truth_vec[0].to_f-prev_slam_time)/(slam_vec[0].to_f-prev_slam_time)
		last_slam_x = prev_slam_x + (slam_vec[slam_col  ].to_f-prev_slam_x) * time_coeff
		last_slam_y = prev_slam_y + (slam_vec[slam_col+1].to_f-prev_slam_y) * time_coeff
		break
	end

	prev_slam_x = slam_vec[slam_col].to_f
	prev_slam_y = slam_vec[slam_col+1].to_f
	prev_slam_time = slam_vec[0].to_f
end
print "second slam #{slam_vec[0].to_f} #{slam_vec[slam_col].to_f} #{slam_vec[slam_col+1].to_f} ; interpolated #{last_slam_x} #{last_slam_y}\n"


# find truth line after the robot has made min_dist
while (truth_line = truth_file.gets)
	truth_vec,ok = check_valid_line(truth_line, truth_col)
	if (not ok) then next end;

	truth_dist = dist(last_truth_x, last_truth_y, truth_vec[truth_col].to_f, truth_vec[truth_col+1].to_f)
	
	if (truth_dist > min_dist)
		
		# find slam line after truth line
		while (slam_line = slam_file.gets)
			slam_vec,ok = check_valid_line(slam_line, slam_col)
			if (not ok) then next end;
	
			dobreak = 0
			if (slam_vec[0].to_f >= truth_vec[0].to_f)
				time_coeff = (truth_vec[0].to_f-prev_slam_time)/(slam_vec[0].to_f-prev_slam_time)
				current_slam_x = prev_slam_x + (slam_vec[slam_col  ].to_f-prev_slam_x) * time_coeff
				current_slam_y = prev_slam_y + (slam_vec[slam_col+1].to_f-prev_slam_y) * time_coeff

				slam_dist = dist(last_slam_x, last_slam_y, current_slam_x, current_slam_y)
				scale_factor = slam_dist/truth_dist;
				avetime = (truth_vec[0].to_f + last_truth_time) / 2.0
				res_file.print("#{avetime} #{scale_factor}\n")
				
				#print "at time slam #{slam_vec[0].to_f} truth #{truth_vec[0].to_f}\n"
				
				print "between time #{last_truth_time} and #{truth_vec[0].to_f}: sf of #{scale_factor} ; truth moved of #{truth_dist} (#{last_truth_x},#{last_truth_y} to #{truth_vec[truth_col].to_f},#{truth_vec[truth_col+1].to_f}) and slam moved of #{slam_dist} (#{last_slam_x},#{last_slam_y} to #{current_slam_x},#{current_slam_y})\n"
				
				last_slam_x = current_slam_x
				last_slam_y = current_slam_y
				last_truth_x = truth_vec[truth_col].to_f
				last_truth_y = truth_vec[truth_col+1].to_f
				last_truth_time = truth_vec[0].to_f

				dobreak = 1
			end
			
			prev_slam_x = slam_vec[slam_col].to_f
			prev_slam_y = slam_vec[slam_col+1].to_f
			prev_slam_time = slam_vec[0].to_f

			if (dobreak == 1) then break end;
		end

	end
	
end

slam_file.close
truth_file.close
res_file.close

