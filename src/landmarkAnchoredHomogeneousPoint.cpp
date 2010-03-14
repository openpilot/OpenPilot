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
		 * Constructor from map and indirect array
		 */
		Landmark3DAnchoredHomogeneousPoint::Landmark3DAnchoredHomogeneousPoint(MapAbstract & map,
		    const jblas::ind_array & ial) :
			LandmarkAbstract(map, ial) {
			type("AHP");
		}

		namespace landmarkAHP {

		} // namespace ahp
	} // namespace rtslam
} // namespace jafar
