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

		RawImage::RawImage() : quickHarrisDetector(5, 10.0){
		}

		void RawImage::setJafarImage(jafarImage_ptr_t img_) {
			this->img = img_;
		}


		void RawImage::extractAppearance(const jblas::veci & pos, const jblas::veci & size, appearance_ptr_t & appPtr){
			Image dst(size(0), size(1), img->depth(), JfrImage_CS_GRAY);
//			Image dst(320, 240, img.get()->depth(), JfrImage_CS_GRAY);
			std::cout << "depths : " << img->depth() << " " << dst.depth() << std::endl;
			int shift_x = (size(0)-1)/2;
			int shift_y = (size(1)-1)/2;
			int x_src = pos(0)-shift_x;
			int y_src = pos(1)-shift_y;

//			cvSetImageROI((*img), cvRect(x_src, y_src, size, size));
//
//			cvCopyImage(IplImage(img), IplImage(dst));

//			JFR_DEBUG("xs: " << x_src << "ys: " << y_src)
			img->copy(dst, x_src, y_src, 0, 0, size(0), size(1));
//			img->copy(dst, 0, 0, 0, 0, 320, 240);

			appearenceimage_ptr_t appImgPtr(new AppearenceImage);
			appImgPtr->patch = dst;
			appPtr = appImgPtr;

			// write ppm image from patch
			img->print();
			dst.print();
			img->save("/home/jsola/img.ppm");
			dst.save("/home/jsola/patch.ppm");

		}

//		void RawImage::getPatch(const int x, const int y, const int width, const int height, Image & dst) {
//
//			 // Must have dimensions of output image
//			IplImage* patch = cvCreatImage(width, height, img.get()->depth());
//
//			int shift_x = (width - 1) / 2;
//			int x0 = x - shift_x;
//			int shift_y = (height - 1) / 2;
//			int y0 = y - shift_y;
//
//			// Say what the source region is
//			cvSetImageROI(img.get(), cvRect(x0, y0, width, height));
//
//			// Do the copy
//			cvCopy(img.get(), patch);
//			cvResetImageROI( src);
//
//			return patch;
//
//			/* get properties, needed to create dest image */
//			int width = src->width;
//			int height = src->height;
//      int depth     = src->depth;
//      int nchannels = src->nChannels;
//      /* create destination image */
//      IplImage *dst = cvCreateImage( cvSize( 45, 45 ),
//                                     depth, nchannels );
//      cvRect(0,0,45,45) ;
//      cvSetImageROI()
//      /* copy from source to dest */
//      cvCopy( src, dst, NULL );
//      /* and save to file */
//      cvSaveImage( "test.ppm", dst );
//		}

		bool RawImage::detect(const detect_method met, feature_ptr_t & featPtr,
		    ROI* roiPtr) {

			switch (met) {
				case HARRIS: {

					featurepoint_ptr_t featPntPtr(new FeaturePoint);

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
