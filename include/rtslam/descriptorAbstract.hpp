/*
 * DescriptorAbstract.h
 *
 *  Created on: 20 avr. 2010
 *      Author: jeanmarie
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

				//FIXME See the stuff on using DescAbs::predApp... ask Nico.
				bool predictAppearance(observation_ptr_t & obsPtr) {return false;}

		};

	}

}

#endif /* DESCRIPTORABSTRACT_H_ */
