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

		using namespace std;


		/**
		 * Remote constructor from remote map and size of control vector
		 */
		RobotAbstract::RobotAbstract(MapAbstract & _map, const jblas::ind_array & _iar, const size_t _size_control) :
			MapObject(_map, _iar), pose(_map.filter.x, _map.filter.P, jafar::jmath::ublasExtra::ia_head(_iar, 7)), control(
			    _size_control), F_r(_iar.size(), _iar.size()), F_u(_iar.size(), _size_control) {
			// Set robot properties and identifiers
			categoryName("ROBOT"); // robot is categorized
			id(_map.robotIds.getId()); // robot has ID

			// Link robot to map
			_map.addRobot(this); // map has robot
			map = &_map; // robot is in map
		}


		/**
		 * Remote constructor from remote map and size of control vector.
		 */
		RobotAbstract::RobotAbstract(MapAbstract & _map, const size_t _size_state, const size_t _size_control) :
			MapObject(_map, _size_state), pose(_map.filter.x, _map.filter.P, jafar::jmath::ublasExtra::ia_head(state.ia(), 7)),
			    control(_size_control), F_r(_size_state, _size_state), F_u(_size_state, _size_control) {
			// Set robot properties and identifiers
			categoryName("ROBOT"); // robot is categorized
			id(_map.robotIds.getId()); // robot has ID

			// Link robot to map
			_map.addRobot(this); // map has robot
			map = &_map; // robot is in map
		}

	}
}
