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

#ifndef RAWSEGPROCESSORS_HPP
#define RAWSEGPROCESSORS_HPP


#include "dseg/DirectSegmentsTracker.hpp"
#include "dseg/ConstantVelocityPredictor.hpp"
#include "rtslam/hierarchicalDirectSegmentDetector.hpp"

#include "rtslam/rawImage.hpp"
#include "rtslam/sensorPinHole.hpp"
#include "rtslam/descriptorImageSeg.hpp"

namespace jafar {
namespace rtslam {

   class DsegMatcher
   {
      private:
         dseg::DirectSegmentsTracker matcher;
         dseg::ConstantVelocityPredictor predictor;

      public:
         struct matcher_params_t {
            // RANSAC
            int maxSearchSize;
            double lowInnov;      ///<     search region radius for first RANSAC consensus
            double threshold;     ///<     matching threshold
            double mahalanobisTh; ///< Mahalanobis distance for outlier rejection
            double relevanceTh; ///< Mahalanobis distance for no information rejection
            double measStd;       ///<       measurement noise std deviation
            double measVar;       ///<       measurement noise variance
            // DSEG
            double translationError;
            double rotationError;
         } params;

      public:
         DsegMatcher(double translationError, double rotationError):
            matcher(), predictor(translationError,rotationError)
         {
            params.translationError = translationError;
            params.rotationError = rotationError;
         }

         void match(const boost::shared_ptr<RawImage> & rawPtr, const appearance_ptr_t & targetApp, const image::ConvexRoi & roi, Measurement & measure, appearance_ptr_t & app)
         {
            app_seg_ptr_t targetAppSpec = SPTR_CAST<AppearanceSegment>(targetApp);
            app_seg_ptr_t appSpec = SPTR_CAST<AppearanceSegment>(app);
/*
            measure.std(params.measStd);
            measure.matchScore = matcher.match(targetAppSpec->patch, *(rawPtr->img),
               roi, measure.x()(0), measure.x()(1));
            measure.x() += targetAppSpec->offset.x();
            measure.P() += targetAppSpec->offset.P(); // no cross terms
            rawPtr->img->extractPatch(appSpec->patch, (int)measure.x()(0), (int)measure.x()(1), appSpec->patch.width(), appSpec->patch.height());
            appSpec->offset.x()(0) = measure.x()(0) - ((int)(measure.x()(0)) + 0.5);
            appSpec->offset.x()(1) = measure.x()(1) - ((int)(measure.x()(1)) + 0.5);
            appSpec->offset.P() = measure.P();
*/
         }
   };

   class HDsegDetector
   {
      private:
         HierarchicalDirectSegmentDetector detector;
         boost::shared_ptr<DescriptorFactoryAbstract> descFactory;

      public:
         struct detector_params_t {
            // RANSAC
            double measStd;       ///<       measurement noise std deviation
            double measVar;       ///<       measurement noise variance
            // HDSEG
            int hierarchyLevel;
         } params;

      public:
         HDsegDetector(int hierarchyLevel, double measStd,
            boost::shared_ptr<DescriptorFactoryAbstract> const &descFactory):
            detector(), descFactory(descFactory)
         {
            params.hierarchyLevel = hierarchyLevel;
            params.measStd = measStd;
            params.measVar = measStd * measStd;
         }

         bool detect(const boost::shared_ptr<RawImage> & rawData, const image::ConvexRoi &roi, boost::shared_ptr<FeatureSegment> & featPtr)
         {
            bool ret = false;
            featPtr.reset(new FeatureSegment());
            featPtr->measurement.std(params.measStd);

            ret = detector.detectIn(*(rawData->img.get()), featPtr, &roi);

            if(ret)
            {
               // Don't extract yet

               /*
               // extract appearance
               vec pix = featPtr->measurement.x();
               boost::shared_ptr<AppearanceImagePoint> appPtr = SPTR_CAST<AppearanceImagePoint>(featPtr->appearancePtr);
               rawData->img->extractPatch(appPtr->patch, (int)pix(0), (int)pix(1), params.patchSize, params.patchSize);
               appPtr->offset.x()(0) = pix(0) - ((int)pix(0) + 0.5);
               appPtr->offset.x()(1) = pix(1) - ((int)pix(1) + 0.5);
               appPtr->offset.P() = jblas::zero_mat(2); // by definition this is our landmark projection
               */
            }

            JFR_DEBUG("returning " << (ret ? "true" : "false"));
            return ret;
         }

         void fillDataObs(const boost::shared_ptr<FeatureSegment> & featPtr, boost::shared_ptr<ObservationAbstract> & obsPtr)
         {
            // extract observed appearance
            app_seg_ptr_t app_src = SPTR_CAST<AppearanceSegment>(featPtr->appearancePtr);
            app_seg_ptr_t app_dst = SPTR_CAST<AppearanceSegment>(obsPtr->observedAppearance);
//            app_src->patch.copyTo(app_dst->patch);
            /*app_src->patch.copy(app_dst->patch, (app_src->patch.width()-app_dst->patch.width())/2,
                  (app_src->patch.height()-app_dst->patch.height())/2, 0, 0,
                  app_dst->patch.width(), app_dst->patch.height());*/
            //app_dst->offset = app_src->offset;

            // create descriptor
            descriptor_ptr_t descPtr(descFactory->createDescriptor());
            obsPtr->landmarkPtr()->setDescriptor(descPtr);
         }

   };

}}

#endif // RAWSEGPROCESSORS_HPP
