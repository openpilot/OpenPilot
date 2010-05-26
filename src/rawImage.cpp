/**
 * rawImageSimu.cpp
 *
 * \date 1/04/2010
 * \author jmcodol
 *
 *  \file rawImageSimu.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/rawImage.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;

		///////////////////////////////////
		// RAW IMAGE CONTAINING REAL IMAGE
		///////////////////////////////////

			/*
			 * Operator << for class rawAbstract.
			 * It shows some informations
			 */
			std::ostream& operator <<(std::ostream & s, jafar::rtslam::RawImage & rawIS) {
				s << " I am a raw-data image structure" << endl;
				return s;
			}

			void RawImage::setJafarImage(jafarImage_ptr_t img_) {
				this->img = img_ ;
			}

	} // namespace rtslam
} // namespace jafar
