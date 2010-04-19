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
#include "rtslam/landmarkAnchoredHomogeneousPoint.hpp"
#include "rtslam/observationPinHoleAnchoredHomogeneous.hpp"

namespace jafar {
	namespace rtslam {

		using namespace std;
		using namespace jblas;
		using namespace ublas;

		ObservationPinHoleAnchoredHomogeneousPoint::ObservationPinHoleAnchoredHomogeneousPoint(
		    const sensor_ptr_t & pinholePtr, const landmark_ptr_t & ahpPtr) :

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
			// Some temps of known size
			vec7 sg;
			vec3 v;
			vec2 u;
			double invDist;
			mat V_sg(3, 7);
			mat V_ahp(3, 7);
			mat23 U_v;


			// Need to know some dynamic sizes:
			// Sizes of Jacobian SG_rs depend on the sensor being LOCAL or REMOTE.
			// This is easy since we have ia_rsl with the overall size of mapped robot, sensor and landmark.
			size_t size_rsl = ia_rsl.size();
			size_t size_l = landmarkPtr->size();
			size_t size_rs = size_rsl - size_l;

			mat SG_rs(sensorPtr->pose.size(), size_rs); // This temp is of variable size <-- we need to know size_rs before initializing it.

			// We make the projection.
			// This is decomposed in three steps:
			// - Get global sensor pose.
			// - Transform lmk to sensor pose, ready for Bearing-only projection.
			// - Project into pin-hole sensor
			//
			// Here it is OK to use the abstract pointer:
			sensorPtr->globalPose(sg, SG_rs); //      Pose of sensor in map; Jacobian DG_rs wrt. robot and sensor
			// These functions below use the down-casted pointer because they need to know the particular object parameters and/or methods:
			ahpPtr->toBearingOnlyFrame(sg, v, invDist, V_sg, V_ahp); // lmk in sensor frame
			pinHolePtr->projectPoint(v, u, U_v); //   Project lmk, get expected pixel u and Jacobians

			// We perform Jacobian composition. We use the chain rule.
			mat V_rs = prod(V_sg, SG_rs);
			mat U_rs = prod(U_v, V_rs);
			mat U_ahp = prod(U_v, V_ahp);


			// Write results:
			expectation.x() = u; //                   Output: assign expectation mean and Jacobians
			ublas::subrange(EXP_rsl, 0, 2, 0, size_rs) = U_rs;
			ublas::subrange(EXP_rsl, 0, 2, size_rs, size_rsl) = U_ahp;
			expectation.nonObs(0) = invDist; //       Assign non-observable part (inverse-distance)
		}

		void ObservationPinHoleAnchoredHomogeneousPoint::backProject_func() {
			vec3 v;
			mat32 V_u;
			mat V_s(3,1);
			vec7 sg;
			mat AHP_sg(7, 7);
			mat AHP_v(7,3);
			mat AHP_u(7,2);
			mat AHP_rho(7,1);

			// Need to know some dynamic sizes:
			// Sizes of Jacobian SG_rs depend on the sensor being LOCAL or REMOTE.
			// This is easy since we have ia_rsl with the overall size of mapped robot, sensor and landmark.
			size_t size_rsl = ia_rsl.size();
			size_t size_l = landmarkPtr->size();
			size_t size_rs = size_rsl - size_l;

			// This matrices are of variable size <-- we need to know size_rs before initializing them.
			mat SG_rs(sensorPtr->pose.size(), size_rs);
			mat AHP_rs(7, size_rs);

			// Inputs: measurement and prior
			vec2 u = measurement.x();
			double rho = prior.x(0);

			// We make the back-projection.
			// This is decomposed in three steps:
			// - Get global sensor pose.
			// - Back-project from pin-hole sensor to bearing-only frame
			// - Transform Bearing-only lmk from sensor pose, using inverse-distance prior.
			//
			// Here it is OK to use the abstract pointer:
			sensorPtr->globalPose(sg, SG_rs); //      Pose of sensor in map; Jacobian SG_rs wrt. robot and sensor
			// These functions below use the down-casted pointer because they need to know the particular object parameters and/or methods:
			pinHolePtr->backProjectPoint(u, 1.0, v, V_u, V_s); //   back-Project pixel u using prior depth 1, get expected 3D vector v and Jacobians
			ahpPtr->fromBearingOnlyFrame(sg, v, rho, AHP_sg, AHP_v, AHP_rho); // lmk in global frame

			// Here we apply the chain rule for composing Jacobians
			AHP_rs = prod(AHP_sg, SG_rs);
			AHP_u = prod(AHP_v, V_u);

		}

		bool ObservationPinHoleAnchoredHomogeneousPoint::predictVisibility() {
			bool inimg = pinhole::isInImage(expectation.x(), pinHolePtr->imgSize(0), pinHolePtr->imgSize(1));
			bool infront = (expectation.nonObs(0) > 0.0);
			events.visible = inimg && infront;
			return events.visible;
		}

	}
}

