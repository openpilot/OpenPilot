/*
 * descriptorSeg.hpp
 *
 *  Created on: 24 Feb 2011
 *      Author: bhautboi
 * \ingroup rtslam
 */

#ifndef DESCRIPTORSEG_HPP
#define DESCRIPTORSEG_HPP

#include "boost/shared_ptr.hpp"

#include "rtslam/observationAbstract.hpp"
#include "rtslam/observationPinHoleAnchoredHomogeneousPointsLine.hpp"


#include "rtslam/descriptorAbstract.hpp"
#include "rtslam/descriptorImageSeg.hpp"
#include "rtslam/featureSegment.hpp"
#include "rtslam/appearanceSegment.hpp"

namespace jafar {
   namespace rtslam {

      class DescriptorSegFirstView;
      typedef boost::shared_ptr<DescriptorSegFirstView> desc_seg_fv_ptr_t;
      class DescriptorSegMultiView;
      typedef boost::shared_ptr<DescriptorSegMultiView> desc_seg_mv_ptr_t;

      /**
       * 	A feature view, storing the feature appearance and the conditions
       * that correspond to this appearance
       */
      class SegFeatureView
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

      std::ostream& operator <<(std::ostream & s, SegFeatureView const & fv);
      image::oimstream& operator <<(image::oimstream & s, SegFeatureView const & fv);

      /**
       * This descriptor for image points only keeps the appearance of the feature
       * the first time it was viewed.
       * In addition it only uses scale and rotation for predicting appearance.
       */
      class DescriptorSegFirstView: public DescriptorAbstract {
         protected:
            SegFeatureView view;
         protected:
            int descSize;
         public:
            DescriptorSegFirstView(int descSize);
            virtual ~DescriptorSegFirstView();

            virtual std::string typeName() const {
               return "Seg-First-View";
            }

            virtual bool addObservation(const observation_ptr_t & obsPtr);
            virtual bool predictAppearance(const observation_ptr_t & obsPtrNew);
            virtual bool isPredictionValid(const observation_ptr_t & obsPtr) { return true; }

            virtual void desc_text(std::ostream& os) const;
            virtual void desc_image(image::oimstream& os) const;
      };

      class DescriptorSegFirstViewFactory: public DescriptorFactoryAbstract
      {
         protected:
            int descSize; ///< see DescriptorImageSegFirstView::descSize
         public:
            DescriptorSegFirstViewFactory(int descSize):
               descSize(descSize) {}
            DescriptorAbstract *createDescriptor()
               { return new DescriptorSegFirstView(descSize); }
      };




      /**
       * This descriptor for image points stores appearances of the feature from
       * different points of view.
       * it can use an homography to predict the appearance
       */
/*      class DescriptorSegMultiView: public DescriptorAbstract
      {
         public:
            typedef std::vector<SegFeatureView> SegFeatureViewList; ///< a SegFeatureView list
            enum PredictionType { ptNone, ptAffine, ptHomographic };

         protected:
            SegFeatureViewList views; ///< the different views of the feature
            SegFeatureView lastValidView; ///< the last valid view
            bool lastObsFailed;
         protected:
            int descSize; ///< the size of the patches in the descriptor
            double scaleStep; ///< the difference of scale that provokes storing of a new view, and the max difference of scale to use a view
            double angleStep; ///< the difference of angle that provokes storing of a new view, in degrees
            PredictionType predictionType;
         private:
            double cosAngleStep;
         public:
            DescriptorSegMultiView(int descSize, double scaleStep, double angleStep, PredictionType predictionType);
            virtual ~DescriptorSegMultiView() {}

            virtual std::string typeName() const {
               return "Seg-Multi-View";
            }

            virtual bool addObservation(const observation_ptr_t & obsPtr);
            virtual bool predictAppearance(const observation_ptr_t & obsPtr);
            virtual bool isPredictionValid(const observation_ptr_t & obsPtr);

            virtual void desc_text(std::ostream& os) const;
            virtual void desc_image(image::oimstream& os) const;
         protected:
            // return the closest view and if it is in the bounds or not
            inline void checkView(jblas::vec const &current_pov, double const &current_pov_norm2, jblas::vec const &lmk, SegFeatureView &view, double &cosClosestAngle, SegFeatureView* &closestView) const;
            bool getClosestView(const observation_ptr_t & obsPtr, SegFeatureView* &closestView);
      };

      class DescriptorSegMultiViewFactory: public DescriptorFactoryAbstract
      {
         protected:
            int descSize; ///< see DescriptorImageSegMultiView::descSize
            double scaleStep; ///< see DescriptorImageSegMultiView::scaleStep
            double angleStep; ///< see DescriptorImageSegMultiView::angleStep
            DescriptorSegMultiView::PredictionType predictionType; ///< see DescriptorImageSegMultiView::predictionType
         public:
            DescriptorSegMultiViewFactory(int descSize, double scaleStep, double angleStep, DescriptorSegMultiView::PredictionType predictionType):
               descSize(descSize), scaleStep(scaleStep), angleStep(angleStep), predictionType(predictionType) {}
            DescriptorAbstract *createDescriptor()
               { return new DescriptorSegMultiView(descSize, scaleStep, angleStep, predictionType); }
      };
*/
   }
}
#endif /* DESCRIPTORSEG_H_ */


