/**
 * \file descriptorImagePoint.hpp
 *
 * \date 20/04/2010
 * \author jmcodol
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
				unsigned frame;
				bool used;
				
				void clear()
				{
					appearancePtr.reset();
					obsModelPtr.reset();
				}
				
				bool initFromObs(const observation_ptr_t & obsPtr, int descSize);
		};

		std::ostream& operator <<(std::ostream & s, FeatureView const & fv);
		image::oimstream& operator <<(image::oimstream & s, FeatureView const & fv);
		
		/**
		 * This descriptor for image points only keeps the appearance of the feature
		 * the first time it was viewed.
		 * In addition it only uses scale and rotation for predicting appearance.
		 */
		class DescriptorImagePointFirstView: public DescriptorAbstract {
			protected:
				FeatureView view;
			protected:
				int descSize;
			public:
				DescriptorImagePointFirstView(int descSize);
				virtual ~DescriptorImagePointFirstView();

				virtual std::string typeName() const {
					return "Image-Point-First-View";
				}

				virtual bool addObservation(const observation_ptr_t & obsPtr);
				virtual bool predictAppearance(const observation_ptr_t & obsPtrNew);
				virtual bool isPredictionValid(const observation_ptr_t & obsPtr) { return true; }
				
				virtual void desc_text(std::ostream& os) const;
				virtual void desc_image(image::oimstream& os) const;
		};
		
		class DescriptorImagePointFirstViewFactory: public DescriptorFactoryAbstract
		{
			protected:
				int descSize; ///< see DescriptorImagePointFirstView::descSize
			public:
				DescriptorImagePointFirstViewFactory(int descSize):
					descSize(descSize) {}
				DescriptorAbstract *createDescriptor() 
					{ return new DescriptorImagePointFirstView(descSize); }
		};
				
		
		
		
		/**
		 * This descriptor for image points stores appearances of the feature from
		 * different points of view.
		 * it can use an homography to predict the appearance
		 */
		class DescriptorImagePointMultiView: public DescriptorAbstract
		{
			public:
				typedef std::vector<FeatureView> FeatureViewList; ///< a FeatureView list
				enum PredictionType { ptNone = 0, ptAffine, ptHomographic };
				
			protected:
				FeatureViewList views; ///< the different views of the feature
				FeatureView lastValidView; ///< the last valid view
				bool lastObsFailed;
			protected:
				int descSize; ///< the size of the patches in the descriptor
				double scaleStep; ///< the difference of scale that provokes storing of a new view, and the max difference of scale to use a view
				double angleStep; ///< the difference of angle that provokes storing of a new view, in degrees
				PredictionType predictionType;
			private:
				double cosAngleStep;
			public:
				DescriptorImagePointMultiView(int descSize, double scaleStep, double angleStep, PredictionType predictionType);
				virtual ~DescriptorImagePointMultiView() {}
				
				virtual std::string typeName() const {
					return "Image-Point-Multi-View";
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
				inline void checkView(jblas::vec const &current_pov, double const &current_pov_norm2, jblas::vec const &lmk, FeatureView &view, double &cosClosestAngle, FeatureView* &closestView) const;
				bool getClosestView(const observation_ptr_t & obsPtr, FeatureView* &closestView);
		};
		
		class DescriptorImagePointMultiViewFactory: public DescriptorFactoryAbstract
		{
			protected:
				int descSize; ///< see DescriptorImagePointMultiView::descSize
				double scaleStep; ///< see DescriptorImagePointMultiView::scaleStep
				double angleStep; ///< see DescriptorImagePointMultiView::angleStep
				DescriptorImagePointMultiView::PredictionType predictionType; ///< see DescriptorImagePointMultiView::predictionType
			public:
				DescriptorImagePointMultiViewFactory(int descSize, double scaleStep, double angleStep, DescriptorImagePointMultiView::PredictionType predictionType):
					descSize(descSize), scaleStep(scaleStep), angleStep(angleStep), predictionType(predictionType) {}
				DescriptorAbstract *createDescriptor() 
					{ return new DescriptorImagePointMultiView(descSize, scaleStep, angleStep, predictionType); }
		};
		
	}
}
#endif /* DESCRIPTORIMAGEPOINTSIMU_H_ */
