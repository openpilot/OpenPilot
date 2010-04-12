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

#include "boost/shared_ptr.hpp"
#include "rtslam/pinhole.hpp"

#include "rtslam/observationPinHoleAnchoredHomogeneous.hpp"

namespace jafar {
	namespace rtslam {

		using namespace std;
		using namespace jblas;
		using namespace ublas;


		ObservationPinHoleAnchoredHomogeneousPoint::ObservationPinHoleAnchoredHomogeneousPoint(const sensor_ptr_t & pinholePtr,
				const landmark_ptr_t & ahpPtr) :

			ObservationAbstract(pinholePtr, ahpPtr, 2, pinholePtr->robotPtr->pose.size() + pinholePtr->state.size(), 1) {
			categoryName("PINHOLE-AHP OBS");
			link(pinholePtr, ahpPtr);
		}

		void ObservationPinHoleAnchoredHomogeneousPoint::link(const sensor_ptr_t & _senPtr, const landmark_ptr_t & _lmkPtr) { ///< Link to sensor and landmark
			ObservationAbstract::link(_senPtr, _lmkPtr);
			// Use this pointer below to access the pin-hole specific parameters.
			pinHolePtr = boost::dynamic_pointer_cast<SensorPinHole>(sensorPtr);
			ahpPtr = boost::dynamic_pointer_cast<LandmarkAnchoredHomogeneousPoint>(landmarkPtr);
		}

		void ObservationPinHoleAnchoredHomogeneousPoint::project_func() {
			vec7 sg; // Some temps of known size
			vec3 v;
			vec2 u;
			double invDist;
			mat V_sg(3, 7);
			mat V_ahp(3, 7);
			mat23 U_v;

			size_t size_rsl = ia_rsl.size(); //       Sizes of Jacobians depend on the sensor being LOCAL or REMOTE.
			size_t size_l = landmarkPtr->size(); //   But this is already known since we have ia_rsl with the same size.
			size_t size_rs = size_rsl - size_l;

			mat SG_rs(sensorPtr->pose.size(), size_rs); // This temp was of variable size

			sensorPtr->globalPose(sg, SG_rs); //      Pose of sensor in map, Jac. wrt. robot and sensor
			LandmarkAnchoredHomogeneousPoint::toBearingOnlyFrame(sg, landmarkPtr->state.x(), v, invDist, V_sg, V_ahp); // lmk in sensor frame
			// This function below uses the down-casted pointer because it needs to know the sensor parameters.
			pinHolePtr->projectPoint(v, u, U_v); //   Project lmk, get expected pixel u and Jacobians

			mat V_rs = prod(V_sg, SG_rs); //          The chain rule !
			mat U_rs = prod(U_v, V_rs);
			mat U_ahp = prod(U_v, V_ahp);

			expectation.x(u); //                      Output: assign expectation mean and Jacobians
			ublas::subrange(EXP_rsl, 0, 2, 0, size_rs) = U_rs;
			ublas::subrange(EXP_rsl, 0, 2, size_rs, size_rsl) = U_ahp;
			expectation.nonObs(0) = invDist; //       Assign non-observable part (inverse-distance)
		}

		void ObservationPinHoleAnchoredHomogeneousPoint::backProject_func() {
			// \todo implement back-projection
		}

		bool ObservationPinHoleAnchoredHomogeneousPoint::predictVisibility(){
			bool inimg = pinhole::isInImage(expectation.x(), pinHolePtr->imgSize(0), pinHolePtr->imgSize(1));
			bool infront = (expectation.nonObs(0) > 0.0);
			events.visible = inimg && infront;
			return events.visible;
		}

	}
}

