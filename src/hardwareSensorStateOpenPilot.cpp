/**
 * \file hardwareSensorStateOpenPilot.cpp
 *
 * File for getting data from openpilot (INS+GPS state information)
 *
 * \date 10/10/2012
 * \author corvuscorax
 *
 * \ingroup rtslam
 */

#include "kernel/timingTools.hpp"
#include "rtslam/hardwareSensorStateOpenPilot.hpp"


namespace jafar {
namespace rtslam {
namespace hardware {


	void HardwareSensorStateOpenPilot::capture(OpenPilotStateInformation *state)
	{ try {

		if (!started) return;
		reading.arrival = kernel::Clock::getTime();
		reading.data(0) = reading.arrival;
		// position
		reading.data(1) = state->position.x;
		reading.data(2) = state->position.y;
		reading.data(3) = state->position.z;
		reading.data(4) = state->attitude.roll;
		reading.data(5) = state->attitude.pitch;
		reading.data(6) = state->attitude.yaw;
		reading.data(7) = state->positionVariance;
		reading.data(8) = state->positionVariance;
		reading.data(9) = state->positionVariance;
		reading.data(10) = state->attitudeVariance;
		reading.data(11) = state->attitudeVariance;
		reading.data(12) = state->attitudeVariance;
	
		int buff_write = getWritePos();
		buffer(buff_write).data = reading.data;
		last_timestamp = reading.arrival;
		incWritePos();

		if (mode == 1)
		{
			// we put the maximum precision because we want repeatability with the original run
			f << std::setprecision(50) << reading.data << std::endl;
		}

		
	} catch (kernel::Exception &e) { std::cout << e.what(); throw e; } }
	
	
	HardwareSensorStateOpenPilot::HardwareSensorStateOpenPilot(kernel::VariableCondition<int> &condition, unsigned bufferSize, int mode, std::string dump_path):
		HardwareSensorProprioAbstract(condition, bufferSize, false), mode(mode), dump_path(dump_path)
	{
		addQuantity(qPos);
		addQuantity(qOriEuler);
		reading.resize(readingSize());
		// configure
	}

	void HardwareSensorStateOpenPilot::start()
	{
		// start acquire task
		//preloadTask();
		if (started) { std::cout << "Warning: This HardwareSensorStateOpenPilot has already been started" << std::endl; return; }
		started = true;
		last_timestamp = kernel::Clock::getTime();
		
		/*if (mode == 2)
		{ // wait that log has been read before first frame
			boost::unique_lock<boost::mutex> l(mutex_data);
			cond_offline_full.wait(l);
		}*/
		//if (mode == 1 || mode == 2)
		// TODO: make a traditional preload/read thread to read in dumped state data
		// (so far unneeded since openpilot uses its own .opl logs for state information to be replayed)
		if (mode == 1)
		{
			std::ostringstream oss; oss << dump_path << "/OpenPilotState.log";
			f.open(oss.str().c_str(), (mode == 1 ? std::ios_base::out : std::ios_base::in));
		}
		
	}
	
}}}

