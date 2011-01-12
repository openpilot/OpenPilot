/*
 * \file descriptorAbstract.cpp
 * \date 20/04/2010
 * \author jmcodol
 * \ingroup rtslam
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
