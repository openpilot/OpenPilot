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
#include "image/Image.hpp"
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

		RawImage::RawImage() : quickHarrisDetector(5, 10, 5){
		}

		void RawImage::setJafarImage(jafarImage_ptr_t img_) {
			this->img = img_;
		}


		void RawImage::extractAppearance(const jblas::veci & pos, const jblas::veci & size, appearance_ptr_t & appPtr){
			Image dst(size(0), size(1), img->depth(), JfrImage_CS_GRAY);
			int shift_x = (size(0)-1)/2;
			int shift_y = (size(1)-1)/2;
			int x_src = pos(0)-shift_x;
			int y_src = pos(1)-shift_y;

			img->copy(dst, x_src, y_src, 0, 0, size(0), size(1));

			app_img_pnt_ptr_t appImgPtr(new AppearenceImagePoint(dst));
//			cout << "patch sum: " << appImgPtr->patchSum << "; squareSum: " << appImgPtr->patchSquareSum << endl;
			appPtr = appImgPtr;

		}

		bool RawImage::detect(const detect_method met, feature_ptr_t & featPtr,
		    ROI* roiPtr) {

			switch (met) {
				case HARRIS: {

					feat_img_pnt_ptr_t featPntPtr(new FeatureImagePoint);

					if (quickHarrisDetector.detectIn(*(img.get()), featPntPtr, roiPtr)) {

						featPtr = featPntPtr;

						// get patch and construct feature
						vec pix(2);
						pix = featPntPtr->state.x();
						veci size(2); size(0) = 45, size(1) = 45;
						extractAppearance(pix, size, featPntPtr->appearancePtr);

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
