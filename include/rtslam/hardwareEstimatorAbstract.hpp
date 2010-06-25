/**
 * \file hardwareEstimatorAbstract.hpp
 *
 * Header file for hardware robots
 *
 * \author croussil@laas.fr
 * \date 18/06/2010
 *
 * \ingroup rtslam
 */

#ifndef HARDWARE_ESTIMATOR_ABSTRACT_HPP_
#define HARDWARE_ESTIMATOR_ABSTRACT_HPP_

namespace jafar {
namespace rtslam {
namespace hardware {

	class HardwareEstimatorAbstract;
	typedef boost::shared_ptr<HardwareEstimatorAbstract> hardware_estimator_ptr_t;


	class HardwareEstimatorAbstract
	{
		public:
			
			virtual void acquireReadings(double t1, double t2, jblas::mat &readings) = 0;
	};

}}}

#endif

