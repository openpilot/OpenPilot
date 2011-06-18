/**
 * \file hardwareEstimatorMti.cpp
 * \date 18/06/2010
 * \author croussil
 * \ingroup rtslam
 */

#include "rtslam/hardwareEstimatorMti.hpp"

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
	{ try {
#ifdef HAVE_MTI
		INERTIAL_DATA data;
#endif
		jblas::vec row(10);
		std::fstream f;
		if (mode == 1 || mode == 2)
		{
			std::ostringstream oss; oss << dump_path << "/MTI.log";
			f.open(oss.str().c_str(), (mode == 1 ? std::ios_base::out : std::ios_base::in));
		}
		
		while (true)
		{
			boost::unique_lock<boost::mutex> l(mutex_data, boost::defer_lock_t());
			if (mode == 2)
			{
				f >> row;
				l.lock();
				if (write_position == read_position) cond_offline.notify_all();
				if (f.eof()) { cond_offline.notify_all(); f.close(); return; }
				while (write_position == read_position) cond_data.wait(l);
// std::cout << "MTI preload: put at write_position " << write_position << " (read_position " << read_position << ") ts " << std::setprecision(16) << row(0) << std::endl;
			} else
			{
#ifdef HAVE_MTI
				if (!mti->read(&data)) continue;
				l.lock();
				if (write_position == read_position) JFR_ERROR(RtslamException, RtslamException::BUFFER_OVERFLOW, "Data not read: Increase MTI buffer size !");
				row(0) = data.TIMESTAMP_FILTERED;
				row(1) = data.ACC[0];
				row(2) = data.ACC[1];
				row(3) = data.ACC[2];
				row(4) = data.GYR[0];
				row(5) = data.GYR[1];
				row(6) = data.GYR[2];
				row(7) = data.MAG[0];
				row(8) = data.MAG[1];
				row(9) = data.MAG[2];
#endif
			}
			ublas::matrix_row<jblas::mat>(buffer, write_position) = row;
			buffer(write_position,0) += timestamps_correction;
			++write_position; if (write_position >= bufferSize) write_position = 0;
			l.unlock();
			
			if (mode == 1)
			{
				// we put the maximum precision because we want repeatability with the original run
				f << std::setprecision(50) << row << std::endl;
			}
		}
		
		if (mode == 1 || mode == 2)
			f.close();
		
	} catch (kernel::Exception &e) { std::cout << e.what(); throw e; } }

	HardwareEstimatorMti::HardwareEstimatorMti(std::string device, double trigger_mode, double trigger_freq, double trigger_shutter, int bufferSize_, int mode, std::string dump_path):
#ifdef HAVE_MTI
		mti(NULL),
#endif
		buffer(bufferSize_, 10), bufferSize(bufferSize_), write_position(0), read_position(bufferSize_-1),
		timestamps_correction(0.0)/*, tightly_synchronized(false)*/, mode(mode), dump_path(dump_path)
	{
		if (mode != 2)
		{
#ifdef HAVE_MTI
			mti = new MTI(device.c_str(), MTI_OPMODE_CALIBRATED, MTI_OPFORMAT_MAT, MTI_SYNCOUTMODE_DISABLED);

			INERTIAL_CONFIG config;
			// default syncout pin modes and settings
			config.syncOutMode          = MTI_SYNCOUTMODE_PULSE;
			config.syncOutPulsePolarity = MTI_SYNCOUTPULSE_POS;
			// number of acquisitions to skip before syncOut pin actuates
			
			if (trigger_mode != 0)
			{
				const double period = 10e-3; // 10ms = 100Hz
				config.syncOutSkipFactor = std::floor(1. / trigger_freq / period + 0.001) - 1; // mark all acquisitions
				realFreq = 1. / (period * (config.syncOutSkipFactor + 1) );
				std::cout << "MTI trigger set to freq " << realFreq << " Hz" << std::endl;
				// number of ns to offset pin action from sensor sampling
				config.syncOutOffset = 0; // no offset
				// number of ns to define pulse width
				if (trigger_shutter < 1e-6 || trigger_mode != 1) trigger_shutter = 0.5e-3;
//				config.syncOutPulseWidth = trigger_shutter/1e-9; // new driver
				config.syncOutPulseWidth = trigger_shutter/33.9e-9; // old driver
	
				// Set SyncOut settings
				if (!mti->set_syncOut(config.syncOutMode, config.syncOutPulsePolarity,
					config.syncOutSkipFactor, config.syncOutOffset,
					config.syncOutPulseWidth))
					std::cout << "mti.set_syncOut failed" << std::endl;
			}
//if (!mti.set_outputSkipFactor(49))
//std::cout << "mti.set_outputFactor failed" << std::endl;
#endif
		} else
		{
			realFreq = trigger_freq;
		}
		
	}
	
	
	HardwareEstimatorMti::~HardwareEstimatorMti()
	{
#ifdef HAVE_MTI
		delete mti;
#endif
	}

	void HardwareEstimatorMti::start()
	{
		for(int i = 0; i < bufferSize; ++i) buffer(i,0) = -1.;
		// start acquire task
		preloadTask_thread = new boost::thread(boost::bind(&HardwareEstimatorMti::preloadTask,this));
		if (mode == 2)
		{ // wait that log has been read before first frame
			boost::unique_lock<boost::mutex> l(mutex_data);
			cond_offline.wait(l);
		}
		std::cout << " done." << std::endl;
	}
	
	void HardwareEstimatorMti::setSyncConfig(double timestamps_correction/*, bool tightly_synchronized, double tight_offset*/)
	{
		this->timestamps_correction = timestamps_correction;
		//this->tightly_synchronized = tightly_synchronized;
		//this->tight_offset = tight_offset;
	}

	
	jblas::mat_indirect HardwareEstimatorMti::acquireReadings(double t1, double t2)
	{
		JFR_ASSERT(t1 <= t2, "");
		boost::unique_lock<boost::mutex> l(mutex_data);
		int i1, i2;
		int i, j;

		// find indexes by dichotomy
		int i_left = write_position, i_right = write_position + bufferSize-1;
		while(i_left != i_right)
		{
			j = (i_left+i_right)/2;
			i = j % bufferSize;
			if (buffer(i,0) >= t1) i_right = j; else i_left = j+1;
		}
		i = i_left % bufferSize;
		i1 = (i-1 + bufferSize) % bufferSize;
		if (t1 <= 1.0 && buffer(i1,0) < 0.0) i1 = i;
		bool no_larger = (buffer(i,0) < t1);
		bool no_smaller = (i == write_position);
		if (no_larger && buffer(i1,0) < 0.0)  // no data at all
			return ublas::project(buffer, jmath::ublasExtra::ia_set(ublas::range(0,0)), jmath::ublasExtra::ia_set(ublas::range(0, buffer.size2())));
		if (no_smaller) JFR_ERROR(RtslamException, RtslamException::BUFFER_OVERFLOW, "Missing data: increase MTI buffer size !");
		
		if (no_larger)
			i2 = i1;
		else
		{
			i_right = write_position + bufferSize-1;
			while(i_left != i_right)
			{
				j = (i_left+i_right)/2;
				i = j % bufferSize;
				if (buffer(i,0) >= t2) i_right = j; else i_left = j+1;
			}
			i = i_left % bufferSize;
			i2 = i;
		}
		
		// handle tight sync ; or do nothing, because anyway there's a difference between the Mti pulse, the middle of the exposure of the frame which is the real  date of the frame, and the end of the frame which is supposed to be the provided frame date, and it is difficult to decide where to do the modification...
		/*if (tightly_synced)
		{
			i1p1 = (i1+1) % bufferSize;
		}*/
		
		// return mat_indirect
		read_position = i1;
// std::cout << "MTI acquire: read_position " << read_position << " (write_position " << write_position << ")" << std::endl;
		l.unlock();
		cond_data.notify_all();
// std::cout << "acquireReadings between " << std::setprecision(18) << t1 << " and " << t2 << ", ie indexes " << i1 << " and " << i2 << " with t(i1) " << buffer(i1, 0) << " and t(i2) " << buffer(i2, 0) << std::endl;

		if (i1 < i2)
		{
			return ublas::project(buffer, 
				jmath::ublasExtra::ia_set(ublas::range(i1,i2+1)),
				jmath::ublasExtra::ia_set(ublas::range(0,buffer.size2())));
		} else
		{
			return ublas::project(buffer, 
				jmath::ublasExtra::ia_concat(jmath::ublasExtra::ia_set(ublas::range(i1,buffer.size1())),
				                            jmath::ublasExtra::ia_set(ublas::range(0,i2+1))),
				jmath::ublasExtra::ia_set(ublas::range(0,buffer.size2())));
		}
	}


}}}

