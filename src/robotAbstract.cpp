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
		 * Remote constructor from remote map and size of control vector
		 */
		RobotAbstract::RobotAbstract(MapAbstract & _map, jblas::ind_array & _iar, size_t _size_control) :
			MapObject(_map, _iar),
			//			state(_map.filter.x, _map.filter.P, _iar),
			pose(_map.filter.x, _map.filter.P, jafar::jmath::ublasExtra::ia_head(_iar, 7)),
			control(_size_control),
			F_r(_iar.size(), _iar.size()),
			F_u(_iar.size(), _size_control) {
			categoryName("ROBOT");
		}

	}
}
