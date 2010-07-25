/**
 * \file hardwareEstimatorMti.cpp
 *
 * \author croussil@laas.fr
 * \date 18/06/2010
 *
 * \ingroup rtslam
 */

#include "rtslam/hardwareEstimatorMti.hpp"

#ifdef HAVE_MTI

#include <sys/time.h>
#include <boost/bind.hpp>

#include "kernel/jafarMacro.hpp"
#include "jmath/misc.hpp"
#include "jmath/indirectArray.hpp"

#include "rtslam/rtslamException.hpp"

namespace jafar {
namespace rtslam {
namespace hardware {

	void HardwareEstimatorMti::preloadTask(void)
	{
		INERTIAL_DATA data;
		struct timeval tv;
		struct timezone tz;
		
		jblas::vec row(10);
		std::fstream f;
		if (mode == 1 || mode == 2)
		{
			std::ostringstream oss; oss << dump_path << "/MTI.log";
			f.open(oss.str().c_str(), (mode == 1 ? std::ios_base::out : std::ios_base::in));
		}
		
		while (true)
		{
			if (mode == 2)
			{
				boost::unique_lock<boost::mutex> l(mutex_data);
				if (position == reading_pos) { boost::this_thread::yield(); continue; }
				if (position == read_pos) { boost::this_thread::yield(); continue; }
				l.unlock();
				f >> row;
				ublas::matrix_row<jblas::mat>(buffer, position) = row;
			} else
			{
				if (!mti.read(&data)) continue;
				gettimeofday(&tv, &tz);
				// TODO compute delay between now and the real date of the reading
				// TODO implement a precise timestamp in viam-libs
				boost::unique_lock<boost::mutex> l(mutex_data);
				if (position == reading_pos) JFR_ERROR(RtslamException, RtslamException::BUFFER_OVERFLOW, "Data not released: Increase MTI buffer size !");
				if (position == read_pos) JFR_ERROR(RtslamException, RtslamException::BUFFER_OVERFLOW, "Data not read: Increase MTI buffer size !");
				l.unlock();
				buffer(position,0) = tv.tv_sec + tv.tv_usec*1e-6;
				buffer(position,1) = data.ACC[0];
				buffer(position,2) = data.ACC[1];
				buffer(position,3) = data.ACC[2];
				buffer(position,4) = data.GYR[0];
				buffer(position,5) = data.GYR[1];
				buffer(position,6) = data.GYR[2];
				buffer(position,7) = data.MAG[0];
				buffer(position,8) = data.MAG[1];
				buffer(position,9) = data.MAG[2];
			}
			
			if (mode == 1)
			{
				
				f << ublas::matrix_row<jblas::mat>(buffer, position);
			}
			
			++position;
			if (position >= bufferSize) position = 0;
		}
		
		if (mode == 1 || mode == 2)
			f.close();
	}

	HardwareEstimatorMti::HardwareEstimatorMti(std::string device, double freq, double shutter, int bufferSize_, int mode, std::string dump_path):
		mti(device.c_str(), MTI_OPMODE_BOTH, MTI_OPFORMAT_MAT),
		buffer(bufferSize_, 10), bufferSize(bufferSize_), position(0), reading_pos(-1), read_pos(bufferSize_-1), mode(mode), dump_path(dump_path)
	{
		INERTIAL_CONFIG config;
		// default syncout pin modes and settings
		config.syncOutMode          = MTI_SYNCOUTMODE_PULSE;
		config.syncOutPulsePolarity = MTI_SYNCOUTPULSE_POS;
		// number of acquisitions to skip before syncOut pin actuates
		
		const double period = 10e-3; // 10ms = 100Hz
		config.syncOutSkipFactor = std::floor(1. / freq / period + 0.001) - 1; // mark all acquisitions
		realFreq = 1. / (period * (config.syncOutSkipFactor + 1) );
		std::cout << "MTI trigger set to freq " << realFreq << " Hz" << std::endl;
		// number of clock ticks @ 33.9ns to offset pin action from sensor sampling
		config.syncOutOffset = 0; // no offset
		// number of clock ticks @ 33.9ns to define pulse width
		config.syncOutPulseWidth = shutter/33.9e-9;

		// Set SyncOut settings
		if (!mti.set_syncOut(config.syncOutMode, config.syncOutPulsePolarity,
			config.syncOutSkipFactor, config.syncOutOffset,
			config.syncOutPulseWidth))
			std::cout << "mti.set_syncOut failed" << std::endl;
		for(int i = 0; i < bufferSize; ++i) buffer(i,0) = 0.;
		// start acquire task
		preloadTask_thread = new boost::thread(boost::bind(&HardwareEstimatorMti::preloadTask,this));
	}
	
	
	jblas::mat_indirect HardwareEstimatorMti::acquireReadings(double t1, double t2)
	{
		JFR_ASSERT(t1 <= t2, "");
		// find indexes
		boost::unique_lock<boost::mutex> l(mutex_data);
		int i1, i2, n;
		int i, j;
		for(i = position, j = 0; j < bufferSize; ++j, ++i)
		{
			if (i >= bufferSize) i = 0;
			if (buffer(i,0) >= t1) break;
		}
		i1 = (i == 0 ? bufferSize-1 : i-1);
		if (j == bufferSize && buffer(i1,0) < 1.0)  // no data at all
			return ublas::project(buffer, jmath::ublasExtra::ia_set(ublas::range(0,0)), jmath::ublasExtra::ia_set(ublas::range(0, buffer.size2())));
		if (j == 0) JFR_ERROR(RtslamException, RtslamException::BUFFER_OVERFLOW, "Missing data: increase MTI buffer size !");
			
		for(n = 0; j < bufferSize; ++j, ++i, ++n)
		{
			if (i >= bufferSize) i = 0;
			if (buffer(i,0) >= t2) break;
		}
		i2 = i;
		
		// return mat_indirect
		reading_pos = i1;
		read_pos = i2;
		if (i1 < i2)
		{
			return ublas::project(buffer, 
				jmath::ublasExtra::ia_set(ublas::range(i1,i2)),
				jmath::ublasExtra::ia_set(ublas::range(0,buffer.size2())));
		} else
		{
			return ublas::project(buffer, 
				jmath::ublasExtra::ia_union(jmath::ublasExtra::ia_set(ublas::range(i1,buffer.size1())),
				                            jmath::ublasExtra::ia_set(ublas::range(0,i2))),
				jmath::ublasExtra::ia_set(ublas::range(0,buffer.size2())));
		}
	}


}}}

#endif // #ifdef HAVE_MTI
