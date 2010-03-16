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
#include "jmath/indirectArray.hpp"

namespace jafar {
	namespace rtslam {


		/**
		 * Local constructor from size
		 */
		MapObject::MapObject(const std::size_t _size) :
			state(_size) {
			categoryName_ = "OBJECT";
		}


		/**
		 * Remote constructor from remote map and indirect array
		 */
		MapObject::MapObject(MapAbstract & _map, const size_t _size) :
			map(&_map),
			state(_map.filter.x, _map.filter.P, _map.reserveStates(_size)) {
			categoryName_ = "OBJECT";
		}

	}
}
