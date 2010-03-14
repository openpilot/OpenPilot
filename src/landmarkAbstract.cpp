/**
 * landmarkAbstract.cpp
 *
 *  Created on: 10/03/2010
 *      Author: jsola
 *
 *  \file landmarkAbstract.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/landmarkAbstract.hpp"

namespace jafar {
	namespace rtslam {

		using namespace std;

		/**
		 * constructor.
		 */
		LandmarkAbstract::LandmarkAbstract(MapAbstract & _map, const jblas::ind_array & _ial) :
			MapObject(_map, _ial), map(&_map) {
			categoryName("LANDMARK");
		}

	}
}
