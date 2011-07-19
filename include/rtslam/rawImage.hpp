/**
 * \file rawImage.hpp
 * \date 19/05/2010
 * \author croussil
 * File defining an image type
 * \ingroup rtslam
 */

#ifndef RAWIMAGE_HPP_
#define RAWIMAGE_HPP_

#include "image/Image.hpp"
#include "rtslam/rawAbstract.hpp"
#include "boost/shared_ptr.hpp"
//#include "fdetect/HarrisDetector.hpp"
#include "rtslam/quickHarrisDetector.hpp"

namespace jafar {
	namespace rtslam {
		using namespace jafar::image;
//		using namespace jafar::fdetect;

		class RawImage;
		typedef boost::shared_ptr<RawImage> rawimage_ptr_t;

		typedef boost::shared_ptr<image::Image> jafarImage_ptr_t;
		/**
		 * Class for image
		 * \author croussil
		 * \ingroup rtslam
		 */
		class RawImage: public RawAbstract {

			public:
				RawImage();
				~RawImage(){}

				virtual RawAbstract* clone();

				jafarImage_ptr_t img;

				void setJafarImage(jafarImage_ptr_t img) ;

		};
	}
}

#endif /* RAWIMAGE_HPP_ */
