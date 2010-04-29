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
		                                                                                       const sensor_ptr_t & pinholePtr,
		                                                                                       const landmark_ptr_t & ahpPtr) :
			ObservationAbstract(pinholePtr, ahpPtr, 2, 1) {
			categoryName("PINHOLE-AHP OBS");
		}

		void ObservationPinHoleAnchoredHomogeneousPoint::project_func(const vec7 & sg, const vec & lmk, vec & exp,
		                                                              vec & dist, mat & EXP_sg, mat & EXP_lmk) {
			// resize input vectors
			exp.resize(expectation.size());
			dist.resize(prior.size());

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
			vec4 k = pinHolePtr()->intrinsic;
			vec d = pinHolePtr()->distortion;
			pinhole::projectPoint(k, d, v, exp, EXP_v);


			// We perform Jacobian composition. We use the chain rule.
			EXP_sg = prod(EXP_v, V_sg);
			EXP_lmk = prod(EXP_v, V_lmk);
		}

		void ObservationPinHoleAnchoredHomogeneousPoint::backProject_func(const vec7 & sg, const vec & pix,
		                                                                  const vec & invDist, vec & ahp, mat & AHP_sg,
		                                                                  mat & AHP_pix, mat AHP_invDist) {

			cout << "sg:" << sg << endl;
			cout << "pix:" << pix << endl;
			cout << "invDist:" << invDist << endl;


			vec3 v;
			mat32 V_pix;
			mat V_sg(3, 7);
			mat AHP_v(7, 3);


			// We make the back-projection.
			// This is decomposed in two steps:
			// - Back-project from pin-hole sensor to bearing-only frame
			// - Transform Bearing-only lmk from sensor pose, using inverse-distance prior.
			//
			// These functions below use the down-casted pointer because they need to know the particular object parameters and/or methods:
			mat V_1(3, 1);
			pinhole_ptr_t phPtr = pinHolePtr();
			pinhole::backProjectPoint(phPtr->intrinsic, phPtr->correction, pix, 1.0, v, V_pix, V_1);

			cout << "v:" << v << endl;

			lmkAHP::fromBearingOnlyFrame(sg, v, invDist(0), ahp, AHP_sg, AHP_v, AHP_invDist);

			cout << "ahp:" << ahp << endl;


			// Here we apply the chain rule for composing Jacobians
			AHP_pix = prod(AHP_v, V_pix);

		}

		bool ObservationPinHoleAnchoredHomogeneousPoint::predictVisibility() {
			bool inimg = pinhole::isInImage(expectation.x(), pinHolePtr()->imgSize(0), pinHolePtr()->imgSize(1));
			bool infront = (expectation.nonObs(0) > 0.0);
			events.visible = inimg && infront;
			return events.visible;
		}

		void ObservationPinHoleAnchoredHomogeneousPoint::linkToWeakParentDataManager(void) {
			if (!sensorPtr()) {
				std::cerr << __PRETTY_FUNCTION__ << ": error: senPtr not set yet, linkToParentSensor first." << std::endl;
				//throw			"SENPTR no set.";
			}
			SensorAbstract & sen = *sensorPtr();
			typedef SensorAbstract::DataManagerList dmalist_t;
			dmalist_t & dmalist = sen.dataManagerList();
			// Loop
			for (dmalist_t::iterator iter = dmalist.begin(); iter != dmalist.end(); iter++) {
				boost::shared_ptr<DataManagerAbstract> dma =  *iter;
				boost::shared_ptr<ImageManagerPoint> dms = boost::dynamic_pointer_cast<ImageManagerPoint>(dma);
				if ((bool)dms)
					continue; // this is not the proper type ... continue.
				linkToWeakParentDataManager(dms);
				return;
			}
		}

		void ObservationPinHoleAnchoredHomogeneousPoint::predictAppearance(){
			// TODO implement predict appearance
		}

		/**
		 * find and match the expected appearence in the raw-data
		 */
		void ObservationPinHoleAnchoredHomogeneousPoint::matchFeature(raw_ptr_t rawPtr) {
			// TODO call the namespace image with the raw
		}


	}
}

