/*
 * DescriptorImagePointSimu.h
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

		class DescriptorImagePoint;
		typedef boost::shared_ptr<DescriptorImagePoint> desc_img_pnt_ptr_t;

		class DescriptorImagePoint: public jafar::rtslam::DescriptorAbstract {
			public:
				jblas::vec7 senPoseInit;
				observation_ptr_t obsInitPtr;
				feat_img_pnt_ptr_t featImgPntPtr;

			public:
				DescriptorImagePoint(const feat_img_pnt_ptr_t & featImgPntPtr_,
				    const jblas::vec7 & senPoseInit_, const observation_ptr_t & obsInitPtr_);
				virtual ~DescriptorImagePoint();

				virtual std::string typeName() {
					return "Image point";
				}


				/**
				 * Predict appearance.
				 * \param obsPtrNew pointer to the new observation model
				 */
				bool predictAppearance(const observation_ptr_t & obsPtrNew);
				//bool predictAppearance(const obs_ph_euc_ptr_t & obsPtrNew);
				//bool predictAppearance(const obs_ph_ahp_ptr_t & obsPtrNew);
			protected:
				//bool predictAppearance_img_pt(const observation_ptr_t & obsPtrNew);

		};
	}
}
#endif /* DESCRIPTORIMAGEPOINTSIMU_H_ */
