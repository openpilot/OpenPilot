/*
 * DescriptorAbstract.cpp
 *
 *  Created on: 20 avr. 2010
 *      Author: jeanmarie
 */

#include "rtslam/descriptorAbstract.hpp"

namespace jafar {

	namespace rtslam {

		DescriptorAbstract::DescriptorAbstract() {
			// TODO Auto-generated constructor stub

		}

		DescriptorAbstract::~DescriptorAbstract() {
			// TODO Auto-generated destructor stub
		}

		std::ostream& operator <<(std::ostream & s, DescriptorAbstract const & desc) {
			desc.desc_text(s);
			return s;
		}

		image::oimstream& operator <<(image::oimstream & s, DescriptorAbstract const & desc) {
			desc.desc_image(s);
			return s;
		}

	}
}
