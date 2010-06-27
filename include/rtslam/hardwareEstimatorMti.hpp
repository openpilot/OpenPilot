/**
 * \file hardwareEstimatorMti.hpp
 *
 * Header file for hardware robots
 *
 * \author croussil@laas.fr
 * \date 18/06/2010
 *
 * \ingroup rtslam
 */

#ifndef HARDWARE_ESTIMATOR_MTI_HPP_
#define HARDWARE_ESTIMATOR_MTI_HPP_

#include "jafarConfig.h"
#ifdef HAVE_MTI

#include <MTI-clients/MTI.h>

#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "jmath/jblas.hpp"

#include "rtslam/hardwareEstimatorAbstract.hpp"



namespace jafar {
namespace rtslam {
namespace hardware {

	class HardwareEstimatorMti: public HardwareEstimatorAbstract
	{
		private:
			MTI mti;
			
			boost::mutex mutex_data;
			jblas::mat buffer;
			jblas::mat published;
			int bufferSize;
			int position;

			boost::thread *preloadTask_thread;
			void preloadTask(void);
		
		public:
			
			HardwareEstimatorMti(std::string device, int bufferSize_);
			
			jblas::mat_range acquireReadings(double t1, double t2);

	};

}}}

#endif // #ifdef HAVE_MTI
#endif

