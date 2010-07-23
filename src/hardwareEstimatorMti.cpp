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

#include "jmath/misc.hpp"

namespace jafar {
namespace rtslam {
namespace hardware {

	void HardwareEstimatorMti::preloadTask(void)
	{
		INERTIAL_DATA data;
		struct timeval tv;
		struct timezone tz;
		
		while (true)
		{
			if (!mti.read(&data)) continue;
			gettimeofday(&tv, &tz);
			// TODO compute delay between now and the real date of the reading
			// TODO implement a precise timestamp in viam-libs
			boost::unique_lock<boost::mutex> l(mutex_data);
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
			++position;
			if (position >= bufferSize) position = 0;
			l.unlock();
		}
	}

	HardwareEstimatorMti::HardwareEstimatorMti(std::string device, double freq, double shutter, int bufferSize_):
		mti(device.c_str(), MTI_OPMODE_BOTH, MTI_OPFORMAT_EULER),
		buffer(bufferSize_, 10), published(bufferSize_, 10), bufferSize(bufferSize_), position(0)
	{
		INERTIAL_CONFIG config;
		// default syncout pin modes and settings
		config.syncOutMode          = MTI_SYNCOUTMODE_PULSE;
		config.syncOutPulsePolarity = MTI_SYNCOUTPULSE_POS;
		// number of acquisitions to skip before syncOut pin actuates
		
		const double period = 10e-3; // 10ms = 100Hz
		config.syncOutSkipFactor = jmath::round(1. / freq / period) - 1; // mark all acquisitions
		double realfreq = 1. / (period * (config.syncOutSkipFactor + 1) );
		std::cout << "MTI trigger set to freq " << realfreq << " Hz" << std::endl;
		// number of clock ticks @ 33.9ns to offset pin action from sensor sampling
		config.syncOutOffset = 0; // no offset
		// number of clock ticks @ 33.9ns to define pulse width
		config.syncOutPulseWidth = shutter/33.9e-9;
		// Set SyncOut settings
		mti.set_syncOut(config.syncOutMode, config.syncOutPulsePolarity,
			config.syncOutSkipFactor, config.syncOutOffset,
			config.syncOutPulseWidth);
	
		for(int i = 0; i < bufferSize; ++i) buffer(i,0) = 0.;
		// start acquire task
		preloadTask_thread = new boost::thread(boost::bind(&HardwareEstimatorMti::preloadTask,this));
	}
	
	
	jblas::mat_range HardwareEstimatorMti::acquireReadings(double t1, double t2)
	{
		// find indexes
		boost::unique_lock<boost::mutex> l(mutex_data);
		int i1, i2, n;
		int i, j;
		for(i = position+1, j = 0; j < bufferSize; ++j, ++i)
		{
			if (i >= bufferSize) i = 0;
			i1 = i;
			if (buffer(i,0) >= t1) break;
		}
		i2 = i; n = 0;
		for(; j < bufferSize; ++j, ++i, ++n)
		{
			if (i >= bufferSize) i = 0;
			i2 = i;
			if (buffer(i,0) >= t2) break;
		}
			
		// copy from buffer to published
		if (n == 0) {}
		else if (i1 < i2)
		{
			ublas::project(published, ublas::range(0,n), ublas::range(0,published.size2())) = 
				ublas::project(buffer, ublas::range(i1,i2), ublas::range(0,published.size2()));
		} else
		{
			ublas::project(published, ublas::range(0,published.size1()-i1), ublas::range(0,published.size2())) = 
				ublas::project(buffer, ublas::range(i1,published.size1()), ublas::range(0,published.size2()));
			ublas::project(published, ublas::range(published.size1()-i1,n), ublas::range(0,published.size2())) = 
				ublas::project(buffer, ublas::range(0,i2), ublas::range(0,published.size2()));
		}
		return jblas::mat_range(published, ublas::range(0,n), ublas::range(0,published.size2()));
	}


}}}

#endif // #ifdef HAVE_MTI
