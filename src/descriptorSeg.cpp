/*
 * descriptorSeg.cpp
 *
 *  Created on: 24 feb. 2011
 *      Author: bhautboi@laas.fr
 */

#ifdef HAVE_MODULE_DSEG

#include "jmath/ublasExtra.hpp"
#include "jmath/angle.hpp"

#include "rtslam/descriptorSeg.hpp"
#include "rtslam/rawImage.hpp"
#include "rtslam/quatTools.hpp"

#include "dseg/SegmentHypothesis.hpp"
#include "dseg/LineFitterKalman2.hpp"

namespace jafar {
   namespace rtslam {
      using namespace ublasExtra;
      using namespace quaternion;

      std::ostream& operator <<(std::ostream & s, SegFeatureView const & fv)
      {
         app_seg_ptr_t app = SPTR_CAST<AppearanceSegment>(fv.appearancePtr);
         s << "  -" << (fv.used ? " [*] " : " ") << "at frame " << fv.frame <<
            " by sensor " << fv.obsModelPtr->sensorPtr()->id() << " of " <<
            fv.obsModelPtr->sensorPtr()->typeName() << " at " << fv.senPose <<
            ", with hypothesis " << app->hypothesis();
         return s;
      }
      image::oimstream& operator <<(image::oimstream & s, SegFeatureView const & fv)
      {
         app_seg_ptr_t app = SPTR_CAST<AppearanceSegment>(fv.appearancePtr);
         //s << app->hypothesis();
         return s;
      }

      /***************************************************************************
       * SegFeatureView
       **************************************************************************/

      void SegFeatureView::initFromObs(const observation_ptr_t & obsPtr, int descSize)
      {
         obs_ph_ahpl_ptr_t obsSpecPtr = SPTR_CAST<ObservationPinHoleAnchoredHomogeneousPointsLine>(obsPtr);
         appearancePtr = obsSpecPtr->observedAppearance;
         senPose = obsPtr->sensorPtr()->globalPose();
         obsModelPtr = obsPtr->model;
         measurement = obsPtr->measurement.x();
         frame = obsPtr->sensorPtr()->rawCounter;
         used = false;
      }

      /***************************************************************************
       * DescriptorSegFirstView
       **************************************************************************/

      DescriptorSegFirstView::DescriptorSegFirstView(int descSize):
         DescriptorAbstract(),
         descSize(descSize)
      {
      }

      DescriptorSegFirstView::~DescriptorSegFirstView() {
      }

      void DescriptorSegFirstView::addObservation(const observation_ptr_t & obsPtr)
      {
         if (obsPtr->events.updated && !view.appearancePtr)
            view.initFromObs(obsPtr, descSize);
      }


      bool DescriptorSegFirstView::predictAppearance(const observation_ptr_t & obsPtrNew) {
/*
         landmark_ptr_t lmkPtr = obsPtrNew->landmarkPtr();
         vec6 pnts = lmkPtr->reparametrized();
         vec3 pnt1 = subrange(pnts, 0, 3);
         vec3 pnt2 = subrange(pnts, 3, 6);
         // normally we must cast to the derived type
         app_seg_ptr_t app_dst = SPTR_CAST<AppearanceSegment>(obsPtrNew->predictedAppearance);
//         app_seg_ptr_t app_src = SPTR_CAST<AppearanceSegment>(view.appearancePtr);
         // project the expected position of the segment in the image
         pinhole_ptr_t pinHolePtr = SPTR_CAST<SensorPinHole>(obsPtrNew->sensorPtr());
         vec3 v1 = quaternion::eucToFrame(view.senPose, pnt1);
         vec2 exp1 = pinhole::projectPoint(pinHolePtr->params.intrinsic, pinHolePtr->params.distortion, v1);
         vec3 v2 = quaternion::eucToFrame(view.senPose, pnt2);
         vec2 exp2 = pinhole::projectPoint(pinHolePtr->params.intrinsic, pinHolePtr->params.distortion, v2);
         JFR_DEBUG(exp1 << "\n" << exp2);
          app_seg_ptr_t app_obs = SPTR_CAST<AppearanceSegment>(obsPtrNew->observedAppearance);
         JFR_DEBUG(app_obs->hypothesis()->lineFitter().angle());
         JFR_DEBUG(hypothesis->lineFitter().angle());
*/
         vec4 exp = obsPtrNew->expectation.x();
         double x_o = (exp(0) + exp(2))/2;
         double y_o = (exp(1) + exp(3))/2;
         double angle = atan2(exp(3)-exp(1),exp(2)-exp(0)) - M_PI_2;
         double distance = sqrt((exp(2)-exp(0))*(exp(2)-exp(0)) + (exp(3)-exp(1))*(exp(3)-exp(1)));
         double distance_cube = distance * distance * distance;
         double x1mx2 = exp(0) - exp(2);
         double y1my2 = exp(1) - exp(3);

        // Build the hypothesis
         dseg::SegmentHypothesis* hypothesis = new dseg::SegmentHypothesis(x_o, y_o, angle);
         hypothesis->setExtremity1(exp(0),exp(1));
         hypothesis->setExtremity2(exp(2),exp(3));
         app_seg_ptr_t app_src = SPTR_CAST<AppearanceSegment>(obsPtrNew->observedAppearance);
         hypothesis->setGradientDescriptor(app_src->hypothesis()->gradientDescriptor());

        // Compute uncertainty
         mat44 SIGMA_exp;
         mat44 sigma_cov;
         SIGMA_exp.clear();
         SIGMA_exp(0,0) = 0.5; // x0 wrt x1
         SIGMA_exp(0,2) = 0.5; // x0 wrt x2
         SIGMA_exp(1,1) = 0.5; // y0 wrt y1
         SIGMA_exp(1,3) = 0.5; // y0 wrt y2
			SIGMA_exp(2,0) =  (x1mx2*y1my2) / distance_cube; // u wrt x1
			SIGMA_exp(2,2) = -(x1mx2*y1my2) / distance_cube; // u wrt x2
			SIGMA_exp(2,1) = -(x1mx2*x1mx2) / distance_cube; // u wrt y1
         SIGMA_exp(2,3) =  (x1mx2*x1mx2) / distance_cube; // u wrt y2
         SIGMA_exp(3,0) = -(y1my2*y1my2) / distance_cube; // v wrt x1
         SIGMA_exp(3,2) =  (y1my2*y1my2) / distance_cube; // v wrt x2
			SIGMA_exp(3,1) =  (x1mx2*y1my2) / distance_cube; // v wrt y1
			SIGMA_exp(3,3) = -(x1mx2*y1my2) / distance_cube; // v wrt y2
         sigma_cov = jmath::ublasExtra::prod_JPJt(obsPtrNew->expectation.P() , SIGMA_exp); // sigma_cov = SIGMA_exp * P * SIGMA_exp'
			vec4 sigma = 10*stdevFromCov(sigma_cov); // [sigma_x0 sigma_y0 sigma_u sigma_v]

         JFR_DEBUG("distance_cube\n" << distance_cube << "SIGMA_exp\n" << SIGMA_exp << "sigma_cov \n" << sigma_cov);
std::cout << obsPtrNew->id() << "\nexp " << exp << " P " << stdevFromCov(obsPtrNew->expectation.P()) << "\nSIGMA_exp " << SIGMA_exp << "\nsigma " << sigma << std::endl;

         hypothesis->setUncertainty(sigma(2), sigma(0), sigma(3), sigma(1));

         app_seg_ptr_t app_dst = SPTR_CAST<AppearanceSegment>(obsPtrNew->predictedAppearance);
         app_dst->setHypothesis(hypothesis);

         return true;
      }

      void DescriptorSegFirstView::desc_text(std::ostream& os) const
      {
         os << " of " << typeName() << "; " << view << std::endl;
      }

      void DescriptorSegFirstView::desc_image(image::oimstream& os) const
      {
         os << view << image::endl;
      }

      /***************************************************************************
       * DescriptorSegMultiView
       **************************************************************************/
/*

      DescriptorSegMultiView::DescriptorSegMultiView(int descSize, double scaleStep, double angleStep, PredictionType predictionType):
         DescriptorAbstract(),
         lastObsFailed(false), descSize(descSize), scaleStep(scaleStep), angleStep(angleStep),
         predictionType(predictionType), cosAngleStep(cos(angleStep))
      {
      }

      void DescriptorSegMultiView::addObservation(const observation_ptr_t & obsPtr)
      {
         if (obsPtr->events.updated)
         {
            lastValidView.initFromObs(obsPtr, descSize);
            lastObsFailed = false;
         }
         else if (obsPtr->events.predicted && obsPtr->events.measured && !obsPtr->events.matched)
         {
            lastObsFailed = true;
         }
      }

      bool DescriptorSegMultiView::predictAppearance(const observation_ptr_t & obsPtr)
      {
         SegFeatureView* view_src;
         getClosestView(obsPtr, view_src);
         if (!view_src) return false;

         landmark_ptr_t lmkPtr = obsPtr->landmarkPtr();
         jblas::vec lmk = lmkPtr->reparametrized();

         app_seg_ptr_t app_dst = SPTR_CAST<AppearanceSegment>(obsPtr->predictedAppearance);
         app_seg_ptr_t app_src = SPTR_CAST<AppearanceSegment>(view_src->appearancePtr);

         switch (predictionType)
         {
            case ptNone:
            {
               app_src->patch.robustCopy(app_dst->patch,
                  (app_src->patch.width()-app_dst->patch.width())/2, (app_src->patch.height()-app_dst->patch.height())/2,
                  0, 0, app_dst->patch.width(), app_dst->patch.height());
               app_dst->offset.x()(0) = app_src->offset.x()(0) + ((app_src->patch.width()-app_dst->patch.width())%2) * 0.5;
               app_dst->offset.x()(1) = app_src->offset.x()(1) + ((app_src->patch.height()-app_dst->patch.height())%2) * 0.5;
               app_dst->offset.P() = app_src->offset.P();
            }
            case ptAffine:
            {
               double zoom, rotation;
               quaternion::getZoomRotation(view_src->senPose, obsPtr->sensorPtr()->globalPose(), lmk, zoom, rotation);
               app_src->patch.rotateScale(jmath::radToDeg(rotation), zoom, app_dst->patch);

               double alpha = zoom * cos(rotation);
               double beta  = zoom * sin(rotation);
               app_dst->offset.x()(0) = alpha*app_src->offset.x()(0) +  beta*app_src->offset.x()(1);
               app_dst->offset.x()(1) = -beta*app_src->offset.x()(0) + alpha*app_src->offset.x()(1);
               // this is an approximation for angle, but it's ok
               app_dst->offset.P()(0,0) = alpha*app_src->offset.P()(0,0) +  beta*app_src->offset.P()(1,1);
               app_dst->offset.P()(1,1) = -beta*app_src->offset.P()(0,0) + alpha*app_src->offset.P()(1,1);
            }
            case ptHomographic:
            {
#if 0
// TODO get the 3D corners, rotate them, project them, compute homography, apply homography
               int appw = app_dst->patch.width(), apph = app_dst->patch.height();
               CvSeg2D32f corners_dst[4] = {{0.5,0.5},{appw-0.5,0.5},{0.5,apph-0.5},{appw-0.5,apph-0.5}};

               jblas::vec corners_space[4], corner(2), nobs(1); nobs(0) = ublas::norm_2(obsPtr->landmarkPtr->;

               for(int i = 0; i < 4; ++i)
               {
                  corner(0) = obsPtr->expectation.x()(0) - offset(0); //FIXME check sign offset
                  view_src->model->backProject_func(view_src->senPose, view_src->measurement, nobs, corners_space[i]);

               }





               jblas::vec3 normal_dst = sensorPose3d_dst - featurePose3d_dst;
               normal_dst /= norm_2(normal_dst);
               jblas::vec3 normal_src = sensorPose3d_src - featurePose3d_src;
               normal_src /= norm_2(normal_src);
               jblas::vec3 normal_goal = (normal_dst+normal_src)/2;
               normal_goal /= norm_2(normal_goal);


               // estimate the transformation
               CvSeg2D32f src[4] = {{corners[0][0](0),corners[0][0](1)},{corners[0][1](0),corners[0][1](1)},
                     {corners[1][0](0),corners[1][0](1)},{corners[1][1](0),corners[1][1](1)}};
               CvSeg2D32f dst[4] = {{0,0},{fwd,0},{0,fhd},{fwd,fhd}};
               CvMat *homography = cvCreateMat(3, 3, CV_32F);
               cvGetPerspectiveTransform(src, dst, homography);

               // apply the transformation
               cv::warpPerspective(*featureAppearance_src, *featureAppearance_dst, homography, featureAppearance_dst->size());
               cvReleaseMat(&homography);
#endif
            }
         }

         return true;
      }

      bool DescriptorSegMultiView::isPredictionValid(const observation_ptr_t & obsPtr)
      {
         SegFeatureView* tmp;
         return getClosestView(obsPtr, tmp);
      }

      void DescriptorSegMultiView::checkView(jblas::vec const &current_pov, double const &current_pov_norm2, jblas::vec const &lmk, SegFeatureView &view, double &cosClosestAngle, SegFeatureView* &closestView) const
      {
         jblas::vec stored_pov = lmk - ublas::subrange(view.senPose, 0, 3);
         double stored_pov_norm2 = jmath::sum_sqr(stored_pov(0), stored_pov(1), stored_pov(2));

         double cos_angle = ublas::inner_prod(current_pov, stored_pov) / sqrt(current_pov_norm2*stored_pov_norm2);
         double dist_dist = std::max(current_pov_norm2/stored_pov_norm2, stored_pov_norm2/current_pov_norm2);

         if (dist_dist < scaleStep*scaleStep && cos_angle > cosClosestAngle)
            { closestView = &view; cosClosestAngle = cos_angle; }
         view.used = false;
      }

      bool DescriptorSegMultiView::getClosestView(const observation_ptr_t & obsPtr, SegFeatureView* &closestView)
      {
         // There are two criterias : angle, and distance. Angle is more important because it is more difficult
         // to fix, particularly if you don't know the normal. But if distance is too different, you can't use the view at all.

         // init vars
         closestView = NULL;
         double cosClosestAngle = -1;

         // get info about the current point of view
         landmark_ptr_t lmkPtr = obsPtr->landmarkPtr();
         vec lmk = lmkPtr->reparametrize_func(lmkPtr->state.x());
         jblas::vec current_pov = lmk - ublas::subrange(obsPtr->sensorPtr()->globalPose(), 0, 3); // Point of view
         double current_pov_norm2 = jmath::sum_sqr(current_pov(0), current_pov(1), current_pov(2));

         // check with all the exisiting views
         for(typename SegFeatureViewList::iterator it = views.begin(); it < views.end(); it++)
            checkView(current_pov, current_pov_norm2, lmk, *it, cosClosestAngle, closestView);

         // if we can't have a good view, we add the last one and try with it
         if ((!closestView || (cosClosestAngle < cosAngleStep && lastObsFailed)) && lastValidView.appearancePtr)
         {
            checkView(current_pov, current_pov_norm2, lmk, lastValidView, cosClosestAngle, closestView);
            if (closestView == &lastValidView)
            {
               views.push_back(lastValidView);
               lastValidView.clear();
               closestView = &(views.back());
               JFR_DEBUG("Descriptor of landmark " << obsPtr->id() << " stored a new view ; it has " << views.size() << " of them now");
            }
         }

         // return final result
         if (cosClosestAngle < -0.5) closestView = NULL; // really not usable
         if (closestView) closestView->used = true;
         return (closestView && cosClosestAngle >= cosAngleStep);
      }

      void DescriptorSegMultiView::desc_text(std::ostream& os) const
      {
         os << " of " << typeName() << "; " << views.size() << " view(s):";
         for(SegFeatureViewList::const_iterator it = views.begin(); it != views.end(); ++it)
            os << std::endl << *it;
      }

      void DescriptorSegMultiView::desc_image(image::oimstream& os) const
      {
         int nviews = views.size();
         if (nviews == 0) return;
         int nvy = (int)(sqrt((double)nviews));
         int nvx = nviews/nvy; if (nvx*nvy < nviews) nvx++;

         SegFeatureViewList::const_iterator it = views.begin();
         for(int x = 0, y = 0; it != views.end(); ++it, ++x)
         {
            if (x >= nvx) { x = 0; ++y; os << image::endl; }
            os << *it;
         }
         os << image::endl;
      }

*/
   }
}

#endif //HAVE_MODULE_DSEG
