/*
 * DescriptorImagePointSimu.h
 *
 *  Created on: 20 avr. 2010
 *      Author: jeanmarie
 */

#ifndef DESCRIPTORIMAGEPOINTSIMU_H_
#define DESCRIPTORIMAGEPOINTSIMU_H_

#include "boost/shared_ptr.hpp"

//#include "rtslam/rtSlam.hpp"
#include "rtslam/observationAbstract.hpp"
#include "rtslam/descriptorAbstract.hpp"
#include "rtslam/featurePoint.hpp"
#include "rtslam/appearanceImage.hpp"

namespace jafar {
	namespace rtslam {

		class DescriptorImagePoint;
		typedef boost::shared_ptr<DescriptorImagePoint> descimgpnt_ptr_t;

		class DescriptorImagePoint: public jafar::rtslam::DescriptorAbstract {
			public:
				jblas::vec7 pose0;
				observation_ptr_t obsPtr0;
				featurepoint_ptr_t featImgPntPtr;

			public:
				DescriptorImagePoint(const featurepoint_ptr_t & featPtr,
				    const jblas::vec7 & pose0, observation_ptr_t & obsPtr);
				virtual ~DescriptorImagePoint();

				virtual bool predictAppearance(const landmark_ptr_t & lmkPtr,
				    const observation_ptr_t & obsPtrNew, appearenceimage_ptr_t & appPtr);

		};
	}
}
#endif /* DESCRIPTORIMAGEPOINTSIMU_H_ */
