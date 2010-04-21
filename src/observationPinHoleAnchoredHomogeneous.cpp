/**
 * observationPinHoleAnchoredHomogeneous.cpp
 *
 * \date 14/03/2010
 * \author jsola@laas.fr
 *
 *  \file observationPinHoleAnchoredHomogeneous.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "boost/shared_ptr.hpp"
#include "rtslam/pinholeTools.hpp"
#include "rtslam/ahpTools.hpp"
#include "rtslam/landmarkAnchoredHomogeneousPoint.hpp"
#include "rtslam/observationPinHoleAnchoredHomogeneous.hpp"

namespace jafar {
	namespace rtslam {

		using namespace std;
		using namespace jblas;
		using namespace ublas;

		ObservationPinHoleAnchoredHomogeneousPoint::ObservationPinHoleAnchoredHomogeneousPoint(
		    const sensor_ptr_t & pinholePtr, const landmark_ptr_t & ahpPtr) :

			ObservationAbstract(pinholePtr, ahpPtr, 2, pinholePtr->robotPtr->pose.size() + pinholePtr->state.size(), 1)
		{
			categoryName("PINHOLE-AHP OBS");
			link(pinholePtr, ahpPtr);
		}

		void ObservationPinHoleAnchoredHomogeneousPoint::link(const sensor_ptr_t & _senPtr, const landmark_ptr_t & _lmkPtr)
		{ ///< Link to sensor and landmark
			ObservationAbstract::link(_senPtr, _lmkPtr);
			// Use this pointer below to access the pin-hole specific parameters.
			pinHolePtr = boost::dynamic_pointer_cast<SensorPinHole>(sensorPtr);
			ahpPtr = boost::dynamic_pointer_cast<LandmarkAnchoredHomogeneousPoint>(landmarkPtr);
		}

		void ObservationPinHoleAnchoredHomogeneousPoint::project_func(const vec7 & sg, const vec & lmk, vec & exp,
		    vec & dist, mat & EXP_sg, mat & EXP_lmk)
		{
			// Some temps of known size
			vec3 v;
			mat V_sg(3, 7);
			mat V_lmk(3, 7);
			mat23 EXP_v;

			// We make the projection.
			// This is decomposed in two steps:
			// - Transform lmk to sensor pose, ready for Bearing-only projection.
			// - Project into pin-hole sensor
			//
			// These functions below use the down-casted pointer because they need to know the particular object parameters and/or methods:
			lmkAHP::toBearingOnlyFrame(sg, lmk, v, dist(0), V_sg, V_lmk);
			pinhole::projectPoint(pinHolePtr->intrinsic, pinHolePtr->distortion, v, exp, EXP_v);

			// We perform Jacobian composition. We use the chain rule.
			EXP_sg = prod(EXP_v, V_sg);
			EXP_lmk = prod(EXP_v, V_lmk);
		}

		void ObservationPinHoleAnchoredHomogeneousPoint::backProject_func(const vec7 & sg, const vec & pix,
		    const vec & invDist, vec & ahp, mat & AHP_sg, mat & AHP_pix, mat AHP_invDist)
		{
			vec3 v;
			mat32 V_pix;
			mat V_sg(3, 1);
			mat AHP_v(7, 3);

			// We make the back-projection.
			// This is decomposed in two steps:
			// - Back-project from pin-hole sensor to bearing-only frame
			// - Transform Bearing-only lmk from sensor pose, using inverse-distance prior.
			//
			// These functions below use the down-casted pointer because they need to know the particular object parameters and/or methods:
			mat V_1(3,1);
			pinhole::backProjectPoint(pinHolePtr->intrinsic, pinHolePtr->correction, pix, 1.0, v, V_pix, V_1);
			lmkAHP::fromBearingOnlyFrame(sg, v, invDist(0), ahp, AHP_sg, AHP_v, AHP_invDist);

			// Here we apply the chain rule for composing Jacobians
			AHP_pix = prod(AHP_v, V_pix);

		}

		bool ObservationPinHoleAnchoredHomogeneousPoint::predictVisibility()
		{
			bool inimg = pinhole::isInImage(expectation.x(), pinHolePtr->imgSize(0), pinHolePtr->imgSize(1));
			bool infront = (expectation.nonObs(0) > 0.0);
			events.visible = inimg && infront;
			return events.visible;
		}

	}
}

