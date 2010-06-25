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

#include "cv.h"
#include "highgui.h"


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
		std::ostream& operator <<(std::ostream & s, jafar::rtslam::RawImage & rawIS) {
			s << " I am a raw-data image structure" << endl;
			return s;
		}

		RawImage::RawImage() : quickHarrisDetector(9, 15.0, 5.0){
		}

		void RawImage::setJafarImage(jafarImage_ptr_t img_) {
			this->img = img_;
		}


		void RawImage::extractAppearance(const jblas::veci & pos, appearance_ptr_t & appPtr){
			app_img_pnt_ptr_t app = SPTR_CAST<AppearanceImagePoint>(appPtr);
			cv::Size size = app->patch.size();
			int shift_x = (size.width-1)/2;
			int shift_y = (size.height-1)/2;
			int x_src = pos(0)-shift_x;
			int y_src = pos(1)-shift_y;
			img->copy(app->patch, x_src, y_src, 0, 0, size.width, size.height);
		}

		bool RawImage::detect(const detect_method met, const feature_ptr_t & featPtr,
		    ROI* roiPtr) {

			switch (met) {
				case HARRIS: {

					//feat_img_pnt_ptr_t featPntPtr(new FeatureImagePoint);
					feat_img_pnt_ptr_t featPntPtr = SPTR_CAST<FeatureImagePoint>(featPtr);

					if (quickHarrisDetector.detectIn(*(img.get()), featPntPtr, roiPtr)) {

						//featPtr = featPntPtr;

						// get patch and construct feature
						vec pix = featPntPtr->measurement.x();
						extractAppearance(pix, featPntPtr->appearancePtr);
//((AppearanceImagePoint*)(featPntPtr->appearancePtr.get()))->patch.save("extracted_app.png");
						return true;

					} else {
						return false;
					}
				}
					break;
				default:
					JFR_ERROR(RtslamException, RtslamException::UNKNOWN_DETECTION_METHOD, "Unrecognized or inexistent feature detection method.");
					return false;
			}
		}
		
		bool RawImage::match(const match_method met, const appearance_ptr_t & targetApp, cv::Rect &roi, Measurement & measure, const appearance_ptr_t & app)
		{
			switch (met) {
				case ZNCC: {
					app_img_pnt_ptr_t targetAppImg = SPTR_CAST<AppearanceImagePoint>(targetApp);
					app_img_pnt_ptr_t appImg = SPTR_CAST<AppearanceImagePoint>(app);
					
					measure.matchScore = correl::Explorer<correl::Zncc>::exploreTranslation(
							targetAppImg->patch, *img, roi.x, roi.x+roi.width-1, 1, roi.y, roi.y+roi.height-1, 1,
							measure.x()(0), measure.x()(1));
			
					// TODO set appearance
//					app = img->copy(appImg->patch, measure.x()(0), measure.x()(1));
			
					return true;
				}
				default:
					JFR_ERROR(RtslamException, RtslamException::UNKNOWN_MATCHING_METHOD, "Unrecognized or inexistent feature matching method.");
					return false;
			}
		}

	} // namespace rtslam
} // namespace jafar
