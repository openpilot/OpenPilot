/**
 * mapObject.cpp
 *
 *  Created on: 13/03/2010
 *      Author: jsola
 *
 *  \file mapObject.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/mapObject.hpp"

namespace jafar {
	namespace rtslam {

		/**
		 * Local constructor from size
		 */
		MapObject::MapObject(std::size_t _size) :
			state(_size) {
		}

		/**
		 * Remote constructor from remote map and indirect array
		 * \param _map the remote map
		 * \param _iar the indirect array pointing to the remote storage
		 */
		MapObject::MapObject(MapAbstract & _map, const jblas::ind_array & _iar) :
			map(&_map), state(_map.filter.x, _map.filter.P, _iar) {
			categoryName_ = "OBJECT";
		}

	}
}
