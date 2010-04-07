/**
 * landmarkAnchoredHomogeneousPoint.cpp
 *
 *  Created on: 08/03/2010
 *      Author: jsola
 *
 *  \file landmarkAnchoredHomogeneousPoint.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/landmarkAnchoredHomogeneousPoint.hpp"

namespace jafar {
	namespace rtslam {


		/**
		 * Constructor from map
		 */
		LandmarkAnchoredHomogeneousPoint::LandmarkAnchoredHomogeneousPoint(const map_ptr_t & mapPtr) :
			LandmarkAbstract(mapPtr, 7) {
			type("AHP");
		}


	} // namespace rtslam
} // namespace jafar
