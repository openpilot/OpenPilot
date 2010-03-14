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

namespace jafar {
	namespace rtslam {
		using namespace std;

		/*
		 * Local pose constructor - only mean
		 */
		//				template<class V>
		SensorAbstract::SensorAbstract(const jblas::vec & _pose) :
			MapObject(0),
			pose(_pose) {
			categoryName("SENSOR");
		}

		/*
		 * Local pose constructor - full Gaussian.
		 */
		SensorAbstract::SensorAbstract(const Gaussian & _pose) :
			MapObject(0),
			pose(_pose) {
			categoryName("SENSOR");
		}

		/*
		 * Remote pose constructor.
		 */
		SensorAbstract::SensorAbstract(MapAbstract & _map, const jblas::ind_array & _ias) :
			MapObject(_map, _ias), pose(_map.filter.x, _map.filter.P, jafar::jmath::ublasExtra::ia_head(_ias, 7)) {
			categoryName("SENSOR");
		}

		void SensorAbstract::installToRobot(RobotAbstract & rob) {
			rob.sensorsList.push_back(this);
			robot = &rob;
		}


	}
}
