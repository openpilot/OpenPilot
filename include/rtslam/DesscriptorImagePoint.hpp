/*
 * DesscriptorImagePointSimu.h
 *
 *  Created on: 20 avr. 2010
 *      Author: jeanmarie
 */

#ifndef DESSCRIPTORIMAGEPOINTSIMU_H_
#define DESSCRIPTORIMAGEPOINTSIMU_H_

#include "rtslam/DescriptorAbstract.hpp"
#include "boost/shared_ptr.hpp"

namespace jafar {
	namespace rtslam {

		class DesscriptorImagePoint;
		typedef boost::shared_ptr<DesscriptorImagePoint> descimgpnt_ptr_t;


		class DesscriptorImagePoint: public jafar::rtslam::DescriptorAbstract {
			public:
				DesscriptorImagePoint();
				virtual ~DesscriptorImagePoint();
		};
	}
}
#endif /* DESSCRIPTORIMAGEPOINTSIMU_H_ */
