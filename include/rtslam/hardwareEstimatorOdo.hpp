/**
 * \file hardwareEstimatorOdo.hpp
 *
 * Header file for hardware odometry robots
 *
 * \date 18/01/2012
 * \author dmarquez
 *
 * \ingroup rtslam
 */

#ifndef HARDWARE_ESTIMATOR_ODO_HPP_
#define HARDWARE_ESTIMATOR_ODO_HPP_

#include "jafarConfig.h"
#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include "jmath/jblas.hpp"
#include "jmath/indirectArray.hpp"
#include "rtslam/hardwareEstimatorAbstract.hpp"
#include "rtslam/hardwareSensorAbstract.hpp"
#include "kernel/keyValueFile.hpp"

namespace jafar {
namespace rtslam {
namespace hardware {
	class Position;

	class HardwareEstimatorOdo: public HardwareEstimatorAbstract
	{
		private:		
			unsigned index_load_;
			jblas::mat buffer;
			int bufferSize;
			
			boost::mutex mutex_data;
			boost::condition_variable cond_data;
			boost::condition_variable cond_offline; // to be sure we don't need data before they are read
			int write_position; // next position where to write, oldest available reading
			int read_position; // oldest position not released (being read or not read at all)
			
			double timestamps_correction;			
			int mode;
			RawVec reading;
			std::string dump_path;
			double realFreq;

			boost::thread *preloadTask_thread;
			void preloadTask(void);
		
		public:
			
			HardwareEstimatorOdo(double trigger_mode, double trigger_freq, double trigger_shutter, int bufferSize_, int mode = 0, std::string dump_path = ".");
			~HardwareEstimatorOdo();
			virtual void start();
			void setSyncConfig(double timestamps_correction = 0.0/*, bool tightly_synchronized = false, double tight_offset*/);
			
			/**
			 * @return data position: x y z roll pitch yaw
			 */
			jblas::mat_indirect acquireReadings(double t1, double t2);
			void releaseReadings() { }
			jblas::ind_array instantValues() { return jmath::ublasExtra::ia_set(1,7); }
			jblas::ind_array incrementValues() { return jmath::ublasExtra::ia_set(1,1); }

			double getFreq() { return realFreq; }
			
			Position loadPosition(unsigned int index_) const;
			void loadPosition(unsigned int index_, Position& pos) const;
	};
	
	class Position: public kernel::KeyValueFileLoad 
	{
		public:
			Position() {}
	
		public:
			double m_date;
			jblas::vec6 m_mainToBase; 
			
		protected:
			virtual void loadKeyValueFile(jafar::kernel::KeyValueFile const& keyValueFile);
		
	private:
		friend Position HardwareEstimatorOdo::loadPosition(unsigned int index_) const;
		friend void HardwareEstimatorOdo::loadPosition(unsigned int index_, Position& pos) const;
	};

}}}

#endif

