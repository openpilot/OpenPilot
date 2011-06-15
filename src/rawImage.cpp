/**
 * \file rawImageSimu.cpp
 * \date 01/04/2010
 * \author jmcodol
 * \ingroup rtslam
 */

//#include "cv.h"
//#include "highgui.h"


#include "boost/shared_ptr.hpp"
#include "image/Image.hpp"
#include "image/roi.hpp"

#include "rtslam/rtslamException.hpp"
#include "rtslam/rawImage.hpp"
#include "rtslam/featurePoint.hpp"
#include "rtslam/appearanceImage.hpp"

#include "correl/explorer.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;
		using namespace jafar::image;

		///////////////////////////////////
		// RAW IMAGE CONTAINING REAL IMAGE
		///////////////////////////////////

		/*
		 * Operator << for class rawAbstract.
		 * It shows some informations
		 */
		std::ostream& operator <<(std::ostream & s, jafar::rtslam::RawImage const & rawIS) {
			s << " I am a raw-data image structure" << endl;
			return s;
		}

		RawImage::RawImage(){
		}

		void RawImage::setJafarImage(jafarImage_ptr_t img_) {
			this->img = img_;
		}

	} // namespace rtslam
} // namespace jafar
