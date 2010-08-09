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
#ifdef HAVE_MTI
		INERTIAL_DATA data;
#endif
		int read_position;
		
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
				while(position == reading_pos || position == read_pos) cond_data.wait(l);
				read_position = position;
				++position; if (position >= bufferSize) position = 0;
				l.unlock();
				f >> row;
				ublas::matrix_row<jblas::mat>(buffer, read_position) = row;
			} else
			{
#ifdef HAVE_MTI
				if (!mti.read(&data)) continue;
				boost::unique_lock<boost::mutex> l(mutex_data);
				if (position == reading_pos) JFR_ERROR(RtslamException, RtslamException::BUFFER_OVERFLOW, "Data not released: Increase MTI buffer size !");
				if (position == read_pos) JFR_ERROR(RtslamException, RtslamException::BUFFER_OVERFLOW, "Data not read: Increase MTI buffer size !");
				read_position = position;
				++position; if (position >= bufferSize) position = 0;
				l.unlock();
				buffer(read_position,0) = data.TIMESTAMP_FILTERED + timestamps_correction;
				buffer(read_position,1) = data.ACC[0];
				buffer(read_position,2) = data.ACC[1];
				buffer(read_position,3) = data.ACC[2];
				buffer(read_position,4) = data.GYR[0];
				buffer(read_position,5) = data.GYR[1];
				buffer(read_position,6) = data.GYR[2];
				buffer(read_position,7) = data.MAG[0];
				buffer(read_position,8) = data.MAG[1];
				buffer(read_position,9) = data.MAG[2];
#endif
			}
			
			if (mode == 1)
			{
				// we put the maximum precision because we want repeatability with the original run
				f << std::setprecision(50) << ublas::matrix_row<jblas::mat>(buffer, read_position) << std::endl;
			}
		}
		
		if (mode == 1 || mode == 2)
			f.close();
	}

	HardwareEstimatorMti::HardwareEstimatorMti(std::string device, double trigger_freq, double trigger_shutter, int bufferSize_, int mode, std::string dump_path, bool start_reading):
#ifdef HAVE_MTI
		mti(device.c_str(), MTI_OPMODE_CALIBRATED, MTI_OPFORMAT_MAT, MTI_SYNCOUTMODE_DISABLED),
#endif
		buffer(bufferSize_, 10), bufferSize(bufferSize_), position(0), reading_pos(-1), read_pos(bufferSize_-1),
		timestamps_correction(0.0)/*, tightly_synchronized(false)*/, mode(mode), dump_path(dump_path)
	{
		if (mode != 2)
		{
#ifdef HAVE_MTI
			INERTIAL_CONFIG config;
			// default syncout pin modes and settings
			config.syncOutMode          = MTI_SYNCOUTMODE_PULSE;
			config.syncOutPulsePolarity = MTI_SYNCOUTPULSE_POS;
			// number of acquisitions to skip before syncOut pin actuates
			
			const double period = 10e-3; // 10ms = 100Hz
			config.syncOutSkipFactor = std::floor(1. / trigger_freq / period + 0.001) - 1; // mark all acquisitions
			realFreq = 1. / (period * (config.syncOutSkipFactor + 1) );
			std::cout << "MTI trigger set to freq " << realFreq << " Hz" << std::endl;
			// number of clock ticks @ 33.9ns to offset pin action from sensor sampling
			config.syncOutOffset = 0; // no offset
			// number of clock ticks @ 33.9ns to define pulse width
			config.syncOutPulseWidth = trigger_shutter/33.9e-9;

			// Set SyncOut settings
			if (!mti.set_syncOut(config.syncOutMode, config.syncOutPulsePolarity,
				config.syncOutSkipFactor, config.syncOutOffset,
				config.syncOutPulseWidth))
				std::cout << "mti.set_syncOut failed" << std::endl;

//if (!mti.set_outputSkipFactor(49))
//std::cout << "mti.set_outputFactor failed" << std::endl;
#endif
		}
		
		if (start_reading)
		{
			for(int i = 0; i < bufferSize; ++i) buffer(i,0) = -1.;
			// start acquire task
			preloadTask_thread = new boost::thread(boost::bind(&HardwareEstimatorMti::preloadTask,this));
			std::cout << "Mti is initializating..." << std::flush;
			sleep(5); // give some time to the time estimator to converge
			std::cout << " done." << std::endl;
		}
	}

	bool HardwareEstimatorMti::setSyncConfig(double timestamps_correction/*, bool tightly_synchronized*/)
	{
		this->timestamps_correction = timestamps_correction;
		//this->tightly_synchronized = tightly_synchronized;
	}

	
	#define DICHOTOMY 1
	jblas::mat_indirect HardwareEstimatorMti::acquireReadings(double t1, double t2)
	{
		JFR_ASSERT(t1 <= t2, "");
		// find indexes
		boost::unique_lock<boost::mutex> l(mutex_data);
		int i1, i2;
		int i, j;
	#if DICHOTOMY
		int i_left = position, i_right = position + bufferSize-1;
		while(i_left != i_right)
		{
			j = (i_left+i_right)/2;
			i = j % bufferSize;
			if (buffer(i,0) >= t1) i_right = j; else i_left = j+1;
		}
		i = i_left % bufferSize;
		i1 = (i-1 + bufferSize) % bufferSize;
		bool no_larger = (buffer(i,0) < t1);
		bool no_smaller = (i == position);
		if (no_larger && buffer(i1,0) < 0.0)  // no data at all
			return ublas::project(buffer, jmath::ublasExtra::ia_set(ublas::range(0,0)), jmath::ublasExtra::ia_set(ublas::range(0, buffer.size2())));
		if (no_smaller) JFR_ERROR(RtslamException, RtslamException::BUFFER_OVERFLOW, "Missing data: increase MTI buffer size !");
		
		if (no_larger)
			i2 = i1;
		else
		{
			i_right = position + bufferSize-1;
			while(i_left != i_right)
			{
				j = (i_left+i_right)/2;
				i = j % bufferSize;
				if (buffer(i,0) >= t2) i_right = j; else i_left = j+1;
			}
			i = i_left % bufferSize;
			i2 = i;
		}
	#else
		for(i = position, j = 0; j < bufferSize; ++j, ++i)
		{
			if (i >= bufferSize) i = 0;
			if (buffer(i,0) >= t1) break;
		}
		i1 = (i == 0 ? bufferSize-1 : i-1);
		if (j == bufferSize && buffer(i1,0) < 0.0)  // no data at all
			return ublas::project(buffer, jmath::ublasExtra::ia_set(ublas::range(0,0)), jmath::ublasExtra::ia_set(ublas::range(0, buffer.size2())));
		if (j == 0) JFR_ERROR(RtslamException, RtslamException::BUFFER_OVERFLOW, "Missing data: increase MTI buffer size !");
		
		if (j == bufferSize)
			i2 = i1;
		else
		{
			int last_position = (position == 0 ? bufferSize-1 : position-1);
			for(; j < bufferSize-1; ++j, ++i)
			{
				if (i >= bufferSize) i = 0;
				if (i == last_position || buffer(i,0) >= t2) break;
			}
			i2 = i;
		}
	#endif
		
		// handle tight sync ; or do nothing, because anyway there's a difference between the Mti pulse, the middle of the exposure of the frame which is the real  date of the frame, and the end of the frame which is supposed to be the provided frame date.
		/*if (tightly_synced)
		{
			i1p1 = (i1+1) % bufferSize;
		}*/
		
		// return mat_indirect
		reading_pos = i1;
		read_pos = i2;
		l.unlock();
		cond_data.notify_all();
//std::cout << "acquireReadings between " << std::setprecision(16) << t1 << " and " << t2 << " ie indexes " << i1 << " and " << i2 << std::endl;

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
