/**
 *
 * \file rawImage.hpp
 * \author croussil@laas.fr
 * \date 19/05/2010
 * File defining an image type
 * \ingroup rtslam
 */

#ifndef RAWIMAGE_HPP_
#define RAWIMAGE_HPP_

#include "image/Image.hpp"
#include "rtslam/rawAbstract.hpp"
#include "boost/shared_ptr.hpp"
//#include "fdetect/HarrisDetector.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;
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
				jafarImage_ptr_t img;
//				HarrisDetector harrisDetector;

				void setJafarImage(jafarImage_ptr_t img) ;

//				virtual bool detect(const detect_method met, feature_ptr_t & featurePtr) {
//					DetectionResult pixels = harrisDetector.detectIn(*(img.get()));
//					return false;
//				}

			private:

		};
	}
}

#endif /* RAWIMAGE_HPP_ */
