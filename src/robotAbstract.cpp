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

		/**
		 * Sizes constructor
		 */
		RobotAbstract::RobotAbstract(size_t size_state, size_t size_control) :
			//					state(size_state), control(size_control), F_r(size_state, size_state), F_u(size_state, size_control) {
			    control(size_control), F_r(size_state, size_state), F_u(size_state, size_control) {
		}

		/**
		 * Remote constructor from remote map and size of control vector
		 */
		RobotAbstract::RobotAbstract(MapAbstract & _map, jblas::ind_array & _iar, size_t _size_control) :
			state(_map.filter.x, _map.filter.P, _iar), pose(_map.filter.x, _map.filter.P,
			    jafar::jmath::ublasExtra::ia_head(_iar, 7)), control(_size_control), F_r(_iar.size(), _iar.size()), F_u(
			    _iar.size(), _size_control) {
		}


		size_t RobotAbstract::getNextId(void) {
			static size_t ID;
			return ++ID;
		}

	}
}
