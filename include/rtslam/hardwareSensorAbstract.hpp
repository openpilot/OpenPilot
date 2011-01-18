/**
 * \file hardwareSensorAbstract.hpp
 *
 * Header file for hardware sensors
 *
 * \date 18/06/2010
 * \author croussil
 *
 * \ingroup rtslam
 */

#ifndef HARDWARE_SENSOR_HPP_
#define HARDWARE_SENSOR_HPP_


#include <boost/thread/condition_variable.hpp>


#include "rtslam/rawAbstract.hpp"


namespace jafar {
namespace rtslam {
namespace hardware {

class HardwareSensorAbstract;
typedef boost::shared_ptr<HardwareSensorAbstract> hardware_sensor_ptr_t;

class HardwareSensorAbstract
{
	protected:
		boost::condition_variable &rawdata_condition;
		boost::mutex &rawdata_mutex;
	public:
		HardwareSensorAbstract(boost::condition_variable &rawdata_condition, boost::mutex &rawdata_mutex):
			rawdata_condition(rawdata_condition), rawdata_mutex(rawdata_mutex) {}
		/**
		@param rawPtr the latest raw available from the sensor
		@return the number of missed raws, -1 if no raw is available since last call, -2 if no raw will ever be available
		@note must be non blocking
		*/
		virtual int acquireRaw(raw_ptr_t &rawPtr) = 0;
		virtual ~HardwareSensorAbstract() {}
};

}}}

#endif

