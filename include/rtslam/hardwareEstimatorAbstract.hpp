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
			/**
			Returns all reading between t1 and t2, plus the last before t1 and the first after t2.
			Each line is one reading, first element is timestamp (double seconds), the rest is the command.
			*/
			virtual jblas::mat_indirect acquireReadings(double t1, double t2) = 0;
			/**
			
			*/
			virtual void releaseReadings() = 0;
	};

}}}

#endif

