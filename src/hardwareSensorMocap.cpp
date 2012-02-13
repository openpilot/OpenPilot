/**
 * \file hardwareSensorGpsGenom.cpp
 *
 * File for getting data from the genom gps module
 *
 * \date 16/03/2011
 * \author croussil
 *
 * \ingroup rtslam
 */

#include "kernel/timingTools.hpp"
#include "rtslam/hardwareSensorMocap.hpp"


namespace jafar {
namespace rtslam {
namespace hardware {


	void HardwareSensorMocap::preloadTask(void)
	{ try {

		std::fstream f;
		if (mode == 1 || mode == 2)
		{
			std::ostringstream oss; oss << dump_path << "/mocap.log";
			f.open(oss.str().c_str(), (mode == 1 ? std::ios_base::out : std::ios_base::in));
		}
		
		while (true)
		{
			if (mode == 2)
			{
				f >> reading.data;
				boost::unique_lock<boost::mutex> l(mutex_data);
				if (isFull(true)) cond_offline_full.notify_all();
				if (f.eof()) { no_more_data = true; cond_offline_full.notify_all(); f.close(); return; }
				while (isFull(true)) cond_offline_freed.wait(l);
			} else
			{
				// TODO read data from sensor and put it in reading
				reading.arrival = kernel::Clock::getTime();
			}
			
			int buff_write = getWritePos();
			buffer(buff_write).data = reading.data;
			buffer(buff_write).data(0) += timestamps_correction;
			last_timestamp = reading.data(0);
			incWritePos();
			
			if (mode == 1)
			{
				// we put the maximum precision because we want repeatability with the original run
				f << std::setprecision(50) << reading.data << std::endl;
			}
			
		}
	} catch (kernel::Exception &e) { std::cout << e.what(); throw e; } }
	
	
	HardwareSensorMocap::HardwareSensorMocap(kernel::VariableCondition<int> &condition, unsigned bufferSize, int mode, std::string dump_path):
		HardwareSensorProprioAbstract(condition, bufferSize), reading(13), mode(mode), dump_path(dump_path)
	{
		addQuantity(qPos, 1, 3);
		addQuantity(qOriEuler, 4, 3); // using euler x/y/z because the sensors work with euler and the uncertainty is provided with euler)
		// configure
		if (mode == 0 || mode == 1)
		{
		}
	}

	void HardwareSensorMocap::start()
	{
		// start acquire task
		//preloadTask();
		if (started) { std::cout << "Warning: This HardwareSensorMocap has already been started" << std::endl; return; }
		started = true;
		last_timestamp = kernel::Clock::getTime();
		preloadTask_thread = new boost::thread(boost::bind(&HardwareSensorMocap::preloadTask,this));
		
		if (mode == 2)
		{ // wait that log has been read before first frame
			boost::unique_lock<boost::mutex> l(mutex_data);
			cond_offline_full.wait(l);
		}
	}
	
}}}

