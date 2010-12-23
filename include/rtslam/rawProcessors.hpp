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
				JFR_ASSERT(patchSize%2, "patchSize must be an odd number!");
				JFR_ASSERT(minScore>=0.0 && minScore<=1, "minScore must be between 0 and 1!");
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
				measure.x() += targetAppSpec->offset.x();
				measure.P() += targetAppSpec->offset.P(); // no cross terms
				rawPtr->img->extractPatch(appSpec->patch, (int)measure.x()(0), (int)measure.x()(1), appSpec->patch.width(), appSpec->patch.height());
				appSpec->offset.x()(0) = measure.x()(0) - ((int)(measure.x()(0)) + 0.5);
				appSpec->offset.x()(1) = measure.x()(1) - ((int)(measure.x()(1)) + 0.5);
				appSpec->offset.P() = measure.P();
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
			ImagePointHarrisDetector(int convSize, double thres, double edge, int patchSize, double measStd,
				boost::shared_ptr<DescriptorImagePointMultiViewFactory> const &descFactory):
				detector(convSize, thres, edge), descFactory(descFactory)
			{
				JFR_ASSERT(convSize%2, "convSize must be an odd number!");
				JFR_ASSERT(patchSize%2, "patchSize must be an odd number!");
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
					rawData->img->extractPatch(appPtr->patch, (int)pix(0), (int)pix(1), params.patchSize, params.patchSize);
					appPtr->offset.x()(0) = pix(0) - ((int)pix(0) + 0.5);
					appPtr->offset.x()(1) = pix(1) - ((int)pix(1) + 0.5);
					appPtr->offset.P() = jblas::zero_mat(2); // by definition this is our landmark projection

					return true;
				} else return false;
			}
			
			void fillDataObs(const boost::shared_ptr<FeatureImagePoint> & featPtr, boost::shared_ptr<ObservationAbstract> & obsPtr)
			{
				// extract observed appearance
				app_img_pnt_ptr_t app_src = SPTR_CAST<AppearanceImagePoint>(featPtr->appearancePtr);
				app_img_pnt_ptr_t app_dst = SPTR_CAST<AppearanceImagePoint>(obsPtr->observedAppearance);
				app_src->patch.copyTo(app_dst->patch);
				/*app_src->patch.copy(app_dst->patch, (app_src->patch.width()-app_dst->patch.width())/2,
						(app_src->patch.height()-app_dst->patch.height())/2, 0, 0,
						app_dst->patch.width(), app_dst->patch.height());*/
				app_dst->offset = app_src->offset;

				// create descriptor
				vec7 globalSensorPose = obsPtr->sensorPtr()->globalPose();
				descriptor_ptr_t descPtr(descFactory->createDescriptor());
				descPtr->addObservation(obsPtr);
				obsPtr->landmarkPtr()->setDescriptor(descPtr);
			}

	};

}}

#endif
