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
			int bufferSize;
			int position; // next position
			int reading_pos;
			int read_pos;
			
			int mode;
			std::string dump_path;
			double realFreq;

			boost::thread *preloadTask_thread;
			void preloadTask(void);
		
		public:
			
			HardwareEstimatorMti(std::string device, double freq, double shutter, int bufferSize_, int mode = 0, std::string dump_path = ".");
			
			jblas::mat_indirect acquireReadings(double t1, double t2);
			void releaseReadings() { reading_pos = -1; }

			double getFreq() { return realFreq; }
	};

}}}

#endif // #ifdef HAVE_MTI
#endif

