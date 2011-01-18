/**
 * \file hardwareEstimatorInertialAdhocSimulator.hpp
 *
 * Header file for hardware robots
 *
 * \author croussil
 * \date 12/01/2011
 *
 * \ingroup rtslam
 */

#ifndef HARDWARE_ESTIMATOR_INERTIAL_ADHOC_SIMULATOR_HPP_
#define HARDWARE_ESTIMATOR_INERTIAL_ADHOC_SIMULATOR_HPP_

#include "jafarConfig.h"

#include "jmath/jblas.hpp"
#include "jmath/indirectArray.hpp"

#include "rtslam/hardwareEstimatorAbstract.hpp"


namespace jafar {
namespace rtslam {
namespace hardware {

	class HardwareEstimatorInertialAdhocSimulator: public HardwareEstimatorAbstract
	{
		private:
			double dt;
			boost::shared_ptr<simu::AdhocSimulator> simulator;
			size_t robId;
			
			jblas::mat buffer;
			int bufferSize;
			
		private:
			struct estimator_params_t
			{
				double gravity;
				jblas::vec gyr_bias;
				jblas::vec gyr_gain;
				MultiDimNormalDistribution *gyr_noise;
				jblas::vec acc_bias;
				jblas::vec acc_gain;
				MultiDimNormalDistribution *acc_noise;
				double timestamps_correction;
				
				estimator_params_t(): gravity(9.81),
					gyr_bias(3), gyr_gain(3), gyr_noise(NULL),
					acc_bias(3), acc_gain(3), acc_noise(NULL),
					timestamps_correction(0.0) {}
				~estimator_params_t() { delete gyr_noise; delete acc_noise; }
			} params;
			
		public:
			/**
			 * @param freq the acquisition, must be a multiple of sensor frequence if you want to simulate synchronization
			 * @param bufferSize_
			 * @param simulator
			 * @param robId
			 */
			HardwareEstimatorInertialAdhocSimulator(double freq, int bufferSize_,
				boost::shared_ptr<simu::AdhocSimulator> simulator, size_t robId):
				dt(1./freq), simulator(simulator), robId(robId), buffer(bufferSize_, 10), bufferSize(bufferSize_) {}

			void setSyncConfig(double timestamps_correction = 0.0/*, bool tightly_synchronized = false, double tight_offset*/)
			{
				params.timestamps_correction = timestamps_correction;
			}
			
			void setErrors(double gravity = 9.81,
			               double gyr_bias = 0.0, double gyr_bias_noisestd = 0.0,
			               double gyr_gain = 1.0, double gyr_gain_noisestd = 0.0,
			               double gyr_noisestd = 0.0,
			               double acc_bias = 0.0, double acc_bias_noisestd = 0.0,
			               double acc_gain = 1.0, double acc_gain_noisestd = 0.0,
			               double acc_noisestd = 0.0)
			{
				jblas::scalar_vec x, P;
				params.gravity = gravity;
				
				x = jblas::scalar_vec(3, gyr_bias); P = jblas::scalar_vec(3, gyr_bias_noisestd*gyr_bias_noisestd);
				params.gyr_bias = MultiDimNormalDistribution(x, P, rtslam::rand()).get();
				x = jblas::scalar_vec(3, gyr_gain); P = jblas::scalar_vec(3, gyr_gain_noisestd*gyr_gain_noisestd);
				params.gyr_gain = MultiDimNormalDistribution(x, P, rtslam::rand()).get();
				x = jblas::scalar_vec(3, 0.0); P = jblas::scalar_vec(3, gyr_noisestd*gyr_noisestd);
				params.gyr_noise = new MultiDimNormalDistribution(x, P, rtslam::rand());
				
				x = jblas::scalar_vec(3, acc_bias); P = jblas::scalar_vec(3, acc_bias_noisestd*acc_bias_noisestd);
				params.acc_bias = MultiDimNormalDistribution(x, P, rtslam::rand()).get();
				x = jblas::scalar_vec(3, acc_gain); P = jblas::scalar_vec(3, acc_gain_noisestd*acc_gain_noisestd);
				params.acc_gain = MultiDimNormalDistribution(x, P, rtslam::rand()).get();
				x = jblas::scalar_vec(3, 0.0); P = jblas::scalar_vec(3, acc_noisestd*acc_noisestd);
				params.acc_noise = new MultiDimNormalDistribution(x, P, rtslam::rand());
			}
			
			jblas::mat_indirect acquireReadings(double t1, double t2)
			{
				size_t n1 = (int)(t1/dt);
				size_t n2 = (int)(t2/dt +1-1e-4);
				size_t i = 0;
				for(size_t n = n1; n <= n2; ++n,++i)
				{
					double t = n*dt;
					
					jblas::vec tmp = simulator->getRobotInertial(robId, t);
					ublas::subrange(tmp, 0, 3) = ublas::element_prod(ublas::subrange(tmp, 0, 3), params.acc_gain) + 
						params.acc_bias + params.acc_noise->get();
					ublas::subrange(tmp, 3, 6) = ublas::element_prod(ublas::subrange(tmp, 3, 6), params.gyr_gain) + 
						params.gyr_bias + params.gyr_noise->get();
						
					buffer(i,0) = t + params.timestamps_correction;
					buffer(i,1) = tmp(0);
					buffer(i,2) = tmp(1);
					buffer(i,3) = tmp(2);
					buffer(i,4) = tmp(3);
					buffer(i,5) = tmp(4);
					buffer(i,6) = tmp(5);
					buffer(i,7) = 0.;
					buffer(i,8) = 0.;
					buffer(i,9) = 0.; // TODO magneto
				}
				return ublas::project(buffer, 
					jmath::ublasExtra::ia_set(ublas::range(0,i)),
					jmath::ublasExtra::ia_set(ublas::range(0,buffer.size2())));
			}
			
			void releaseReadings() { }
			jblas::ind_array instantValues() { return jmath::ublasExtra::ia_set(1,10); }
			jblas::ind_array incrementValues() { return jmath::ublasExtra::ia_set(1,1); }
	};

}}}

#endif

