/**
 * landmarkAnchoredHomogeneousPoint.cpp
 *
 * \date 08/03/2010
 * \author jsola@laas.fr
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

		vec3 LandmarkAnchoredHomogeneousPoint::toEuclidean() {
			return lmkAHP::ahp2euc(state.x());
		}



	} // namespace rtslam
} // namespace jafar
