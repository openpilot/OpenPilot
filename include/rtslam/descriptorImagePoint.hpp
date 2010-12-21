/*
 * DescriptorImagePointFirstView.h
 *
 *  Created on: 20 avr. 2010
 *      Author: jeanmarie
 * \ingroup rtslam
 */

#ifndef DESCRIPTORIMAGEPOINTSIMU_H_
#define DESCRIPTORIMAGEPOINTSIMU_H_

#include "boost/shared_ptr.hpp"

//#include "rtslam/rtSlam.hpp"
#include "rtslam/observationAbstract.hpp"
#include "rtslam/observationPinHoleEuclideanPoint.hpp"
#include "rtslam/observationPinHoleAnchoredHomogeneous.hpp"


#include "rtslam/descriptorAbstract.hpp"
#include "rtslam/featurePoint.hpp"
#include "rtslam/appearanceImage.hpp"

namespace jafar {
	namespace rtslam {

		class DescriptorImagePointFirstView;
		typedef boost::shared_ptr<DescriptorImagePointFirstView> desc_img_pnt_fv_ptr_t;
		class DescriptorImagePointMultiView;
		typedef boost::shared_ptr<DescriptorImagePointMultiView> desc_img_pnt_mv_ptr_t;

		/**
		 * 	A feature view, storing the feature appearance and the conditions
		 * that correspond to this appearance
		 */
		class FeatureView
		{
			public:
				jblas::vec7 senPose;
				appearance_ptr_t appearancePtr;
				observation_model_ptr_t obsModelPtr;
				jblas::vec measurement;
				
				void clear()
				{
					appearancePtr.reset();
					obsModelPtr.reset();
				}
		};

		
		/**
		 * This descriptor for image points only keeps the appearance of the feature
		 * the first time it was viewed.
		 * In addition it only uses scale and rotation for predicting appearance.
		 */
		class DescriptorImagePointFirstView: public jafar::rtslam::DescriptorAbstract {
			public:
				FeatureView view;

			public:
				DescriptorImagePointFirstView();
				virtual ~DescriptorImagePointFirstView();

/*				virtual std::string typeName() {
					return "Image point";
				}
*/
				virtual void addObservation(const observation_ptr_t & obsPtr);
				virtual bool predictAppearance(const observation_ptr_t & obsPtrNew);
				virtual bool isPredictionValid(const observation_ptr_t & obsPtr) { return true; }
		};
		
		
		
		
		
		/**
		 * This descriptor for image points stores appearances of the feature from
		 * different points of view.
		 * it can use an homography to predict the appearance
		 */
		class DescriptorImagePointMultiView: public jafar::rtslam::DescriptorAbstract
		{
			public:
				typedef std::vector<FeatureView> FeatureViewList; ///< a FeatureView list
				enum PredictionType { ptNone, ptAffine, ptHomographic };
				
			protected:
				FeatureViewList views; ///< the different views of the feature
				FeatureView lastValidView; ///< the last valid view
		protected:
				double scaleStep; ///< the difference of scale that provokes storing of a new view, and the max difference of scale to use a view
				double angleStep; ///< the difference of angle that provokes storing of a new view, in degrees
				PredictionType predictionType;
			private:
				double cosAngleStep;
			public:
				DescriptorImagePointMultiView(double scaleStep, double angleStep, PredictionType predictionType);
				virtual ~DescriptorImagePointMultiView() {}
				
				virtual void addObservation(const observation_ptr_t & obsPtr);
				virtual bool predictAppearance(const observation_ptr_t & obsPtr);
				virtual bool isPredictionValid(const observation_ptr_t & obsPtr);
				
			protected:
				/**
				 * return the closest view and if it is in the bounds or not
				 */
				inline void checkView(jblas::vec const &current_pov, double const &current_pov_norm2, jblas::vec const &lmk, FeatureView &view, double &cosClosestAngle, FeatureView* &closestView) const;
				bool getClosestView(const observation_ptr_t & obsPtr, FeatureView* &closestView);
		};
		
		class DescriptorImagePointMultiViewFactory
		{
			protected:
				double scaleStep; ///< see DescriptorImagePointMultiView::scaleStep
				double angleStep; ///< see DescriptorImagePointMultiView::angleStep
				DescriptorImagePointMultiView::PredictionType predictionType; ///< see DescriptorImagePointMultiView::predictionType
			public:
				DescriptorImagePointMultiViewFactory(double scaleStep, double angleStep, DescriptorImagePointMultiView::PredictionType predictionType):
					scaleStep(scaleStep), angleStep(angleStep), predictionType(predictionType) {}
				DescriptorImagePointMultiView *createDescriptor() 
					{ return new DescriptorImagePointMultiView(scaleStep, angleStep, predictionType); }
		};
		
	}
}
#endif /* DESCRIPTORIMAGEPOINTSIMU_H_ */
