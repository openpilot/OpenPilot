require 'jafar/kernel'
require 'jafar/geom'
require 'jafar/rtslam/rtslam'
Jafar.register_module Jafar::Rtslam

module Jafar
module Rtslam

# The common origin is the initial position of the truth with null orientation
# in the truth frame.
# The truth file has to be manually cut and start simultaneously with slam file
	
def Rtslam.sync_truth(slam_filename, truth_filename, res_filename)
	# transforms
	truth2rtruth_arr = [-0.013, -0.005, -0.012, 0, 0, 0]
	truth2rtruth_vec = Jmath::arrayToVec(truth2rtruth_arr)
	truth2rtruth_t3d = Geom::T3DEuler.new(truth2rtruth_vec)

	slam2rslam_arr = [0, 0, 0, 0, 0, Math::PI]
	slam2rslam_vec = Jmath::arrayToVec(slam2rslam_arr)
	slam2rslam_t3d = Geom::T3DEuler.new(slam2rslam_vec)
	
	# values
	slam_t_col=0
	slam_x_col=1
	slam_y_col=2
	slam_z_col=3
	slam_w_col=8
	slam_p_col=9
	slam_r_col=10

	truth_t_col=0
	truth_x_col=1
	truth_y_col=2
	truth_z_col=3
	truth_w_col=19
	truth_p_col=18
	truth_r_col=17

	# uncertainties (sigmas)
	slam_sx_col=23
	slam_sy_col=24
	slam_sz_col=25
	slam_sw_col=30
	slam_sp_col=31
	slam_sr_col=32

	truth_sx_col=-1
	truth_sy_col=-1
	truth_sz_col=-1
	truth_sw_col=-1
	truth_sp_col=-1
	truth_sr_col=-1

	truth_sx=0.001
	truth_sy=0.001
	truth_sz=0.001
	truth_sw=2.0/200.0
	truth_sp=2.0/200.0
	truth_sr=2.0/200.0

	# format
	slam_ncol=45
	truth_ncol=20

	slam_file = File.open(slam_filename)
	truth_file = File.open(truth_filename)
	res_file = File.open(res_filename,"w")

	init = true
	
	truth_final_t = 0
	truth_prev_t = 0
	rtruth_prev_t3d = Geom::T3DEuler.new(true)
	rtruth_raw_t3d = Geom::T3DEuler.new(true)
	rtruth_final_t3d = Geom::T3DEuler.new(true)
	rslam_raw_t3d = Geom::T3DEuler.new(true)
	truth_transfo_t3d = Geom::T3DEuler.new(false)
	slam_transfo_t3d = Geom::T3DEuler.new(false)
	error_final_t3d = Geom::T3DEuler.new(true)

	truthP_raw_arr = [truth_sx, truth_sy, truth_sz, truth_sw, truth_sp, truth_sr]
	truthP_raw_vec = Jmath::arrayToVec(truthP_raw_arr)
	
	# get slam line
	while (slam_line = slam_file.gets)
		if slam_line[0] == 35 or slam_line[0] == 0 or slam_line[0] == 10 then next end
		slam_strvec = slam_line.split(' ')
		if slam_strvec.length < slam_ncol then next end
		
		slam_raw_t = slam_strvec[slam_t_col].to_f
		slam_raw_arr = [slam_strvec[slam_x_col].to_f, slam_strvec[slam_y_col].to_f, slam_strvec[slam_z_col].to_f,
		                slam_strvec[slam_w_col].to_f, slam_strvec[slam_p_col].to_f, slam_strvec[slam_r_col].to_f]
		slamP_raw_arr = [slam_strvec[slam_sx_col].to_f, slam_strvec[slam_sy_col].to_f, slam_strvec[slam_sz_col].to_f,
		                 slam_strvec[slam_sw_col].to_f, slam_strvec[slam_sp_col].to_f, slam_strvec[slam_sr_col].to_f]
		slam_raw_vec = Jmath::arrayToVec(slam_raw_arr)
		slamP_raw_vec = Jmath::arrayToVec(slamP_raw_arr)
		slam_raw_t3d = Geom::T3DEuler.new(slam_raw_vec, slamP_raw_vec)
		
		if init then slam_transfo_t = slam_raw_t  end
		slam_final_t = slam_raw_t - slam_transfo_t
		
		# get truth line
		if init or truth_final_t < slam_final_t then
			if not init then truth_prev_t = truth_final_t; rtruth_prev_t3d = rtruth_final_t3d end
			
			while (truth_line = truth_file.gets)
				if truth_line[0] == 35 or truth_line[0] == 0 or truth_line[0] == 10 then next end
				truth_strvec = truth_line.split(' ')
				if truth_strvec.length < truth_ncol then next end
				
				truth_raw_t = truth_strvec[truth_t_col].to_f
				truth_raw_arr = [truth_strvec[truth_x_col].to_f, truth_strvec[truth_y_col].to_f, truth_strvec[truth_z_col].to_f,
												truth_strvec[truth_w_col].to_f, truth_strvec[truth_p_col].to_f, truth_strvec[truth_r_col].to_f]
				truth_raw_vec = Jmath::arrayToVec(truth_raw_arr)
				truth_raw_t3d = Geom::T3DEuler.new(truth_raw_vec, truthP_raw_vec)
				
				if init then truth_transfo_t = truth_raw_t  end
				truth_final_t = truth_raw_t - truth_transfo_t
					
				if init or truth_final_t >= slam_final_t then break end
			end
		end
		
		# convert to robot center
		rtruth_raw_t3d = Geom::T3D::composeToEuler(truth_raw_t3d, truth2rtruth_t3d)
		rslam_raw_t3d = Geom::T3D::composeToEuler(slam_raw_t3d, slam2rslam_t3d)
		
		# 
		if (init) then
			rslam_final_arr = [0, 0, 0, rtruth_raw_t3d.getX.get(3), rtruth_raw_t3d.getX.get(4), rtruth_raw_t3d.getX.get(5)]
			rslam_final_vec = Jmath::arrayToVec(rslam_final_arr)
			rslam_final_t3d = Geom::T3DEuler.new(rslam_final_vec, true)
			rtruth_final_t3d = Geom::T3DEuler.new(rslam_final_vec, truthP_raw_vec)
			rtruth_inter_t3d = rtruth_final_t3d
			
			truth_transfo_t3d = Geom::T3D::composeToEuler(rtruth_final_t3d, Geom::T3D::invToEuler(rtruth_raw_t3d))
			slam_transfo_t3d = Geom::T3D::composeToEuler(rslam_final_t3d, Geom::T3D::invToEuler(rslam_raw_t3d))
# print(Jafar::Geom::print(truth_raw_t3d))
# print(Jafar::Geom::print(truth2rtruth_t3d))
# print(Jafar::Geom::print(rtruth_raw_t3d))
# print(Jafar::Geom::print(slam_raw_t3d))
# print(Jafar::Geom::print(rslam_raw_t3d))
# print(Jafar::Geom::print(truth_transfo_t3d))
# print(Jafar::Geom::print(slam_transfo_t3d))
			
			# remove cov
			truth_transfo_t3d.set(truth_transfo_t3d.getX)
			slam_transfo_t3d.set(slam_transfo_t3d.getX)
			
			init = false
		else
			rslam_final_t3d = Geom::T3D::composeToEuler(slam_transfo_t3d, rslam_raw_t3d)
			rtruth_final_t3d = Geom::T3D::composeToEuler(truth_transfo_t3d, rtruth_raw_t3d)
			
			# interpolation FIXME deal better with angles and modulo
			coeff = (slam_final_t - truth_prev_t) / (truth_final_t - truth_prev_t)
			rtruth_inter_t3d = Geom::T3DEuler.new(
				rtruth_prev_t3d.getX    + (rtruth_final_t3d.getX    - rtruth_prev_t3d.getX   ) * coeff,
				rtruth_prev_t3d.getXCov + (rtruth_final_t3d.getXCov - rtruth_prev_t3d.getXCov) * coeff)
		end
		
		# error
		error_final_t3d = Geom::T3D::composeToEuler(rslam_final_t3d, Geom::T3D::invToEuler(rtruth_inter_t3d))
	
		rslam_final_t3d_dev = rslam_final_t3d.getXStdDev
		rtruth_final_t3d_dev = rtruth_inter_t3d.getXStdDev
		error_final_t3d_dev = error_final_t3d.getXStdDev
		
		res_line = "\
#{slam_final_t} \
#{rslam_final_t3d.getX.get(0)} #{rslam_final_t3d.getX.get(1)} #{rslam_final_t3d.getX.get(2)} \
#{rslam_final_t3d.getX.get(3)} #{rslam_final_t3d.getX.get(4)} #{rslam_final_t3d.getX.get(5)} \
#{rslam_final_t3d_dev.get(0)} #{rslam_final_t3d_dev.get(1)} #{rslam_final_t3d_dev.get(2)} \
#{rslam_final_t3d_dev.get(3)} #{rslam_final_t3d_dev.get(4)} #{rslam_final_t3d_dev.get(5)} \
#{rtruth_inter_t3d.getX.get(0)} #{rtruth_inter_t3d.getX.get(1)} #{rtruth_inter_t3d.getX.get(2)} \
#{rtruth_inter_t3d.getX.get(3)} #{rtruth_inter_t3d.getX.get(4)} #{rtruth_inter_t3d.getX.get(5)} \
#{rtruth_final_t3d_dev.get(0)} #{rtruth_final_t3d_dev.get(1)} #{rtruth_final_t3d_dev.get(2)} \
#{rtruth_final_t3d_dev.get(3)} #{rtruth_final_t3d_dev.get(4)} #{rtruth_final_t3d_dev.get(5)} \
#{error_final_t3d.getX.get(0)} #{error_final_t3d.getX.get(1)} #{error_final_t3d.getX.get(2)} \
#{error_final_t3d.getX.get(3)} #{error_final_t3d.getX.get(4)} #{error_final_t3d.getX.get(5)} \
#{error_final_t3d_dev.get(0)} #{error_final_t3d_dev.get(1)} #{error_final_t3d_dev.get(2)} \
#{error_final_t3d_dev.get(3)} #{error_final_t3d_dev.get(4)} #{error_final_t3d_dev.get(5)}"
		
		res_file.puts(res_line)
	end


	slam_file.close
	truth_file.close
	res_file.close

end

end
end
			
