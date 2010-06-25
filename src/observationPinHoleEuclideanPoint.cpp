/*
 * observationPinHoleEuclideanPoint.cpp
 *
 *  Created on: Apr 15, 2010
 *      Author: agonzale
 */

#include "rtslam/observationPinHoleEuclideanPoint.hpp"
#include "jmath/ublasExtra.hpp"
#include "boost/shared_ptr.hpp"
#include "rtslam/pinholeTools.hpp"
#include "rtslam/landmarkEuclideanPoint.hpp"
#include "rtslam/descriptorImagePoint.hpp"

namespace jafar {
	namespace rtslam {

		using namespace std;
		using namespace jblas;
		using namespace ublas;

		ObservationPinHoleEuclideanPoint::ObservationPinHoleEuclideanPoint(
		    const sensor_ptr_t & pinholePtr, const landmark_ptr_t & eucPtr) :
			ObservationAbstract(pinholePtr, eucPtr, 2, 1) {
			type = PNT_PH_EUC;
		}

		void ObservationPinHoleEuclideanPoint::setup(const sensor_ptr_t & pinholePtr, const landmark_ptr_t & eucPtr, double _noiseStd, int patchSize)
		{
			ObservationAbstract::setup(_noiseStd, getPrior());
			id() = eucPtr->id();
			// TODO: is this cast necessary? Change the arg of the setup if not.
			linkToPinHole(boost::dynamic_pointer_cast<SensorPinHole>
				      (pinholePtr));
			linkToParentEUC(eucPtr);
			predictedAppearance.reset(new AppearanceImagePoint(patchSize, patchSize, CV_8U));
			observedAppearance.reset(new AppearanceImagePoint(patchSize, patchSize, CV_8U));
		}


		void ObservationPinHoleEuclideanPoint::project_func(const vec7 & sg,
		    const vec & lmk, vec & exp, vec & dist) {
			// resize input vectors
			exp.resize(expectation.size());
			dist.resize(prior.size());

			// Some temps of known size
			vec3 v;
			v = quaternion::eucToFrame(sg, lmk);

			exp = pinhole::projectPoint(pinHolePtr()->params.intrinsic, pinHolePtr()->params.distortion, v);
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

			pinhole::projectPoint(pinHolePtr()->params.intrinsic, pinHolePtr()->params.distortion,
			                      v, exp, EXP_v);

			// We perform Jacobian composition. We use the chain rule.
			EXP_sg = prod(EXP_v, V_sg);
			EXP_lmk = prod(EXP_v, V_lmk);
		}

		void ObservationPinHoleEuclideanPoint::backProject_func(const vec7 & sg,
		    const vec & meas, const vec & nobs, vec & euc) {

			vec3 v;
			vec4 k = pinHolePtr()->params.intrinsic;
			vec c = pinHolePtr()->params.correction;
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

			pinhole::backProjectPoint(pinHolePtr()->params.intrinsic, pinHolePtr()->params.correction, meas, 1.0,
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
			                                pinHolePtr()->params.width,
			                                pinHolePtr()->params.height);
			bool infront = (expectation.nonObs(0) > 0.0);
			events.visible = inimg && infront;
			return events.visible;
		}

		void ObservationPinHoleEuclideanPoint::predictAppearance() {
			desc_img_pnt_ptr_t descPtr = boost::static_pointer_cast<DescriptorImagePoint>(landmarkPtr()->descriptorPtr);
			obs_ph_euc_ptr_t _this = boost::static_pointer_cast<ObservationPinHoleEuclideanPoint>(shared_from_this());
			descPtr->predictAppearance(_this);
		}

	}
}
