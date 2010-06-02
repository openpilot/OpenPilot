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

#include "rtslam/rtslamException.hpp"
#include "rtslam/rawImage.hpp"
#include "rtslam/featurePoint.hpp"

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
			this->img = img_;
		}

		bool RawImage::detect(const detect_method met, feature_ptr_t & featPtr,
		    const ROI* roiPtr) {

			switch (met) {
				case HARRIS: {
					InterestFeature feat_fdetect(0, 0);

					if (harrisDetector.detectBestPntInRoi(*(img.get()), &feat_fdetect), roiPtr) {

						boost::shared_ptr<FeaturePoint> featPntPtr(new FeaturePoint);

						featPntPtr->setup(feat_fdetect.u(), feat_fdetect.v(), feat_fdetect.quality());

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
