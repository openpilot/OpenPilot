/**
 * robotAbstract.cpp
 *
 *  Created on: 08/03/2010
 *      Author: jsola
 *
 *  \file robotAbstract.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/robotAbstract.hpp"

namespace jafar {
	namespace rtslam {

		void RobotAbstract::installSensor(SensorAbstract & sen) {
			sensorsList.push_back(&sen);
			sen.robot = this;
		}

		size_t RobotAbstract::getNextId(void) {
			static size_t ID;
			return ++ID;
		}

	}
}
