/*
 * DescriptorAbstract.h
 *
 *  Created on: 20 avr. 2010
 *      Author: jeanmarie
 * \ingroup rtslam
 */

#ifndef DESCRIPTORABSTRACT_H_
#define DESCRIPTORABSTRACT_H_

#include <boost/smart_ptr.hpp>

#include "jmath/jblas.hpp"

#include "rtslam/rtSlam.hpp"
//#include "rtslam/appearanceAbstract.hpp"
#include "rtslam/appearanceImage.hpp"

namespace jafar {

	namespace rtslam {

		class DescriptorAbstract {
			
			
			public:

				DescriptorAbstract();
				virtual ~DescriptorAbstract();

				virtual bool predictAppearance(observation_ptr_t & obsPtr) {
					cout << __FILE__ << ":" << __LINE__ << endl;
					return false;}

				virtual std::string categoryName() {
					return "DESCRIPTOR";
				}


		};

	}

}

#endif /* DESCRIPTORABSTRACT_H_ */
