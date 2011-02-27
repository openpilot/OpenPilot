/**
 * \file observationPinHoleAnchoredHomogeneous.cpp
 * \date 14/03/2010
 * \author jsola
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

		ObservationModelPinHoleAnchoredHomogeneousPoint::ObservationModelPinHoleAnchoredHomogeneousPoint(
			const sensor_ptr_t & pinholePtr)
		{
			init_sizes();
			linkToSensorSpecific(pinholePtr);
		}
		
		ObservationPinHoleAnchoredHomogeneousPoint::ObservationPinHoleAnchoredHomogeneousPoint(
		    const sensor_ptr_t & pinholePtr, const landmark_ptr_t & ahpPtr) :
			ObservationAbstract(pinholePtr, ahpPtr, 2, 1) {
			modelSpec.reset(new ObservationModelPinHoleAnchoredHomogeneousPoint());
			model = modelSpec;
			type = PNT_PH_AH;
		}

		void ObservationPinHoleAnchoredHomogeneousPoint::setup(double dmin)
		{
			//ObservationAbstract::setup(reparTh, killSizeTh, killSearchTh, killMatchTh, killConsistencyTh);
			//ObservationAbstract::setup(_noiseStd, getPrior());
			Gaussian prior(1);
			prior.x(0) = 1/(3*dmin);
			prior.P(0,0) = prior.x(0)*prior.x(0);
			setPrior(prior);
		}


		void ObservationModelPinHoleAnchoredHomogeneousPoint::project_func(
		    const vec7 & sg, const vec & lmk, vec & exp, vec & dist) {
			// OK JS 12/6/2010
			// resize input vectors
			exp.resize(exp_size);
			dist.resize(prior_size);

			// Some temps of known size
			vec3 v;

			lmkAHP::toBearingOnlyFrame(sg, lmk, v, dist(0));
			dist(0) *= jmath::sign(v(2));
			vec4 k = pinHolePtr()->params.intrinsic;
			vec d = pinHolePtr()->params.distortion;
			exp = pinhole::projectPoint(k, d, v);
		}

		void ObservationModelPinHoleAnchoredHomogeneousPoint::project_func(
		    const vec7 & sg, const vec & lmk, vec & exp, vec & dist, mat & EXP_sg,
		    mat & EXP_lmk) {
			// OK JS 12/6/2010
			// resize input vectors
			exp.resize(exp_size);
			dist.resize(prior_size);

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
			dist(0) *= jmath::sign(v(2));
			vec4 k = pinHolePtr()->params.intrinsic;
			vec d = pinHolePtr()->params.distortion;
			pinhole::projectPoint(k, d, v, exp, EXP_v);

			// We perform Jacobian composition. We use the chain rule.
			EXP_sg = prod(EXP_v, V_sg);
			EXP_lmk = prod(EXP_v, V_lmk);
		}

		void ObservationModelPinHoleAnchoredHomogeneousPoint::backProject_func(
		    const vec7 & sg, const vec & pix, const vec & invDist, vec & ahp) {
			// OK JS 12/6/2010
			vec3 v;
			v = pinhole::backprojectPoint(pinHolePtr()->params.intrinsic, pinHolePtr()->params.correction, pix, (double)1.0);
			ublasExtra::normalize(v);
			ahp = lmkAHP::fromBearingOnlyFrame(sg, v, invDist(0));
		}

		void ObservationModelPinHoleAnchoredHomogeneousPoint::backProject_func(
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

		bool ObservationModelPinHoleAnchoredHomogeneousPoint::predictVisibility_func(jblas::vec x, jblas::vec nobs)
		{
			bool inimg = pinhole::isInImage(x, pinHolePtr()->params.width, pinHolePtr()->params.height);
			bool infront = (nobs(0) > 0.0);
// JFR_DEBUG("ObservationModelPHAHP::predictVisibility_func x " << x << " nobs " << nobs << " inimg/infront " << inimg << "/" << infront);
			return inimg && infront;
		}
		

		bool ObservationPinHoleAnchoredHomogeneousPoint::predictAppearance_func() {
			observation_ptr_t _this = shared_from_this();
			return landmarkPtr()->descriptorPtr->predictAppearance(_this);
		}

		void ObservationPinHoleAnchoredHomogeneousPoint::desc_image(image::oimstream& os) const
		{
			if (events.predictedApp)
			{
				app_img_pnt_ptr_t predApp = SPTR_CAST<AppearanceImagePoint>(predictedAppearance);
				os << predApp->patch << image::hsep;
			}
			
			if (events.measured)
			{
				app_img_pnt_ptr_t obsApp = SPTR_CAST<AppearanceImagePoint>(observedAppearance);
				os << obsApp->patch << image::endl;
			}
		}

	}
}

