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
		LandmarkAnchoredHomogeneousPoint::LandmarkAnchoredHomogeneousPoint(MapAbstract & map) :
			LandmarkAbstract(map, 7) {
			type("AHP");
		}


	} // namespace rtslam
} // namespace jafar
