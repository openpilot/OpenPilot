/*
 * DescriptorImagePointSimu.h
 *
 *  Created on: 20 avr. 2010
 *      Author: jeanmarie
 */

#ifndef DESCRIPTORIMAGEPOINTSIMU_H_
#define DESCRIPTORIMAGEPOINTSIMU_H_

#include "rtslam/descriptorAbstract.hpp"
#include "boost/shared_ptr.hpp"

namespace jafar {
	namespace rtslam {

		class DescriptorImagePoint;
		typedef boost::shared_ptr<DescriptorImagePoint> descimgpnt_ptr_t;


		class DescriptorImagePoint: public jafar::rtslam::DescriptorAbstract {
			public:
				DescriptorImagePoint();
				virtual ~DescriptorImagePoint();
		};
	}
}
#endif /* DESCRIPTORIMAGEPOINTSIMU_H_ */
