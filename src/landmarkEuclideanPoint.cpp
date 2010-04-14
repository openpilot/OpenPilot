/*
 * landmarkEuclideanPoint.cpp
 *
 *  Created on: Apr 14, 2010
 *      Author: agonzale
 */

#include "rtslam/landmarkEuclideanPoint.hpp"

namespace jafar {
	namespace rtslam {

		/**
		 * Constructor from map
		 */
		LandmarkEuclideanPoint::LandmarkEuclideanPoint(const map_ptr_t & mapPtr) :
			LandmarkAbstract(mapPtr, 3) {
			type("EUC");
		}


	} // namespace rtslam
} // namespace jafar
