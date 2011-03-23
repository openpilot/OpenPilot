/*
 * \file descriptorImagePoint.cpp
 * \date 20/04/2010
 * \author jmcodol
 * \ingroup rtslam
 */

#include "jmath/ublasExtra.hpp"
#include "jmath/angle.hpp"

#include "rtslam/descriptorImagePoint.hpp"
#include "rtslam/rawImage.hpp"
#include "rtslam/quatTools.hpp"

namespace jafar {
	namespace rtslam {
		using namespace ublasExtra;
		using namespace quaternion;

		std::ostream& operator <<(std::ostream & s, FeatureView const & fv)
		{
			app_img_pnt_ptr_t app = SPTR_CAST<AppearanceImagePoint>(fv.appearancePtr);
			jblas::vec P(2); P(0) = sqrt(app->offset.P()(0,0)); P(1) = sqrt(app->offset.P()(1,1));
			s << "  -" << (fv.used ? " [*] " : " ") << "at frame " << fv.frame << 
				" by sensor " << fv.obsModelPtr->sensorPtr()->id() << " of " << 
				fv.obsModelPtr->sensorPtr()->typeName() << " at " << fv.senPose <<
				", with offset " << app->offset.x() << " +- " << P;
			return s;
		}

		image::oimstream& operator <<(image::oimstream & s, FeatureView const & fv)
		{
			app_img_pnt_ptr_t app = SPTR_CAST<AppearanceImagePoint>(fv.appearancePtr);
			s << app->patch;
			return s;
		}
		
		/***************************************************************************
		 * FeatureView
		 **************************************************************************/
		
		bool FeatureView::initFromObs(const observation_ptr_t & obsPtr, int descSize)
		{
			app_img_pnt_ptr_t app(new AppearanceImagePoint(descSize, descSize, CV_8U));
			sensorext_ptr_t senPtr = SPTR_CAST<SensorExteroAbstract>(obsPtr->sensorPtr());
			rawimage_ptr_t rawPtr = SPTR_CAST<RawImage>(senPtr->rawPtr);
			if (rawPtr->img->extractPatch(app->patch, (int)obsPtr->measurement.x()(0), (int)obsPtr->measurement.x()(1), descSize, descSize))
			{
				app_img_pnt_ptr_t obsApp = SPTR_CAST<AppearanceImagePoint>(obsPtr->observedAppearance);
				app->offset = obsApp->offset;
				appearancePtr = app;

				senPose = senPtr->globalPose();
				obsModelPtr = obsPtr->model;
				measurement = obsPtr->measurement.x();
				frame = senPtr->rawCounter;
				used = false;
				return true;
			} else
				return false;
		}
		
		
		/***************************************************************************
		 * DescriptorImagePointFirstView
		 **************************************************************************/

		DescriptorImagePointFirstView::DescriptorImagePointFirstView(int descSize):
			DescriptorAbstract(),
			descSize(descSize)
		{
		}

		DescriptorImagePointFirstView::~DescriptorImagePointFirstView() {
		}

		bool DescriptorImagePointFirstView::addObservation(const observation_ptr_t & obsPtr)
		{
			if (obsPtr->events.updated && !view.appearancePtr)
				return view.initFromObs(obsPtr, descSize);
			else return false;
		}


		bool DescriptorImagePointFirstView::predictAppearance(const observation_ptr_t & obsPtrNew) {

			double zoom, rotation;
			landmark_ptr_t lmkPtr = obsPtrNew->landmarkPtr();
			vec pnt = lmkPtr->reparametrized();
			quaternion::getZoomRotation(view.senPose, obsPtrNew->sensorPtr()->globalPose(), pnt, zoom, rotation);
			// normally we must cast to the derived type
			app_img_pnt_ptr_t app_dst = SPTR_CAST<AppearanceImagePoint>(obsPtrNew->predictedAppearance);
			app_img_pnt_ptr_t app_src = SPTR_CAST<AppearanceImagePoint>(view.appearancePtr);
			// rotate and zoom the patch, and cut it to the appropriate size
			app_src->patch.rotateScale(jmath::radToDeg(rotation), zoom, app_dst->patch);
			
			double alpha = zoom * cos(rotation);
			double beta  = zoom * sin(rotation);
			app_dst->offset.x()(0) = alpha*app_src->offset.x()(0) +  beta*app_src->offset.x()(1);
			app_dst->offset.x()(1) = -beta*app_src->offset.x()(0) + alpha*app_src->offset.x()(1);
			// this is an approximation for angle, but it's ok
			app_dst->offset.P()(0,0) = alpha*app_src->offset.P()(0,0) +  beta*app_src->offset.P()(1,1);
			app_dst->offset.P()(1,1) = -beta*app_src->offset.P()(0,0) + alpha*app_src->offset.P()(1,1);
			
/*char buffer[256];
sprintf(buffer, "descriptor_patch_%03d.png", obsPtrNew->id());
app_src->patch.save(buffer);
JFR_DEBUG("predict with desc " << this << " and view " << &view << " and app " << view.appearancePtr.get() << " and patch " << &(app_src->patch));
sprintf(buffer, "predicted_patch_%03d.png", obsPtrNew->id());
app_dst->patch.save(buffer);
*/
			return true;
		}

		void DescriptorImagePointFirstView::desc_text(std::ostream& os) const
		{
			os << " of " << typeName() << "; " << view << std::endl;
		}

		void DescriptorImagePointFirstView::desc_image(image::oimstream& os) const
		{
			os << view << image::endl;
		}
		
		/***************************************************************************
		 * DescriptorImagePointMultiView
		 **************************************************************************/


		DescriptorImagePointMultiView::DescriptorImagePointMultiView(int descSize, double scaleStep, double angleStep, PredictionType predictionType):
			DescriptorAbstract(),
			lastObsFailed(false), descSize(descSize), scaleStep(scaleStep), angleStep(angleStep),
			predictionType(predictionType), cosAngleStep(cos(angleStep))
		{
		}
		
		bool DescriptorImagePointMultiView::addObservation(const observation_ptr_t & obsPtr)
		{
			if (obsPtr->events.updated)
			{
				int res = lastValidView.initFromObs(obsPtr, descSize);
				lastObsFailed = false;
				return res;
			}
			else if (obsPtr->events.predicted && obsPtr->events.measured && !obsPtr->events.matched)
			{
				lastObsFailed = true;
				return false;
			}
			return false;
		}
		
		bool DescriptorImagePointMultiView::predictAppearance(const observation_ptr_t & obsPtr)
		{
			FeatureView* view_src;
			getClosestView(obsPtr, view_src);
			if (!view_src) return false;

			landmark_ptr_t lmkPtr = obsPtr->landmarkPtr();
			jblas::vec lmk = lmkPtr->reparametrized();
			
			app_img_pnt_ptr_t app_dst = SPTR_CAST<AppearanceImagePoint>(obsPtr->predictedAppearance);
			app_img_pnt_ptr_t app_src = SPTR_CAST<AppearanceImagePoint>(view_src->appearancePtr);
			
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
					CvPoint2D32f corners_dst[4] = {{0.5,0.5},{appw-0.5,0.5},{0.5,apph-0.5},{appw-0.5,apph-0.5}};
					
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
					CvPoint2D32f src[4] = {{corners[0][0](0),corners[0][0](1)},{corners[0][1](0),corners[0][1](1)},
							{corners[1][0](0),corners[1][0](1)},{corners[1][1](0),corners[1][1](1)}};
					CvPoint2D32f dst[4] = {{0,0},{fwd,0},{0,fhd},{fwd,fhd}};
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
		
		bool DescriptorImagePointMultiView::isPredictionValid(const observation_ptr_t & obsPtr)
		{
			FeatureView* tmp;
			return getClosestView(obsPtr, tmp);
		}

		void DescriptorImagePointMultiView::checkView(jblas::vec const &current_pov, double const &current_pov_norm2, jblas::vec const &lmk, FeatureView &view, double &cosClosestAngle, FeatureView* &closestView) const
		{
			jblas::vec stored_pov = lmk - ublas::subrange(view.senPose, 0, 3);
			double stored_pov_norm2 = jmath::sum_sqr(stored_pov(0), stored_pov(1), stored_pov(2));
	
			double cos_angle = ublas::inner_prod(current_pov, stored_pov) / sqrt(current_pov_norm2*stored_pov_norm2);
			double dist_dist = std::max(current_pov_norm2/stored_pov_norm2, stored_pov_norm2/current_pov_norm2);
			
			if (dist_dist < scaleStep*scaleStep && cos_angle > cosClosestAngle)
				{ closestView = &view; cosClosestAngle = cos_angle; }
			view.used = false;
		}

		bool DescriptorImagePointMultiView::getClosestView(const observation_ptr_t & obsPtr, FeatureView* &closestView)
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
			jblas::vec current_pov = lmk - ublas::subrange(obsPtr->sensorPtr()->globalPose(), 0, 3); // point of view
			double current_pov_norm2 = jmath::sum_sqr(current_pov(0), current_pov(1), current_pov(2));
			
			// check with all the exisiting views
			for(FeatureViewList::iterator it = views.begin(); it < views.end(); it++)
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

		void DescriptorImagePointMultiView::desc_text(std::ostream& os) const
		{
			os << " of " << typeName() << "; " << views.size() << " view(s):";
			for(FeatureViewList::const_iterator it = views.begin(); it != views.end(); ++it)
				os << std::endl << *it;
		}

		void DescriptorImagePointMultiView::desc_image(image::oimstream& os) const
		{
			int nviews = views.size();
			if (nviews == 0) return;
			int nvy = (int)(sqrt((double)nviews));
			int nvx = nviews/nvy; if (nvx*nvy < nviews) nvx++;
				
			FeatureViewList::const_iterator it = views.begin();
			for(int x = 0, y = 0; it != views.end(); ++it, ++x)
			{
				if (x >= nvx) { x = 0; ++y; os << image::endl; }
				os << *it;
			}
			os << image::endl;
		}

	}
}
