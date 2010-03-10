/**
 * sensorAbstract.cpp
 *
 *  Created on: 10/03/2010
 *      Author: jsola
 *
 *  \file sensorAbstract.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/sensorAbstract.hpp"

namespace jafar{
	namespace rtslam {
		using namespace std;

		/*
		 * Local pose constructor - only mean
		 */
		//				template<class V>
		SensorAbstract::SensorAbstract(const jblas::vec & _pose) :
			pose(_pose) {
		}

		/*
		 * Local pose constructor - full Gaussian.
		 */
		SensorAbstract::SensorAbstract(const Gaussian & _pose) :
			pose(_pose) {
		}

		/*
		 * Remote pose constructor.
		 */
		SensorAbstract::SensorAbstract(MapAbstract & map, const jblas::ind_array & ias) :
			pose(map.filter.x, map.filter.P, ias) {
		}


		void SensorAbstract::installToRobot(RobotAbstract & rob) {
			rob.sensorsList.push_back(this);
			robot = &rob;
		}


	}
}
