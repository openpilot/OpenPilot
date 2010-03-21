/**
 * observationPinHoleAnchoredHomogeneous.cpp
 *
 *  Created on: 14/03/2010
 *      Author: jsola
 *
 *  \file observationPinHoleAnchoredHomogeneous.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/observationPinHoleAnchoredHomogeneous.hpp"

namespace jafar {
	namespace rtslam {

		using namespace std;
		using namespace jblas;

		ObservationPinHoleAnchoredHomogeneousPoint::ObservationPinHoleAnchoredHomogeneousPoint() :
			ObservationPinHolePoint() {
			categoryName("PINHOLE-AHP OBS");
		}

		void ObservationPinHoleAnchoredHomogeneousPoint::convertToDir() {
			jblas::vec7 ahp = landmark->state.x();
			vec3 p0;
			vec3 m;
			double rho;
			landmarkAHP::split(ahp, p0, m, rho);
			vec7 S; // sensor global pose
			S = quaternion::composeFrames(sensor->robot->pose.x(), sensor->pose.x());
			vec3 T = ublas::project(S, ublas::range(0, 3));
			vec4 Q = ublas::project(S, ublas::range(3, 7));
			jblas::mat33 Rt = quaternion::q2Rt(Q);
			dirVec = ublas::prod(Rt, (m - (T - p0) * rho)); // See Sola, ICRA 2010.
		}

	}
}

