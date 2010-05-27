/*
 * DescriptorAbstract.h
 *
 *  Created on: 20 avr. 2010
 *      Author: jeanmarie
 */

#ifndef DESCRIPTORABSTRACT_H_
#define DESCRIPTORABSTRACT_H_


#include "rtslam/rtSlam.hpp"
#include "jmath/jblas.hpp"

#include <boost/smart_ptr.hpp>

namespace jafar {

	namespace rtslam {

		class DescriptorAbstract {
			public:

				jblas::vec7 pose0;
				appearance_ptr_t app0Ptr;

				DescriptorAbstract();
				virtual ~DescriptorAbstract();

		};

	}

}

#endif /* DESCRIPTORABSTRACT_H_ */
