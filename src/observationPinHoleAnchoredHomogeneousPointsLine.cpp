
/**
 * \file observationPinHoleAnchoredHomogeneousPointsLine.cpp
 * \date 25/02/2011
 * \author bhautboi
 * \ingroup rtslam
 */

#include "boost/shared_ptr.hpp"
#include "rtslam/pinholeTools.hpp"
#include "rtslam/ahplTools.hpp"
#include "rtslam/landmarkAnchoredHomogeneousPointsLine.hpp"
#include "rtslam/observationPinHoleAnchoredHomogeneousPointsLine.hpp"
#include "rtslam/descriptorImageSeg.hpp"

namespace jafar {
   namespace rtslam {

      using namespace std;
      using namespace jblas;
      using namespace ublas;

      ObservationModelPinHoleAnchoredHomogeneousPointsLine::ObservationModelPinHoleAnchoredHomogeneousPointsLine(
         const sensor_ptr_t & pinholePtr)
      {
         init_sizes();
         linkToSensorSpecific(pinholePtr);
      }

      ObservationPinHoleAnchoredHomogeneousPointsLine::ObservationPinHoleAnchoredHomogeneousPointsLine(
          const sensor_ptr_t & pinholePtr, const landmark_ptr_t & ahplPtr) :
         ObservationAbstract(pinholePtr, ahplPtr, 4, 2) {
         modelSpec.reset(new ObservationModelPinHoleAnchoredHomogeneousPointsLine());
         model = modelSpec;
         type = PNT_PH_AHPL;
      }

      void ObservationPinHoleAnchoredHomogeneousPointsLine::setup(double reparTh, int killSizeTh, int killSearchTh, double killMatchTh, double killConsistencyTh, double dmin)
      {
         ObservationAbstract::setup(reparTh, killSizeTh, killSearchTh, killMatchTh, killConsistencyTh);
         //ObservationAbstract::setup(_noiseStd, getPrior());
         Gaussian prior(2);
         prior.x(0) = 1/(3*dmin);
         prior.x(1) = 1/(3*dmin);
         prior.P(0,0) = prior.x(0)*prior.x(0);
         prior.P(1,1) = prior.x(1)*prior.x(1);
         setPrior(prior);
      }


      void ObservationModelPinHoleAnchoredHomogeneousPointsLine::project_func(
          const vec7 & sg, const vec & lmk, vec & exp, vec & dist) {
         // resize input vectors
         exp.resize(exp_size);
         dist.resize(prior_size);

         // Some temps of known size
         vec3 v1;
         vec3 v2;

         lmkAHPL::toBearingOnlyFrame(sg, lmk, v1, v2, dist(0), dist(1));
         dist(0) *= jmath::sign(v1(2));
         dist(1) *= jmath::sign(v2(2));
         vec4 k = pinHolePtr()->params.intrinsic;
         vec d = pinHolePtr()->params.distortion;
         subrange(exp,0,2) = pinhole::projectPoint(k, d, v1);
         subrange(exp,2,4) = pinhole::projectPoint(k, d, v2);
      }

      void ObservationModelPinHoleAnchoredHomogeneousPointsLine::project_func(
          const vec7 & sg, const vec & lmk, vec & exp, vec & dist, mat & EXP_sg,
          mat & EXP_lmk) {
         // resize input vectors
         exp.resize(exp_size);
         dist.resize(prior_size);

         // Some temps of known size
         vec3 v1;
         vec3 v2;
         mat V_sg(6, 7);
         mat V_lmk(6, 11);
         mat EXP_v(4, 6);

         // We make the projection.
         // This is decomposed in two steps:
         // - Transform lmk to sensor pose, ready for Bearing-only projection.
         // - Project into pin-hole sensor
         //
         // These functions below use the down-casted pointer because they need to know the particular object parameters and/or methods:
         mat V1_sg(3,7), V2_sg(3,7);
         mat V1_lmk(3,11), V2_lmk(3,11);
         lmkAHPL::toBearingOnlyFrame(sg, lmk, v1, v2, dist(0), dist(1),V1_sg,V1_lmk,V2_sg,V2_lmk);
         subrange(V_sg,0,3,0,7)  = V1_sg;
         subrange(V_lmk,0,3,0,11) = V1_lmk;
         subrange(V_sg,3,6,0,7)  = V2_sg;
         subrange(V_lmk,3,6,0,11) = V2_lmk;

         dist(0) *= jmath::sign(v1(2));
         dist(1) *= jmath::sign(v2(2));
         vec4 k = pinHolePtr()->params.intrinsic;
         vec d = pinHolePtr()->params.distortion;

         mat EXP1_v1(2,3), EXP2_v2(2,3);
         vec2 exp1, exp2;
         pinhole::projectPoint(k, d, v1, exp1, EXP1_v1);
         pinhole::projectPoint(k, d, v2, exp2, EXP2_v2);
         subrange(exp,0,2) = exp1;
         subrange(exp,2,4) = exp2;
         subrange(EXP_v,0,2,0,3) = EXP1_v1;
         subrange(EXP_v,2,4,3,6) = EXP2_v2;
         // We perform Jacobian composition. We use the chain rule.
         EXP_sg  = prod(EXP_v, V_sg );
         EXP_lmk = prod(EXP_v, V_lmk);
      }

      void ObservationModelPinHoleAnchoredHomogeneousPointsLine::backProject_func(
          const vec7 & sg, const vec & pix, const vec & invDist, vec & ahpl) {
         vec3 v1;
         vec3 v2;
         v1 = pinhole::backprojectPoint(pinHolePtr()->params.intrinsic, pinHolePtr()->params.correction, subrange(pix,0,2), (double)1.0);
         ublasExtra::normalize(v1);
         v2 = pinhole::backprojectPoint(pinHolePtr()->params.intrinsic, pinHolePtr()->params.correction, subrange(pix,2,4), (double)1.0);
         ublasExtra::normalize(v2);
         ahpl = lmkAHPL::fromBearingOnlyFrame(sg, v1, v2, invDist(0), invDist(1));
      }

      void ObservationModelPinHoleAnchoredHomogeneousPointsLine::backProject_func(
          const vec7 & sg, const vec & pix, const vec & invDist, vec & ahpl,
          mat & AHPL_sg, mat & AHPL_pix, mat & AHPL_invDist) {

         vec6 v, vn; // 3d vector and normalized vector
         // temporal Jacobians:
         mat V_pix(6,4);
 //        mat V_sg(6, 7);
         mat AHPL_vn(11,6);
         mat V_1(6, 1);
         mat VN_v(6,6), VN_pix(6,4);

         mat V1_pix1(3,2), V2_pix2(3,2);
         mat V1_1(3,1), V2_1(3,1);
         vec3 v1,v2;
         pinhole::backProjectPoint(pinHolePtr()->params.intrinsic, pinHolePtr()->params.correction,
                                   pix, 1.0, v1, V1_pix1, V1_1);
         pinhole::backProjectPoint(pinHolePtr()->params.intrinsic, pinHolePtr()->params.correction,
                                   pix, 1.0, v2, V2_pix2, V2_1);
         subrange(v,0,3) = v1;
         subrange(v,3,6) = v2;
         subrange(V_1,0,3,0,1) = V1_1;
         subrange(V_1,3,6,0,1) = V2_1;
         subrange(V_pix,0,3,0,2) = V1_pix1;
         subrange(V_pix,3,6,2,4) = V2_pix2;

         vn = v;
         ublasExtra::normalize(vn);
         ublasExtra::normalizeJac(v, VN_v);

         mat AHPL_vn1(11,3), AHPL_vn2(11,3);
         mat AHPL_invDist1(11,1), AHPL_invDist2(11,1);
         vec3 vn1 = subrange(vn,0,3);
         vec3 vn2 = subrange(vn,3,6);
         double rho1 = invDist(0);
         double rho2 = invDist(1);
         lmkAHPL::fromBearingOnlyFrame(sg, vn1, vn2, rho1, rho2,
                                       ahpl, AHPL_sg, AHPL_vn1, AHPL_vn2, AHPL_invDist1, AHPL_invDist2);
         subrange(AHPL_vn,0,11,0,3) = AHPL_vn1;
         subrange(AHPL_vn,0,11,3,6) = AHPL_vn2;
         subrange(AHPL_invDist,0,11,0,1) = AHPL_invDist1;
         subrange(AHPL_invDist,0,11,1,2) = AHPL_invDist2;

         // Here we apply the chain rule for composing Jacobians
         VN_pix = prod(VN_v, V_pix);
         AHPL_pix = prod(AHPL_vn, VN_pix);

      }

      bool ObservationModelPinHoleAnchoredHomogeneousPointsLine::predictVisibility_func(jblas::vec x, jblas::vec nobs)
      {
         bool inimg = pinhole::isInImage(x, pinHolePtr()->params.width, pinHolePtr()->params.height);
         bool infront = (nobs(0) > 0.0);
// JFR_DEBUG("ObservationModelPHAHP::predictVisibility_func x " << x << " nobs " << nobs << " inimg/infront " << inimg << "/" << infront);
         return inimg && infront;
      }


      bool ObservationPinHoleAnchoredHomogeneousPointsLine::predictAppearance_func() {
         observation_ptr_t _this = shared_from_this();
         return landmarkPtr()->descriptorPtr->predictAppearance(_this);
      }

      bool ObservationPinHoleAnchoredHomogeneousPointsLine::voteForReparametrizingLandmark(){
//			cout << "evaluating linearity for lmk: " << id() << endl;
         return (lmkAHPL::linearityScore(sensorPtr()->globalPose(), landmarkPtr()->state.x(), landmarkPtr()->state.P()) < reparTh);
      }

      void ObservationPinHoleAnchoredHomogeneousPointsLine::desc_image(image::oimstream& os) const
      {
         if (events.predictedApp)
         {
            app_img_seg_ptr_t predApp = SPTR_CAST<AppearanceImageSegment>(predictedAppearance);
            os << predApp->patch << image::hsep;
         }

         if (events.measured)
         {
            app_img_seg_ptr_t obsApp = SPTR_CAST<AppearanceImageSegment>(observedAppearance);
            os << obsApp->patch << image::endl;
         }
      }

   }
}

