/*
 * observationPinHoleEuclideanPoint.cpp
 *
 *  Created on: Apr 15, 2010
 *      Author: agonzale
 */

#include "boost/shared_ptr.hpp"
#include "rtslam/pinholeTools.hpp"
#include "rtslam/landmarkEuclideanPoint.hpp"
#include "rtslam/observationPinHoleEuclideanPoint.hpp"

namespace jafar {
	namespace rtslam {

		using namespace std;
		using namespace jblas;
		using namespace ublas;

		ObservationPinHoleEuclideanPoint::ObservationPinHoleEuclideanPoint(
		    const sensor_ptr_t & pinholePtr, const landmark_ptr_t & eucPtr) :
			ObservationAbstract(pinholePtr, eucPtr, 2, 1) {
			categoryName("PINHOLE-EUC OBS");
		}

		void ObservationPinHoleEuclideanPoint::project_func(const vec7 & sg,
		    const vec & lmk, vec & exp, vec & dist, mat & EXP_sg, mat & EXP_lmk) {
			// resize input vectors
			exp.resize(expectation.size());
			dist.resize(prior.size());

			// Some temps of known size
			vec3 v;
			mat V_sg(3, 7);
			mat V_lmk(3, 7);
			mat23 EXP_v;

			quaternion::eucToFrame(sg, lmk, v, V_sg, V_lmk);

			pinhole::projectPoint(pinHolePtr()->intrinsic, pinHolePtr()->distortion,
			                      v, exp, EXP_v);

			// We perform Jacobian composition. We use the chain rule.
			EXP_sg = prod(EXP_v, V_sg);
			EXP_lmk = prod(EXP_v, V_lmk);

		}

		void ObservationPinHoleEuclideanPoint::backProject_func(const vec7 & sg,
		    const vec & meas, const vec & nobs, vec & lmk, mat & LMK_sg,
		    mat & LMK_meas, mat LMK_nobs) {
			// todo : implement back-projection of ObsPHEucPt
		}

		bool ObservationPinHoleEuclideanPoint::predictVisibility() {
			bool inimg = pinhole::isInImage(expectation.x(),
			                                pinHolePtr()->imgSize(0),
			                                pinHolePtr()->imgSize(1));
			bool infront = (expectation.nonObs(0) > 0.0);
			events.visible = inimg && infront;
			return events.visible;
		}

		void ObservationPinHoleEuclideanPoint::linkToWeakParentDataManager(
		    void) {
			if (!sensorPtr()) {
				std::cerr << __PRETTY_FUNCTION__
				    << ": error: senPtr not set yet, linkToParentSensor first."
				    << std::endl;
				//throw			"SENPTR no set.";
			}
			SensorAbstract & sen = *sensorPtr();
			typedef SensorAbstract::DataManagerList dmalist_t;
			dmalist_t & dmalist = sen.dataManagerList();
			// Loop
			for (dmalist_t::iterator iter = dmalist.begin(); iter != dmalist.end(); iter++) {
				boost::shared_ptr<DataManagerAbstract> dma = *iter;
				boost::shared_ptr<ImageManagerPoint> dms = boost::dynamic_pointer_cast<
				    ImageManagerPoint>(dma);
				if ((bool) dms) continue; // this is not the proper type ... continue.
				linkToWeakParentDataManager(dms);
				return;
			}
		}

		void ObservationPinHoleEuclideanPoint::predictAppearance() {
			// TODO implement predict appearance
		}

		/**
		 * find and match the expected appearence in the raw-data
		 */
		void ObservationPinHoleEuclideanPoint::matchFeature(
		    raw_ptr_t rawPtr) {
			// TODO call the namespace image with the raw
		}

	}
}
