/**
 * \file hardwareSensorAbstract.hpp
 *
 * Header file for hardware sensors
 *
 * \author croussil@laas.fr
 * \date 18/06/2010
 *
 * \ingroup rtslam
 */

#ifndef HARDWARE_SENSOR_HPP_
#define HARDWARE_SENSOR_HPP_




#include "rtslam/rawAbstract.hpp"


namespace jafar {
namespace rtslam {



class HardwareSensorAbstract
{

	public:
		/**
		@param rawPtr the latest raw available from the sensor
		@return the number of missed raws, -1 if no raw is available since last call
		@note must be non blocking
		*/
		virtual int getRaw(raw_ptr_t &rawPtr) = 0;
		virtual void releaseRaw() = 0;
		virtual ~HardwareSensorAbstract() {}
};

}}

#endif

