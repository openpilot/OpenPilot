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
#include "rtslam/descriptorImagePoint.hpp"

namespace jafar {
	namespace rtslam {

		using namespace std;
		using namespace jblas;
		using namespace ublas;

		ObservationPinHoleAnchoredHomogeneousPoint::ObservationPinHoleAnchoredHomogeneousPoint(
		    const sensor_ptr_t & pinholePtr, const landmark_ptr_t & ahpPtr) :
			ObservationAbstract(pinholePtr, ahpPtr, 2, 1) {
			type = PNT_PH_AH;
		}

		void ObservationPinHoleAnchoredHomogeneousPoint::setup(const sensor_ptr_t & pinholePtr, const landmark_ptr_t & ahpPtr, const vec & _noiseStd, int patchSize)
		{
			ObservationAbstract::setup(_noiseStd, getPrior());
			id() = ahpPtr->id();
			linkToParentPinHole(pinholePtr);
			linkToParentAHP(ahpPtr);
			predictedAppearance.reset(new AppearenceImagePoint(patchSize, patchSize, CV_8U));
			observedAppearance.reset(new AppearenceImagePoint(patchSize, patchSize, CV_8U));
		}


		void ObservationPinHoleAnchoredHomogeneousPoint::project_func(
		    const vec7 & sg, const vec & lmk, vec & exp, vec & dist) {
			// OK JS 12/6/2010
			// resize input vectors
			exp.resize(expectation.size());
			dist.resize(prior.size());

			// Some temps of known size
			vec3 v;

			lmkAHP::toBearingOnlyFrame(sg, lmk, v, dist(0));
			vec4 k = pinHolePtr()->params.intrinsic;
			vec d = pinHolePtr()->params.distortion;
			exp = pinhole::projectPoint(k, d, v);
		}

		void ObservationPinHoleAnchoredHomogeneousPoint::project_func(
		    const vec7 & sg, const vec & lmk, vec & exp, vec & dist, mat & EXP_sg,
		    mat & EXP_lmk) {
			// OK JS 12/6/2010
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
			vec4 k = pinHolePtr()->params.intrinsic;
			vec d = pinHolePtr()->params.distortion;
			pinhole::projectPoint(k, d, v, exp, EXP_v);

			// We perform Jacobian composition. We use the chain rule.
			EXP_sg = prod(EXP_v, V_sg);
			EXP_lmk = prod(EXP_v, V_lmk);
		}

		void ObservationPinHoleAnchoredHomogeneousPoint::backProject_func(
		    const vec7 & sg, const vec & pix, const vec & invDist, vec & ahp) {
			// OK JS 12/6/2010
			vec3 v;
			v = pinhole::backprojectPoint(pinHolePtr()->params.intrinsic, pinHolePtr()->params.correction, pix, (double)1.0);
			ublasExtra::normalize(v);
			ahp = lmkAHP::fromBearingOnlyFrame(sg, v, invDist(0));
		}

		void ObservationPinHoleAnchoredHomogeneousPoint::backProject_func(
		    const vec7 & sg, const vec & pix, const vec & invDist, vec & ahp,
		    mat & AHP_sg, mat & AHP_pix, mat & AHP_invDist) {

			// OK JS 12/6/2010
			vec3 v, vn; // 3d vector and normalized vector
			// temporal Jacobians:
			mat V_pix(3,2);
			mat V_sg(3, 7);
			mat AHP_vn(7,3);
			mat V_1(3, 1);
			mat VN_v(3,3), VN_pix(3,2);

			pinhole::backProjectPoint(pinHolePtr()->params.intrinsic, pinHolePtr()->params.correction, pix, 1.0,
			                          v, V_pix, V_1);

			vn = v;
			ublasExtra::normalize(vn);
			ublasExtra::normalizeJac(v, VN_v);

			lmkAHP::fromBearingOnlyFrame(sg, vn, invDist(0), ahp, AHP_sg, AHP_vn,
			                             AHP_invDist);

			// Here we apply the chain rule for composing Jacobians
			VN_pix = prod(VN_v, V_pix);
			AHP_pix = prod(AHP_vn, VN_pix);

		}

		bool ObservationPinHoleAnchoredHomogeneousPoint::predictVisibility() {
			bool inimg = pinhole::isInImage(expectation.x(),
			                                pinHolePtr()->params.width,
			                                pinHolePtr()->params.height);
			bool infront = (expectation.nonObs(0) > 0.0);
			events.visible = inimg && infront;
			return events.visible;
		}

		void ObservationPinHoleAnchoredHomogeneousPoint::linkToWeakParentDataManager(
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

		void ObservationPinHoleAnchoredHomogeneousPoint::predictAppearance() {
			cout << __FILE__ << ":" << __LINE__ << endl;

			desc_img_pnt_ptr_t descPtr = boost::static_pointer_cast<DescriptorImagePoint>(landmarkPtr()->descriptorPtr);
			cout << __FILE__ << ":" << __LINE__ << endl;
			obs_ph_ahp_ptr_t _this = boost::static_pointer_cast<ObservationPinHoleAnchoredHomogeneousPoint>(shared_from_this());
			cout << __FILE__ << ":" << __LINE__ << endl;
			descPtr->predictAppearance(_this);
			cout << __FILE__ << ":" << __LINE__ << endl;
		}

		/**
		 * find and match the expected appearence in the raw-data
		 */
		void ObservationPinHoleAnchoredHomogeneousPoint::matchFeature(
		    raw_ptr_t rawPtr) {
			// TODO call the namespace image with the raw
			// fixme these lines below only for compilation purposes
			measurement.x(expectation.x());
			identity_mat I(2);
			measurement.P(I);
			measurement.matchScore = 1.00;

			events.measured = true;
		}

	}
}

