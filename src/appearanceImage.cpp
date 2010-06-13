/*
 * AppearanceAbstract.cpp
 *
 *  Created on: 14 avr. 2010
 *      Author: jeanmarie
 */

#include "rtslam/appearanceImage.hpp"
#include "image/Image.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;

		AppearenceImage::AppearenceImage(const image::Image& patch) : patch(patch) {
			computePatchIntegrals();
		}

		AppearenceImage::~AppearenceImage() {
		}

		void AppearenceImage::computePatchIntegrals(){
			patchSum = 0;
			patchSquareSum = 0;
			uchar* pix = patch.data();
			for (int u = 0; u < patch.width()*patch.height(); u++){
				patchSum += *pix;
				patchSquareSum += (*pix) * (*pix);
				pix ++;
			}
		}

	}
}
