/**
 * \file observationPinHoleAnchoredHomogeneous.hpp
 *
 *  Created on: 14/02/2010
 *      Author: jsola
 *
 * \ingroup rtslam
 */

#ifndef OBSERVATIONPINHOLEANCHOREDHOMOGENEOUS_HPP_
#define OBSERVATIONPINHOLEANCHOREDHOMOGENEOUS_HPP_

#include "rtslam/observationPinHolePoint.hpp"
#include "rtslam/landmarkAnchoredHomogeneousPoint.hpp"

namespace jafar {
	namespace rtslam {


		/**
		 * Class for Pin-Hole observations of Anchored Homogeneous 3D points.
		 * \author jsola
		 * \ingroup rtslam
		 */
		class ObservationPinHoleAnchoredHomogeneousPoint: public ObservationPinHolePoint {
			public:

				ObservationPinHoleAnchoredHomogeneousPoint();

				void convertToDir();
				void project();
				void splitAHP(vec3 & p0, vec3 & m, double rho);

		};

	}
}

#endif /* OBSERVATIONPINHOLEANCHOREDHOMOGENEOUS_HPP_ */
