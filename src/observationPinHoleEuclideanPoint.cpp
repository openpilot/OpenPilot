/*
 * observationPinHoleEuclideanPoint.cpp
 *
 *  Created on: Apr 15, 2010
 *      Author: agonzale
 */

#include "rtslam/observationPinHoleEuclideanPoint.hpp"
#include "jmath/ublasExtra.hpp"
#include "jmath/misc.hpp"
#include "boost/shared_ptr.hpp"
#include "rtslam/pinholeTools.hpp"
#include "rtslam/landmarkEuclideanPoint.hpp"
#include "rtslam/descriptorImagePoint.hpp"

namespace jafar {
	namespace rtslam {

		using namespace std;
		using namespace jblas;
		using namespace ublas;

		ObservationModelPinHoleEuclideanPoint::ObservationModelPinHoleEuclideanPoint(
		    const sensor_ptr_t & pinholePtr)
		{
			init_sizes();
			linkToSensorSpecific(pinholePtr);
		}

		ObservationPinHoleEuclideanPoint::ObservationPinHoleEuclideanPoint(
		    const sensor_ptr_t & pinholePtr, const landmark_ptr_t & eucPtr) :
			ObservationAbstract(pinholePtr, eucPtr, 2, 1) {
			modelSpec.reset(new ObservationModelPinHoleEuclideanPoint());
			model = modelSpec;
			type = PNT_PH_EUC;
		}

		void ObservationPinHoleEuclideanPoint::setup(int patchSize, double dmin, double _reparTh)
		{
			//ObservationAbstract::setup(_noiseStd, getPrior());
			Gaussian prior(1); // should never be used
			setPrior(prior);
//			id() = landmarkPtr()->id();
			// TODO: is this cast necessary? Change the arg of the setup if not.
			//linkToPinHole(SPTR_CAST<SensorPinHole>
			//	      (pinholePtr));
			//linkToParentEUC(eucPtr);
			reparTh = _reparTh;
		}


		void ObservationModelPinHoleEuclideanPoint::project_func(const vec7 & sg,
		    const vec & lmk, vec & exp, vec & dist) {
			// resize input vectors
			exp.resize(exp_size);
			dist.resize(prior_size);

			// Some temps of known size
			vec3 v;
			v = quaternion::eucToFrame(sg, lmk);
			dist(0) = norm_2(v)*jmath::sign(v(2));

			exp = pinhole::projectPoint(pinHolePtr()->params.intrinsic, pinHolePtr()->params.distortion, v);
		}

		void ObservationModelPinHoleEuclideanPoint::project_func(const vec7 & sg,
		    const vec & lmk, vec & exp, vec & dist, mat & EXP_sg, mat & EXP_lmk) {
			// resize input vectors
			exp.resize(exp_size);
			dist.resize(prior_size);

			// Some temps of known size
			vec3 v;
			mat V_sg(3, 7);
			mat V_lmk(3, 7);
			mat23 EXP_v;

			quaternion::eucToFrame(sg, lmk, v, V_sg, V_lmk);
			dist(0) = norm_2(v)*jmath::sign(v(2));

			pinhole::projectPoint(pinHolePtr()->params.intrinsic, pinHolePtr()->params.distortion,
			                      v, exp, EXP_v);

			// We perform Jacobian composition. We use the chain rule.
			EXP_sg = prod(EXP_v, V_sg);
			EXP_lmk = prod(EXP_v, V_lmk);
		}

		void ObservationModelPinHoleEuclideanPoint::backProject_func(const vec7 & sg,
		    const vec & meas, const vec & nobs, vec & euc) {

			vec3 v;
			vec4 k = pinHolePtr()->params.intrinsic;
			vec c = pinHolePtr()->params.correction;
			v = pinhole::backprojectPoint(k, c, meas, (double)1.0);
			ublasExtra::normalize(v);
			v *= nobs(0); // nobs is distance
			euc = quaternion::eucFromFrame(sg, v);
		}

		void ObservationModelPinHoleEuclideanPoint::backProject_func(const vec7 & sg,
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

		bool ObservationModelPinHoleEuclideanPoint::predictVisibility_func(jblas::vec x, jblas::vec nobs)
		{
			bool inimg = pinhole::isInImage(x, pinHolePtr()->params.width, pinHolePtr()->params.height);
			bool infront = (nobs(0) > 0.0);
			return inimg && infront;
		}
		

		void ObservationPinHoleEuclideanPoint::predictAppearance_func() {
			//desc_img_pnt_ptr_t descPtr = SPTR_CAST<DescriptorImagePoint>(landmarkPtr()->descriptorPtr);
			//obs_ph_euc_ptr_t _this = SPTR_CAST<ObservationPinHoleEuclideanPoint>(shared_from_this());
			//descPtr->predictAppearance(_this);
			observation_ptr_t _this = shared_from_this();
			landmarkPtr()->descriptorPtr->predictAppearance(_this);
		}

	}
}
