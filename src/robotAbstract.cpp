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
#include "rtslam/sensorAbstract.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;


		/**
		 * Remote constructor from remote map and size of control vector.
		 */
		RobotAbstract::RobotAbstract(MapAbstract & _map, const size_t _size_state, const size_t _size_control) :
			MapObject(_map, _size_state),
			pose(_map.filter.x, _map.filter.P, jafar::jmath::ublasExtra::ia_head(state.ia(), 7)),
			control(_size_control),
			F_r(_size_state, _size_state),
			F_u(_size_state, _size_control) {

			// Set robot properties and identifiers
			categoryName("ROBOT"); // robot is categorized
			id(_map.robotIds.getId()); // robot has ID

			// Link robot to map
			_map.addRobot(this); // map has robot
			slamMap = &_map; // robot is in map
		}


		/**
		 * Add a sensor to this robot
		 */
		void RobotAbstract::addSensor(SensorAbstract * _senPtr){
			sensors[_senPtr->id()] = _senPtr;
		}




	}
}
