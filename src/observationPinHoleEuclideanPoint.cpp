/*
 * observationPinHoleEuclideanPoint.cpp
 *
 *  Created on: Apr 15, 2010
 *      Author: agonzale
 */

#include "jmath/ublasExtra.hpp"
#include "boost/shared_ptr.hpp"
#include "rtslam/pinholeTools.hpp"
#include "rtslam/landmarkEuclideanPoint.hpp"
#include "rtslam/observationPinHoleEuclideanPoint.hpp"
#include "rtslam/descriptorImagePoint.hpp"

namespace jafar {
	namespace rtslam {

		using namespace std;
		using namespace jblas;
		using namespace ublas;

		ObservationPinHoleEuclideanPoint::ObservationPinHoleEuclideanPoint(
		    const sensor_ptr_t & pinholePtr, const landmark_ptr_t & eucPtr, int patchSize) :
			ObservationAbstract(pinholePtr, eucPtr, 2, 1) {
			type = PNT_PH_EUC;
		}

		void ObservationPinHoleEuclideanPoint::setup(const sensor_ptr_t & pinholePtr, const landmark_ptr_t & eucPtr, const vec & _noiseStd, int patchSize)
		{
			ObservationAbstract::setup(_noiseStd, getPrior());
			id() = ahpPtr->id();
			linkToParentPinHole(pinholePtr);
			linkToParentEuc(eucPtr);
			predictedAppearance.reset(new AppearenceImagePoint(patchSize, patchSize, CV_8U));
			observedAppearance.reset(new AppearenceImagePoint(patchSize, patchSize, CV_8U));
		}


		void ObservationPinHoleEuclideanPoint::project_func(const vec7 & sg,
		    const vec & lmk, vec & exp, vec & dist) {
			// resize input vectors
			exp.resize(expectation.size());
			dist.resize(prior.size());

			// Some temps of known size
			vec3 v;
			v = quaternion::eucToFrame(sg, lmk);

			exp = pinhole::projectPoint(pinHolePtr()->intrinsic, pinHolePtr()->distortion, v);
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
		    const vec & meas, const vec & nobs, vec & euc) {

			vec3 v;
			vec4 k = pinHolePtr()->intrinsic;
			vec c = pinHolePtr()->correction;
			v = pinhole::backprojectPoint(k, c, meas, (double)1.0);
			ublasExtra::normalize(v);
			v *= nobs(0); // nobs is distance
			euc = quaternion::eucFromFrame(sg, v);
		}

		void ObservationPinHoleEuclideanPoint::backProject_func(const vec7 & sg,
		    const vec & meas, const vec & nobs, vec & euc, mat & EUC_sg,
		    mat & EUC_meas, mat & EUC_nobs) {

			vec3 v;
			mat V_meas(3, 2);
			mat V_1(3, 1), VN_v(3,3), VN_meas(3, 2), VS_nobs(3,1), VS_vn(3,3), EUC_vs(3,3), VS_meas(3,2);

			pinhole::backProjectPoint(pinHolePtr()->intrinsic, pinHolePtr()->correction, meas, 1.0,
			                          v, V_meas, V_1);

			vec3 vn = v;
			ublasExtra::normalize(vn); // nobs is inverse-distance
			ublasExtra::normalizeJac(v, VN_v);
			vec3 vs = vn / nobs(0);
			VS_vn = identity_mat(3) / nobs(0);
			ublas::column(VS_nobs,1) = - vn / (nobs(0)*nobs(0));
			VN_meas = prod(VN_v, V_meas);
			VS_meas = prod(VS_vn, VN_meas);

			quaternion::eucFromFrame(sg, vs, euc, EUC_sg, EUC_vs);

			EUC_nobs = prod(EUC_vs, VS_nobs);
			EUC_meas = prod(EUC_vs, VS_meas);

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
			desc_img_pnt_ptr_t descPtr = boost::static_pointer_cast<DescriptorImagePoint>(landmarkPtr()->descriptorPtr);
			obs_ph_euc_ptr_t _this = boost::static_pointer_cast<ObservationPinHoleEuclideanPoint>(shared_from_this());
			descPtr->predictAppearance(_this);
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
