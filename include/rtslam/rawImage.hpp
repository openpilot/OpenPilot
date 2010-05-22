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

namespace jafar {
	namespace rtslam {
		using namespace std;

		class RawImage;
		typedef boost::shared_ptr<RawImage> rawimage_ptr_t;

		/**
		 * Class for image
		 * \author croussil
		 * \ingroup rtslam
		 */
		class RawImage: public RawAbstract {

			public:
				image::Image img;

			private:

		};
	}
}

#endif /* RAWIMAGE_HPP_ */
