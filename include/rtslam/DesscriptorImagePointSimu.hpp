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

		class DesscriptorImagePointSimu;
		typedef boost::shared_ptr<DesscriptorImagePointSimu> descimgpntsimu_ptr_t;


		class DesscriptorImagePointSimu: public jafar::rtslam::DescriptorAbstract {
			public:
				DesscriptorImagePointSimu();
				virtual ~DesscriptorImagePointSimu();
		};
	}
}
#endif /* DESSCRIPTORIMAGEPOINTSIMU_H_ */
