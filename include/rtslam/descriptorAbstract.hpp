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
				virtual bool predictAppearance(const landmark_ptr_t & lmkPtr, const observation_ptr_t & obsPtr, appearance_ptr_t & appPtr){return false;}
				virtual bool predictAppearance(const landmark_ptr_t & lmkPtr, const observation_ptr_t & obsPtr, appearenceimage_ptr_t & appPtr){return false;}

		};

	}

}

#endif /* DESCRIPTORABSTRACT_H_ */
