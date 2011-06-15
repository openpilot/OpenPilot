/*
 * descriptorImageSeg.cpp
 *
 *  Created on: 24 feb. 2011
 *      Author: bhautboi@laas.fr
 */

#ifdef HAVE_MODULE_DSEG

#include "jmath/ublasExtra.hpp"
#include "jmath/angle.hpp"

#include "rtslam/descriptorImageSeg.hpp"
#include "rtslam/rawImage.hpp"
#include "rtslam/quatTools.hpp"

namespace jafar {
   namespace rtslam {
      using namespace ublasExtra;
      using namespace quaternion;

      std::ostream& operator <<(std::ostream & s, ImgSegFeatureView const & fv)
      {
         app_img_seg_ptr_t app = SPTR_CAST<AppearanceImageSegment>(fv.appearancePtr);
			jblas::vec P1(2); P1(0) = sqrt(app->offsetTop.P()(0,0)); P1(1) = sqrt(app->offsetTop.P()(1,1));
			jblas::vec P2(2); P2(0) = sqrt(app->offsetBottom.P()(0,0)); P2(1) = sqrt(app->offsetBottom.P()(1,1));
			s << "  -" << (fv.used ? " [*] " : " ") << "at frame " << fv.frame <<
            " by sensor " << fv.obsModelPtr->sensorPtr()->id() << " of " <<
            fv.obsModelPtr->sensorPtr()->typeName() << " at " << fv.senPose <<
				", with offset " << app->offsetTop.x() << " +- " << P1 <<
				", with offset " << app->offsetBottom.x() << " +- " << P2;
			return s;
      }

      image::oimstream& operator <<(image::oimstream & s, ImgSegFeatureView const & fv)
      {
         app_img_seg_ptr_t app = SPTR_CAST<AppearanceImageSegment>(fv.appearancePtr);
         s << app->patch;
         return s;
      }

      /***************************************************************************
       * ImgSegFeatureView
       **************************************************************************/

      void ImgSegFeatureView::initFromObs(const observation_ptr_t & obsPtr, int descSize)
      {
			app_img_seg_ptr_t obsApp = SPTR_CAST<AppearanceImageSegment>(obsPtr->observedAppearance);
			app_img_seg_ptr_t app(new AppearanceImageSegment(descSize, descSize, CV_8U,obsApp->hypothesis()));
				 sensorext_ptr_t senPtr = SPTR_CAST<SensorExteroAbstract>(obsPtr->sensorPtr());
         rawimage_ptr_t rawPtr = SPTR_CAST<RawImage>(senPtr->rawPtr);

			// Compute dimensions
			int x1 = min(obsPtr->measurement.x()(0), obsPtr->measurement.x()(2)) - descSize/2;
			int x2 = max(obsPtr->measurement.x()(0), obsPtr->measurement.x()(2)) + descSize/2;
			int y1 = min(obsPtr->measurement.x()(1), obsPtr->measurement.x()(3)) - descSize/2;
			int y2 = max(obsPtr->measurement.x()(1), obsPtr->measurement.x()(3)) + descSize/2;
			x1 = max(x1,0);
			x2 = min(x2,rawPtr->img->width() - 1);
			y1 = max(y1,0);
			y2 = min(y2,rawPtr->img->height() - 1);
			if (rawPtr->img->extractPatch(app->patch, (x1+x2)/2, (y1+y2)/2, x2-x1-2, y2-y1-2))
			{
//				Image patch(descSize,descSize + sqrt(sqr(x2-x1)+sqr(y2-y1)),rawPtr->img->depth(), rawPtr->img->colorSpace());
//				float rotation = atan2(y2-y1,x2-x1);
//				app->patch.rotateScale(-jmath::radToDeg(rotation), 1, patch);
//				patch.copyTo(app->patch);

				vec pix = obsPtr->measurement.x();
				vec2 center;
				center[0] = ( pix[0] + pix[2] )/2;
				center[1] = ( pix[1] + pix[3] )/2;

				app->offsetTop.x()(0) = pix(0) - (x1+x2)/2;
				app->offsetTop.x()(1) = pix(1) - (y1+y2)/2;
				app->offsetTop.P() = jblas::zero_mat(2); // by definition this is our landmark projection

				app->offsetBottom.x()(0) = pix(2) - (x1+x2)/2;
				app->offsetBottom.x()(1) = pix(3) - (y1+y2)/2;
				app->offsetBottom.P() = jblas::zero_mat(2); // by definition this is our landmark projection

				app->patchMeanLeft = obsApp->patchMeanLeft;
				app->patchMeanRight = obsApp->patchMeanRight;

				appearancePtr = app;

				senPose = obsPtr->sensorPtr()->globalPose();
				obsModelPtr = obsPtr->model;
				measurement = obsPtr->measurement.x();
				frame = senPtr->rawCounter;
				used = false;
         }
      }


      /***************************************************************************
       * DescriptorImageSegFirstView
       **************************************************************************/

      DescriptorImageSegFirstView::DescriptorImageSegFirstView(int descSize):
         DescriptorAbstract(),
         descSize(descSize)
      {
      }

      DescriptorImageSegFirstView::~DescriptorImageSegFirstView() {
      }

			bool DescriptorImageSegFirstView::addObservation(const observation_ptr_t & obsPtr)
			{
				if (obsPtr->events.updated && !view.appearancePtr)
				{
					view.initFromObs(obsPtr, descSize);
					return true;
				} else
					return false;
			}


      bool DescriptorImageSegFirstView::predictAppearance(const observation_ptr_t & obsPtrNew) {
         double zoom, rotation;
         landmark_ptr_t lmkPtr = obsPtrNew->landmarkPtr();
         //vec seg = lmkPtr->reparametrized(); // Dummy, we reparametrize the first point of the segment for the zoomRotation
         vec3 seg;
         seg[0] = 0;
         seg[1] = 0;
         seg[2] = 0;
         quaternion::getZoomRotation(view.senPose, obsPtrNew->sensorPtr()->globalPose(), seg, zoom, rotation);
         // normally we must cast to the derived type
         app_img_seg_ptr_t app_dst = SPTR_CAST<AppearanceImageSegment>(obsPtrNew->predictedAppearance);
         app_img_seg_ptr_t app_src = SPTR_CAST<AppearanceImageSegment>(view.appearancePtr);

         // rotate and zoom the patch, and cut it to the appropriate size
         app_src->patch.rotateScale(jmath::radToDeg(rotation), zoom, app_dst->patch);

         double alpha = zoom * cos(rotation);
         double beta  = zoom * sin(rotation);
			app_dst->offsetTop.x()(0) = alpha*app_src->offsetTop.x()(0) +  beta*app_src->offsetTop.x()(1);
			app_dst->offsetTop.x()(1) = -beta*app_src->offsetTop.x()(0) + alpha*app_src->offsetTop.x()(1);
         // this is an approximation for angle, but it's ok
			app_dst->offsetTop.P()(0,0) = alpha*app_src->offsetTop.P()(0,0) +  beta*app_src->offsetTop.P()(1,1);
			app_dst->offsetTop.P()(1,1) = -beta*app_src->offsetTop.P()(0,0) + alpha*app_src->offsetTop.P()(1,1);

			app_dst->offsetBottom.x()(0) = alpha*app_src->offsetBottom.x()(0) +  beta*app_src->offsetBottom.x()(1);
			app_dst->offsetBottom.x()(1) = -beta*app_src->offsetBottom.x()(0) + alpha*app_src->offsetBottom.x()(1);
			// this is an approximation for angle, but it's ok
			app_dst->offsetBottom.P()(0,0) = alpha*app_src->offsetBottom.P()(0,0) +  beta*app_src->offsetBottom.P()(1,1);
			app_dst->offsetBottom.P()(1,1) = -beta*app_src->offsetBottom.P()(0,0) + alpha*app_src->offsetBottom.P()(1,1);

/*char buffer[256];
sprintf(buffer, "descriptor_patch_%03d.png", obsPtrNew->id());
app_src->patch.save(buffer);
JFR_DEBUG("predict with desc " << this << " and view " << &view << " and app " << view.appearancePtr.get() << " and patch " << &(app_src->patch));
sprintf(buffer, "predicted_patch_%03d.png", obsPtrNew->id());
app_dst->patch.save(buffer);
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
			vec4 sigma = 2*stdevFromCov(sigma_cov); // [sigma_x0 sigma_y0 sigma_u sigma_v]

//			JFR_DEBUG("distance_cube\n" << distance_cube << "SIGMA_exp\n" << SIGMA_exp << "sigma_cov \n" << sigma_cov);
//			std::cout << obsPtrNew->id() << "\nexp " << exp << " P " << stdevFromCov(obsPtrNew->expectation.P()) << "\nSIGMA_exp " << SIGMA_exp << "\nsigma " << sigma << std::endl;

			app_dst->patchMeanLeft = app_src->patchMeanLeft;
			app_dst->patchMeanRight = app_src->patchMeanRight;

			hypothesis->setUncertainty(sigma(2), sigma(0), sigma(3), sigma(1));
			app_dst->setHypothesis(hypothesis);

			return true;
      }

      void DescriptorImageSegFirstView::desc_text(std::ostream& os) const
      {
         os << " of " << typeName() << "; " << view << std::endl;
      }

      void DescriptorImageSegFirstView::desc_image(image::oimstream& os) const
      {
         os << view << image::endl;
      }

      /***************************************************************************
       * DescriptorImageSegMultiView
       **************************************************************************/


      DescriptorImageSegMultiView::DescriptorImageSegMultiView(int descSize, double scaleStep, double angleStep, PredictionType predictionType):
         DescriptorAbstract(),
         lastObsFailed(false), descSize(descSize), scaleStep(scaleStep), angleStep(angleStep),
         predictionType(predictionType), cosAngleStep(cos(angleStep))
      {
      }

			bool DescriptorImageSegMultiView::addObservation(const observation_ptr_t & obsPtr)
			{
				if (obsPtr->events.updated)
				{
					lastValidView.initFromObs(obsPtr, descSize);
					lastObsFailed = false;
					return true;
				}
				else if (obsPtr->events.predicted && obsPtr->events.measured && !obsPtr->events.matched)
				{
					lastObsFailed = true;
					return false;
				}
				return false;
			}

      bool DescriptorImageSegMultiView::predictAppearance(const observation_ptr_t & obsPtr)
      {
         ImgSegFeatureView* view_src;
         getClosestView(obsPtr, view_src);
         if (!view_src) return false;

         landmark_ptr_t lmkPtr = obsPtr->landmarkPtr();
         jblas::vec lmk = lmkPtr->reparametrized();

         app_img_seg_ptr_t app_dst = SPTR_CAST<AppearanceImageSegment>(obsPtr->predictedAppearance);
         app_img_seg_ptr_t app_src = SPTR_CAST<AppearanceImageSegment>(view_src->appearancePtr);

         switch (predictionType)
         {
            case ptNone:
            {
               app_src->patch.robustCopy(app_dst->patch,
                  (app_src->patch.width()-app_dst->patch.width())/2, (app_src->patch.height()-app_dst->patch.height())/2,
                  0, 0, app_dst->patch.width(), app_dst->patch.height());
					app_dst->offsetTop.x()(0) = app_src->offsetTop.x()(0) + ((app_src->patch.width()-app_dst->patch.width())%2) * 0.5;
					app_dst->offsetTop.x()(1) = app_src->offsetTop.x()(1) + ((app_src->patch.height()-app_dst->patch.height())%2) * 0.5;
					app_dst->offsetBottom.x()(0) = app_src->offsetBottom.x()(0) + ((app_src->patch.width()-app_dst->patch.width())%2) * 0.5;
					app_dst->offsetBottom.x()(1) = app_src->offsetBottom.x()(1) + ((app_src->patch.height()-app_dst->patch.height())%2) * 0.5;
					app_dst->offsetTop.P() = app_src->offsetTop.P();
					app_dst->offsetBottom.P() = app_src->offsetBottom.P();
					app_dst->patchMeanLeft = app_src->patchMeanLeft;
					app_dst->patchMeanRight = app_src->patchMeanRight;
				}
            case ptAffine:
            {
               double zoom, rotation;
               quaternion::getZoomRotation(view_src->senPose, obsPtr->sensorPtr()->globalPose(), lmk, zoom, rotation);
               app_src->patch.rotateScale(jmath::radToDeg(rotation), zoom, app_dst->patch);

               double alpha = zoom * cos(rotation);
               double beta  = zoom * sin(rotation);
					app_dst->offsetTop.x()(0) = alpha*app_src->offsetTop.x()(0) +  beta*app_src->offsetTop.x()(1);
					app_dst->offsetTop.x()(1) = -beta*app_src->offsetTop.x()(0) + alpha*app_src->offsetTop.x()(1);
					app_dst->offsetBottom.x()(0) = alpha*app_src->offsetBottom.x()(0) +  beta*app_src->offsetBottom.x()(1);
					app_dst->offsetBottom.x()(1) = -beta*app_src->offsetBottom.x()(0) + alpha*app_src->offsetBottom.x()(1);
					// this is an approximation for angle, but it's ok
					app_dst->offsetTop.P()(0,0) = alpha*app_src->offsetTop.P()(0,0) +  beta*app_src->offsetTop.P()(1,1);
					app_dst->offsetTop.P()(1,1) = -beta*app_src->offsetTop.P()(0,0) + alpha*app_src->offsetTop.P()(1,1);
					app_dst->offsetBottom.P()(0,0) = alpha*app_src->offsetBottom.P()(0,0) +  beta*app_src->offsetBottom.P()(1,1);
					app_dst->offsetBottom.P()(1,1) = -beta*app_src->offsetBottom.P()(0,0) + alpha*app_src->offsetBottom.P()(1,1);
					app_dst->patchMeanLeft = app_src->patchMeanLeft;
					app_dst->patchMeanRight = app_src->patchMeanRight;
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

      bool DescriptorImageSegMultiView::isPredictionValid(const observation_ptr_t & obsPtr)
      {
         ImgSegFeatureView* tmp;
         return getClosestView(obsPtr, tmp);
      }

      void DescriptorImageSegMultiView::checkView(jblas::vec const &current_pov, double const &current_pov_norm2, jblas::vec const &lmk, ImgSegFeatureView &view, double &cosClosestAngle, ImgSegFeatureView* &closestView) const
      {
         jblas::vec stored_pov = lmk - ublas::subrange(view.senPose, 0, 3);
         double stored_pov_norm2 = jmath::sum_sqr(stored_pov(0), stored_pov(1), stored_pov(2));

         double cos_angle = ublas::inner_prod(current_pov, stored_pov) / sqrt(current_pov_norm2*stored_pov_norm2);
         double dist_dist = std::max(current_pov_norm2/stored_pov_norm2, stored_pov_norm2/current_pov_norm2);

         if (dist_dist < scaleStep*scaleStep && cos_angle > cosClosestAngle)
            { closestView = &view; cosClosestAngle = cos_angle; }
         view.used = false;
      }

      bool DescriptorImageSegMultiView::getClosestView(const observation_ptr_t & obsPtr, ImgSegFeatureView* &closestView)
      {
         /*
         There are two criterias : angle, and distance. Angle is more important because it is more difficult
         to fix, particularly if you don't know the normal. But if distance is too different, you can't use the view at all.
         */

         // init vars
         closestView = NULL;
         double cosClosestAngle = -1;

         // get info about the current point of view
         landmark_ptr_t lmkPtr = obsPtr->landmarkPtr();
         vec lmk = lmkPtr->reparametrize_func(lmkPtr->state.x());
         jblas::vec current_pov = lmk - ublas::subrange(obsPtr->sensorPtr()->globalPose(), 0, 3); // Point of view
         double current_pov_norm2 = jmath::sum_sqr(current_pov(0), current_pov(1), current_pov(2));

         // check with all the exisiting views
         for(ImgSegFeatureViewList::iterator it = views.begin(); it < views.end(); it++)
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

      void DescriptorImageSegMultiView::desc_text(std::ostream& os) const
      {
         os << " of " << typeName() << "; " << views.size() << " view(s):";
         for(ImgSegFeatureViewList::const_iterator it = views.begin(); it != views.end(); ++it)
            os << std::endl << *it;
      }

      void DescriptorImageSegMultiView::desc_image(image::oimstream& os) const
      {
         int nviews = views.size();
         if (nviews == 0) return;
         int nvy = (int)(sqrt((double)nviews));
         int nvx = nviews/nvy; if (nvx*nvy < nviews) nvx++;

         ImgSegFeatureViewList::const_iterator it = views.begin();
         for(int x = 0, y = 0; it != views.end(); ++it, ++x)
         {
            if (x >= nvx) { x = 0; ++y; os << image::endl; }
            os << *it;
         }
         os << image::endl;
      }

   }
}

#endif /* HAVE_MODULE_DSEG */
