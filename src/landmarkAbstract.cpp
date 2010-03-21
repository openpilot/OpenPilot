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
#include "rtslam/observationAbstract.hpp"
#include "rtslam/mapAbstract.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;


		/*
		 * constructor.
		 */
		LandmarkAbstract::LandmarkAbstract(MapAbstract & _map, const size_t _size) :
			MapObject(_map, _size) {
			categoryName("LANDMARK");
		}

		inline void LandmarkAbstract::addObservation(observation_ptr_t _obsPtr) {
			observations[_obsPtr->id()] = _obsPtr;
		}

		void LandmarkAbstract::linkToMap(map_ptr_t _mapPtr){
			slamMap = _mapPtr;
		}




	}
}
