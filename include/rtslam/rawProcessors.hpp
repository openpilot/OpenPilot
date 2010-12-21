/**
 * \file rawProcessors.hpp
 *
 *  Some wrappers to raw processors to be used generically
 *
 * \date 20/07/2010
 * \author croussil@laas.fr
 *
 * ## Add detailed description here ##
 *
 * \ingroup rtslam
 */

#ifndef RAWPROCESSORS_HPP_
#define RAWPROCESSORS_HPP_


#include "correl/explorer.hpp"
#include "rtslam/quickHarrisDetector.hpp"


#include "rtslam/rawImage.hpp"
#include "rtslam/sensorPinHole.hpp"
#include "rtslam/descriptorImagePoint.hpp"

// TODO simu

namespace jafar {
namespace rtslam {

	class ImagePointZnccMatcher
	{
		private:
			correl::FastTranslationMatcherZncc matcher;
		
		public:
			struct matcher_params_t {
				int patchSize;
				int maxSearchSize;
				double lowInnov;      ///<     search region radius for first RANSAC consensus
				double threshold;     ///<     matching threshold
				double mahalanobisTh; ///< Mahalanobis distance for outlier rejection
				double measStd;       ///<       measurement noise std deviation
				double measVar;       ///<       measurement noise variance
			} params;

		public:
			ImagePointZnccMatcher(double minScore, double partialPosition, int patchSize, int maxSearchSize, double lowInnov, double threshold, double mahalanobisTh, double measStd):
				matcher(minScore, partialPosition)
			{
				params.patchSize = patchSize;
				params.maxSearchSize = maxSearchSize;
				params.lowInnov = lowInnov;
				params.threshold = threshold;
				params.mahalanobisTh = mahalanobisTh;
				params.measStd = measStd;
				params.measVar = measStd * measStd;
			}

			void match(const boost::shared_ptr<RawImage> & rawPtr, const appearance_ptr_t & targetApp, const image::ConvexRoi & roi, Measurement & measure, appearance_ptr_t & app)
			{
				app_img_pnt_ptr_t targetAppSpec = SPTR_CAST<AppearanceImagePoint>(targetApp);
				app_img_pnt_ptr_t appSpec = SPTR_CAST<AppearanceImagePoint>(app);
				
				measure.std(params.measStd);
				measure.matchScore = matcher.match(targetAppSpec->patch, *(rawPtr->img),
					roi, measure.x()(0), measure.x()(1));
				measure.x() += targetAppSpec->offset;
				rawPtr->img->robustCopy(appSpec->patch, (int)(measure.x()(0))-appSpec->patch.width()/2,
					(int)(measure.x()(1))-appSpec->patch.height()/2, 0, 0, appSpec->patch.width(), appSpec->patch.height());
				appSpec->offset(0) = measure.x()(0) - ((int)(measure.x()(0)) + 0.5);
				appSpec->offset(1) = measure.x()(1) - ((int)(measure.x()(1)) + 0.5);
			}
	};

	/*
	*/

	class ImagePointHarrisDetector
	{
		private:
			QuickHarrisDetector detector;
			boost::shared_ptr<DescriptorImagePointMultiViewFactory> descFactory;

		public:
			struct detector_params_t {
				int patchSize;  ///<       descriptor patch size
				double measStd; ///<       measurement noise std deviation
				double measVar; ///<       measurement noise variance
			} params;
			
		public:
			ImagePointHarrisDetector(double convSize, double thres, double edge, int patchSize, double measStd,
				boost::shared_ptr<DescriptorImagePointMultiViewFactory> const &descFactory):
				detector(convSize, thres, edge), descFactory(descFactory)
			{
				params.patchSize = patchSize;
				params.measStd = measStd;
				params.measVar = measStd * measStd;
			}

			bool detect(const boost::shared_ptr<RawImage> & rawData, const image::ConvexRoi &roi, boost::shared_ptr<FeatureImagePoint> & featPtr)
			{
				featPtr.reset(new FeatureImagePoint(params.patchSize, params.patchSize, CV_8U));
				featPtr->measurement.std(params.measStd);
				if (detector.detectIn(*(rawData->img.get()), featPtr, &roi))
				{
					// extract appearance
					vec pix = featPtr->measurement.x();
					boost::shared_ptr<AppearanceImagePoint> appPtr = SPTR_CAST<AppearanceImagePoint>(featPtr->appearancePtr);
					//appPtr->patch.resize(params.patchSize, params.patchSize);
					//cv::Size size = appPtr->patch.size();
					cv::Size size(params.patchSize, params.patchSize);
					
					int shift_x = (size.width-1)/2;
					int shift_y = (size.height-1)/2;
					int x_src = (int)pix(0)-shift_x;
					int y_src = (int)pix(1)-shift_y;
					rawData->img->copy(appPtr->patch, x_src, y_src, 0, 0, size.width, size.height);
					appPtr->offset(0) = pix(0) - ((int)pix(0) + 0.5);
					appPtr->offset(1) = pix(1) - ((int)pix(1) + 0.5);

					return true;
				} else return false;
			}
			
			void fillDataObs(const boost::shared_ptr<FeatureImagePoint> & featPtr, boost::shared_ptr<ObservationAbstract> & obsPtr)
			{
				// extract observed appearance
				app_img_pnt_ptr_t app_src = SPTR_CAST<AppearanceImagePoint>(featPtr->appearancePtr);
				app_img_pnt_ptr_t app_dst = SPTR_CAST<AppearanceImagePoint>(obsPtr->observedAppearance);
				app_src->patch.copy(app_dst->patch, (app_src->patch.width()-app_dst->patch.width())/2,
						(app_src->patch.height()-app_dst->patch.height())/2, 0, 0,
						app_dst->patch.width(), app_dst->patch.height());
				app_dst->offset = app_src->offset;

				// create descriptor
				vec7 globalSensorPose = obsPtr->sensorPtr()->globalPose();
				desc_img_pnt_mv_ptr_t descPtr(descFactory->createDescriptor());
//				desc_img_pnt_fv_ptr_t descPtr(new DescriptorImagePointFirstView());
				descPtr->addObservation(obsPtr);
				obsPtr->landmarkPtr()->setDescriptor(descPtr);
			}

	};

}}

#endif
