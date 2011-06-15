/*
 * descriptorImageSeg.hpp
 *
 *  Created on: 24 Feb 2011
 *      Author: bhautboi
 * \ingroup rtslam
 */

#ifndef DESCRIPTORIMAGESEG_H_
#define DESCRIPTORIMAGESEG_H_

#ifdef HAVE_MODULE_DSEG

#include "boost/shared_ptr.hpp"

//#include "rtslam/rtSlam.hpp"
#include "rtslam/observationAbstract.hpp"
//#include "rtslam/observationPinHoleEuclideanPoint.hpp"
#include "rtslam/observationPinHoleAnchoredHomogeneousPointsLine.hpp"


#include "rtslam/descriptorAbstract.hpp"
#include "rtslam/featureSegment.hpp"
#include "rtslam/appearanceImage.hpp"

namespace jafar {
   namespace rtslam {

      class DescriptorImageSegFirstView;
      typedef boost::shared_ptr<DescriptorImageSegFirstView> desc_img_seg_fv_ptr_t;
      class DescriptorImageSegMultiView;
      typedef boost::shared_ptr<DescriptorImageSegMultiView> desc_img_seg_mv_ptr_t;

      /**
       * 	A feature view, storing the feature appearance and the conditions
       * that correspond to this appearance
       */
      class ImgSegFeatureView
      {
         public:
            jblas::vec7 senPose;
            appearance_ptr_t appearancePtr;
            observation_model_ptr_t obsModelPtr;
            jblas::vec measurement;
            unsigned frame;
            bool used;

            void clear()
            {
               appearancePtr.reset();
               obsModelPtr.reset();
            }

            void initFromObs(const observation_ptr_t & obsPtr, int descSize);
      };

      std::ostream& operator <<(std::ostream & s, ImgSegFeatureView const & fv);
      image::oimstream& operator <<(image::oimstream & s, ImgSegFeatureView const & fv);

      /**
       * This descriptor for image points only keeps the appearance of the feature
       * the first time it was viewed.
       * In addition it only uses scale and rotation for predicting appearance.
       */
      class DescriptorImageSegFirstView: public DescriptorAbstract {
         protected:
            ImgSegFeatureView view;
         protected:
            int descSize;
         public:
            DescriptorImageSegFirstView(int descSize);
            virtual ~DescriptorImageSegFirstView();

            virtual std::string typeName() const {
               return "Image-Seg-First-View";
            }

            virtual bool addObservation(const observation_ptr_t & obsPtr);
            virtual bool predictAppearance(const observation_ptr_t & obsPtrNew);
            virtual bool isPredictionValid(const observation_ptr_t & obsPtr) { return true; }

            virtual void desc_text(std::ostream& os) const;
            virtual void desc_image(image::oimstream& os) const;
      };

      class DescriptorImageSegFirstViewFactory: public DescriptorFactoryAbstract
      {
         protected:
            int descSize; ///< see DescriptorImageSegFirstView::descSize
         public:
            DescriptorImageSegFirstViewFactory(int descSize):
               descSize(descSize) {}
            DescriptorAbstract *createDescriptor()
               { return new DescriptorImageSegFirstView(descSize); }
      };




      /**
       * This descriptor for image points stores appearances of the feature from
       * different points of view.
       * it can use an homography to predict the appearance
       */
      class DescriptorImageSegMultiView: public DescriptorAbstract
      {
         public:
            typedef std::vector<ImgSegFeatureView> ImgSegFeatureViewList; ///< a ImgSegFeatureView list
            enum PredictionType { ptNone, ptAffine, ptHomographic };

         protected:
            ImgSegFeatureViewList views; ///< the different views of the feature
            ImgSegFeatureView lastValidView; ///< the last valid view
            bool lastObsFailed;
         protected:
            int descSize; ///< the size of the patches in the descriptor
            double scaleStep; ///< the difference of scale that provokes storing of a new view, and the max difference of scale to use a view
            double angleStep; ///< the difference of angle that provokes storing of a new view, in degrees
            PredictionType predictionType;
         private:
            double cosAngleStep;
         public:
            DescriptorImageSegMultiView(int descSize, double scaleStep, double angleStep, PredictionType predictionType);
            virtual ~DescriptorImageSegMultiView() {}

            virtual std::string typeName() const {
               return "Image-Seg-Multi-View";
            }

            virtual bool addObservation(const observation_ptr_t & obsPtr);
            virtual bool predictAppearance(const observation_ptr_t & obsPtr);
            virtual bool isPredictionValid(const observation_ptr_t & obsPtr);

            virtual void desc_text(std::ostream& os) const;
            virtual void desc_image(image::oimstream& os) const;
         protected:
            /**
             * return the closest view and if it is in the bounds or not
             */
            inline void checkView(jblas::vec const &current_pov, double const &current_pov_norm2, jblas::vec const &lmk, ImgSegFeatureView &view, double &cosClosestAngle, ImgSegFeatureView* &closestView) const;
            bool getClosestView(const observation_ptr_t & obsPtr, ImgSegFeatureView* &closestView);
      };

      class DescriptorImageSegMultiViewFactory: public DescriptorFactoryAbstract
      {
         protected:
            int descSize; ///< see DescriptorImageSegMultiView::descSize
            double scaleStep; ///< see DescriptorImageSegMultiView::scaleStep
            double angleStep; ///< see DescriptorImageSegMultiView::angleStep
            DescriptorImageSegMultiView::PredictionType predictionType; ///< see DescriptorImageSegMultiView::predictionType
         public:
            DescriptorImageSegMultiViewFactory(int descSize, double scaleStep, double angleStep, DescriptorImageSegMultiView::PredictionType predictionType):
               descSize(descSize), scaleStep(scaleStep), angleStep(angleStep), predictionType(predictionType) {}
            DescriptorAbstract *createDescriptor()
               { return new DescriptorImageSegMultiView(descSize, scaleStep, angleStep, predictionType); }
      };

   }
}

#endif /* HAVE_MODULE_DSEG */

#endif /* DESCRIPTORIMAGESEG_H_ */

