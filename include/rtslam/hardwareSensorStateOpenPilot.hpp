/**
 * \file hardwareSensorStateOpenPilot.hpp
 *
 * Header file for getting data from openpilot (INS+GPS state information)
 *
 * \date 10/10/2012
 * \author corvuscorax
 *
 * \ingroup rtslam
 */

#ifndef HARDWARE_SENSOR_STATEOPENPILOT_HPP_
#define HARDWARE_SENSOR_STATEOPENPILOT_HPP_

#include <stdlib.h>
#include <unistd.h>

#include <jafarConfig.h>
#include "kernel/jafarMacro.hpp"
#include "rtslam/hardwareSensorAbstract.hpp"

namespace jafar {
namespace rtslam {
namespace hardware {

class OpenPilotStateInformation
{
	public:
	struct {
		double x;
		double y;
		double z;
	} position;
	struct {
		double roll;
		double pitch;
		double yaw;
	} attitude;
	double positionVariance;
	double attitudeVariance;
};

class HardwareSensorStateOpenPilot: public HardwareSensorProprioAbstract
{
	private:
		
		RawVec reading;
		int mode;
		std::string dump_path;
		std::fstream f;
		double last_timestamp;
		
	public:
		HardwareSensorStateOpenPilot(kernel::VariableCondition<int> &condition, unsigned bufferSize, int mode = 0, std::string dump_path = ".");
		
		virtual void start();
		virtual double getLastTimestamp() { boost::unique_lock<boost::mutex> l(mutex_data); return last_timestamp; }
		void capture(OpenPilotStateInformation * state);
		
};


}}}

#endif // HARDWARE_SENSOR_STATEOPENPILOT_HPP_
