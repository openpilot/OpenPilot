/**
 * \file hardwareEstimatorMti.hpp
 *
 * Header file for hardware robots
 *
 * \date 18/06/2010
 * \author croussil
 *
 * \ingroup rtslam
 */

#ifndef HARDWARE_ESTIMATOR_MTI_HPP_
#define HARDWARE_ESTIMATOR_MTI_HPP_

#include "jafarConfig.h"

#ifdef HAVE_MTI
#include <MTI-clients/MTI.h>
#endif

#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "jmath/jblas.hpp"
#include "jmath/indirectArray.hpp"

#include "rtslam/hardwareEstimatorAbstract.hpp"



namespace jafar {
namespace rtslam {
namespace hardware {

	class HardwareEstimatorMti: public HardwareEstimatorAbstract
	{
		private:
#ifdef HAVE_MTI
			MTI *mti;
#endif
			
			jblas::mat buffer;
			int bufferSize;
			
			boost::mutex mutex_data;
			boost::condition_variable cond_data;
			boost::condition_variable cond_offline; // to be sure we don't need data before they are read
			int write_position; // next position where to write, oldest available reading
			int read_position; // oldest position not released (being read or not read at all)
			
			double timestamps_correction;
//			bool tighly_synchronized;
//			double tight_offset;
			
			int mode;
			std::string dump_path;
			double realFreq;

			boost::thread *preloadTask_thread;
			void preloadTask(void);
		
		public:
			
			HardwareEstimatorMti(std::string device, double trigger_mode, double trigger_freq, double trigger_shutter, int bufferSize_, int mode = 0, std::string dump_path = ".");
			~HardwareEstimatorMti();
			virtual void start();
			void setSyncConfig(double timestamps_correction = 0.0/*, bool tightly_synchronized = false, double tight_offset*/);
			
			/**
			 * @return data with 10 columns: time, accelero (3), gyro (3), magneto (3)
			 */
			jblas::mat_indirect acquireReadings(double t1, double t2);
			void releaseReadings() { }
			jblas::ind_array instantValues() { return jmath::ublasExtra::ia_set(1,10); }
			jblas::ind_array incrementValues() { return jmath::ublasExtra::ia_set(1,1); }

			double getFreq() { return realFreq; }
	};

}}}

#endif

