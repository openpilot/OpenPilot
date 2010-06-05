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
#include "boost/shared_ptr.hpp"
#include "image/roi.hpp"

#include "rtslam/rtslamException.hpp"
#include "rtslam/rawImage.hpp"
#include "rtslam/featurePoint.hpp"
#include "rtslam/appearanceImage.hpp"

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
		std::ostream& operator <<(std::ostream & s, jafar::rtslam::RawImage & rawIS) {
			s << " I am a raw-data image structure" << endl;
			return s;
		}

		RawImage::RawImage() : quickHarrisDetector(5, 10.0){
		}

		void RawImage::setJafarImage(jafarImage_ptr_t img_) {
			this->img = img_;
		}


		void RawImage::extractPatch(const size_t width, const size_t height, featurepoint_ptr_t & featPntPtr){
			Image dst(width, height, img.get()->depth(), JfrImage_CS_GRAY);
			int shift_x = (width-1)/2;
			int shift_y = (height-1)/2;
			int x_src = featPntPtr->state.x(0)-shift_x;
			int y_src = featPntPtr->state.x(1)-shift_y;
			img.get()->copy(dst, x_src, y_src, 0, 0, width, height);
			appearenceimage_ptr_t appImgPtr(new AppearenceImage);
			featPntPtr->appearancePtr = appImgPtr;
		}


		bool RawImage::detect(const detect_method met, feature_ptr_t & featPtr,
		    ROI* roiPtr) {

			switch (met) {
				case HARRIS: {

					featurepoint_ptr_t featPntPtr(new FeaturePoint);

					if (quickHarrisDetector.detectIn(*(img.get()), featPntPtr, roiPtr)) {

						// get patch and construct feature
						extractPatch(45, 45, featPntPtr);

						featPtr = featPntPtr;

						return true;

					} else {
						return false;
					}
				}
					break;
				default:
					JFR_ERROR(RtslamException, RtslamException::UNKNOWN_DETECTION_METHOD, "Unrecognized or inexistent feature detection method.")
					;
					break;
			}
		}

	} // namespace rtslam
} // namespace jafar
