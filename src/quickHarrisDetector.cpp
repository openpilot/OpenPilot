/*
 * \file quickHarrisDetector.cpp
 *
 *  \date  Jun 4, 2010
 *  \author: jsola
 *
 *  \ingroup rtslam
 */

#include "image/roi.hpp"
#include "rtslam/quickHarrisDetector.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;
		using namespace image;

		jafar::rtslam::QuickHarrisDetector::QuickHarrisDetector(
		    float convolutionBoxSize, float threshold) :
			m_threshold(threshold), m_derivationSize(1),
			    m_convolutionSize(convolutionBoxSize) {
		}

		bool jafar::rtslam::QuickHarrisDetector::detectIn(
		    const jafar::image::Image & image, featurepoint_ptr_t featPtr,
		    jafar::image::ROI *roiPtr) {
			//	JFR_PRED_ERROR( image.colorSpace() == JfrImage_CS_GRAY, FdetectException, FdetectException::INVALID_COLORSPACE,"QuickHarrisDetector::detectIn image must be of the same colorspace and in Greyscale");

			ROI localRoi;

			if (roiPtr == 0) {
				localRoi.x = 0;
				localRoi.y = 0;
				localRoi.width = image.width();
				localRoi.height = image.height();
			} else {
				localRoi = *roiPtr;
			}

			int pixBest[2];
			float scoreBest;

			m_quickData = new HQuickData[localRoi.width * localRoi.height]; // processing data structure with integral convolution images

			quickDerivatives(image, localRoi);
			bool success = quickConvolutionWithBestPoint(localRoi, pixBest, scoreBest);
			    // writeHarrisImagesAsPPM(localRoi);

			delete[] m_quickData;

			if (success) {
				featPtr->setup(pixBest[0], pixBest[1], scoreBest);
			}
			return success;
		}

		void jafar::rtslam::QuickHarrisDetector::quickDerivatives(
		    const jafar::image::Image & image, jafar::image::ROI & roi) {
			const uchar* pix_center;
			const uchar* pix_right;
			const uchar* pix_left;
			const uchar* pix_down;
			const uchar* pix_up;

			HQuickData* int_center;
			HQuickData* int_up;
			HQuickData* int_upLeft;
			HQuickData* int_left;

			// dead zone due to derivative mask [-1 0 1]
			int shift_derv = 1; // = (3-1)/2

			// i: vert; j: horz  image coordinates
			int iMin = roi.y + shift_derv;
			int iMax = roi.y + roi.height - shift_derv;
			int jMin = roi.x + shift_derv;
			int jMax = roi.x + roi.width - shift_derv;

			int i, j; // i, j: image coordinates
			int ri; // ri: vertical roi coordinate
			for (i = iMin; i < iMax; i++) {
				ri = i - roi.y;

				pix_center = image.data() + (i * image.step()) + jMin;
				pix_right = pix_center + 1;
				pix_left = pix_center - 1;
				pix_down = pix_center + image.step();
				pix_up = pix_center - image.step();

				int_center = m_quickData + (ri * roi.width) + shift_derv;
				int_up = int_center - roi.width;
				int_upLeft = int_up - 1;
				int_left = int_center - 1;

				for (j = jMin; j < jMax; j++) {
					// Build x and y derivatives, and their products: xx, xy and yy.
					int_center->im_x = *pix_right - *pix_left;
					int_center->im_y = *pix_down - *pix_up;
					int_center->im_xx = int_center->im_x * int_center->im_x;
					int_center->im_xy = int_center->im_x * int_center->im_y;
					int_center->im_yy = int_center->im_y * int_center->im_y;

					// build integral images of the 3 derivative products, xx, xy and yy:
					if (i == iMin && j == jMin) {
						int_center->int_xx = int_center->im_xx;
						int_center->int_xy = int_center->im_xy;
						int_center->int_yy = int_center->im_yy;
					} else if (i == iMin) {
						int_center->int_xx = int_center->im_xx + int_left->int_xx;
						int_center->int_xy = int_center->im_xy + int_left->int_xy;
						int_center->int_yy = int_center->im_yy + int_left->int_yy;
					} else if (j == jMin) {
						int_center->int_xx = int_center->im_xx + int_up->int_xx;
						int_center->int_xy = int_center->im_xy + int_up->int_xy;
						int_center->int_yy = int_center->im_yy + int_up->int_yy;
					} else {
						int_center->int_xx = int_center->im_xx + int_up->int_xx
						    + int_left->int_xx - int_upLeft->int_xx;
						int_center->int_xy = int_center->im_xy + int_up->int_xy
						    + int_left->int_xy - int_upLeft->int_xy;
						int_center->int_yy = int_center->im_yy + int_up->int_yy
						    + int_left->int_yy - int_upLeft->int_yy;
					}
					// Advance all pointers
					pix_center++;
					pix_right++;
					pix_left++;
					pix_up++;
					pix_down++;

					int_center++;
					int_up++;
					int_left++;
					int_upLeft++;

				}
			}

		}

		bool jafar::rtslam::QuickHarrisDetector::quickConvolutionWithBestPoint(
		    const jafar::image::ROI & roi, int pixMax[2], float & scoreMax) {
			// margins
			int shift_conv = (m_convolutionSize - 1) / 2;
			int shift_derv = 1;
			int shift_derv_conv = shift_derv + shift_conv;

			// bounds
			int riMin = shift_derv_conv;
			int riMax = roi.height - shift_derv_conv;
			int rjMin = shift_derv_conv;
			int rjMax = roi.width - shift_derv_conv;

			// data structure pointers
			HQuickData* int_center;
			HQuickData* int_upLeft;
			HQuickData* int_upRight;
			HQuickData* int_downLeft;
			HQuickData* int_downRight;

			float im_low_curv_max = 0; // maximum smallest eigenvalue

			int sm, df; // sum and difference
			float sr, corner_ratio;
			int ri, rj; // roi coordinates

			for (ri = riMin; ri < riMax; ri++) {

				int_center = m_quickData + (ri * roi.width) + rjMin;

				int_upLeft = int_center - (shift_conv * roi.width) - shift_conv;
				int_upRight = int_center - (shift_conv * roi.width) + shift_conv;
				int_downLeft = int_center + (shift_conv * roi.width) - shift_conv;
				int_downRight = int_center + (shift_conv * roi.width) + shift_conv;

				for (rj = rjMin; rj < rjMax; rj++) {

					int_center->im_conv_xx = int_downRight->int_xx - int_upRight->int_xx
					    - int_downLeft->int_xx + int_upLeft->int_xx;
					int_center->im_conv_xy = int_downRight->int_xy - int_upRight->int_xy
					    - int_downLeft->int_xy + int_upLeft->int_xy;
					int_center->im_conv_yy = int_downRight->int_yy - int_upRight->int_yy
					    - int_downLeft->int_yy + int_upLeft->int_yy;

					// get eigenvalues: EIG/eig = I_xx + I_yy +/- sqrt((Ixx - I_yy)^2 + 4*I_xy^2)
					sm = int_center->im_conv_xx + int_center->im_conv_yy;
					df = int_center->im_conv_xx - int_center->im_conv_yy;
					sr = sqrt((double) ((double) df * (double) df + 4
					    * ((double) int_center->im_conv_xy)
					    * ((double) int_center->im_conv_xy)));
					int_center->im_high_curv = (double) sm + sr; // Smallest eigenvalue.
					int_center->im_low_curv = (double) sm - sr; // Largest eigenvalue.

					// detect and write pixel corresponding to strongest corner, with score.
					corner_ratio = int_center->im_high_curv / int_center->im_low_curv; // CAUTION should be high/low
					if (corner_ratio < 2) {
						if (int_center->im_low_curv > im_low_curv_max) {
							im_low_curv_max = int_center->im_low_curv;
							pixMax[0] = roi.x + rj;
							pixMax[1] = roi.y + ri;
						}
					}

					int_center++;
					int_upLeft++;
					int_upRight++;
					int_downLeft++;
					int_downRight++;

				}
			}

			// normalized score: over the size of the derivative and convolution windows
			scoreMax = sqrt(im_low_curv_max / (2 * m_convolutionSize
			    * m_convolutionSize));

			return (scoreMax > m_threshold);

		}

		void jafar::rtslam::QuickHarrisDetector::writeHarrisImagesAsPPM(
		    jafar::image::ROI & roi) {
			FILE *pFile_x = fopen("/home/jsola/im_x.ppm", "w");
			FILE *pFile_y = fopen("/home/jsola/im_y.ppm", "w");
			FILE *pFile_xx = fopen("/home/jsola/im_xx.ppm", "w");
			FILE *pFile_xy = fopen("/home/jsola/im_xy.ppm", "w");
			FILE *pFile_yy = fopen("/home/jsola/im_yy.ppm", "w");
			FILE *pFile_min = fopen("/home/jsola/im_min.ppm", "w");
			FILE *pFile_max = fopen("/home/jsola/im_max.ppm", "w");
			HQuickData * ptr = m_quickData;
			if (pFile_x != NULL) {
				fprintf(pFile_x, "P2\n%d %d\n255\n", roi.width, roi.height);
				fprintf(pFile_y, "P2\n%d %d\n255\n", roi.width, roi.height);
				fprintf(pFile_xx, "P2\n%d %d\n255\n", roi.width, roi.height);
				fprintf(pFile_xy, "P2\n%d %d\n255\n", roi.width, roi.height);
				fprintf(pFile_yy, "P2\n%d %d\n255\n", roi.width, roi.height);
				fprintf(pFile_min, "P2\n%d %d\n255\n", roi.width, roi.height);
				fprintf(pFile_max, "P2\n%d %d\n255\n", roi.width, roi.height);
				for (int i = 0; i < (roi.height * roi.width); i++) {
					fprintf(pFile_x, "%d ", 128 + (ptr->im_x) / 2);
					fprintf(pFile_y, "%d ", 128 + (ptr->im_y) / 2);
					fprintf(pFile_xx, "%d ", 128 + (ptr->im_conv_xx) / 1024);
					fprintf(pFile_xy, "%d ", 128 + (ptr->im_conv_xy) / 1024);
					fprintf(pFile_yy, "%d ", 128 + (ptr->im_conv_yy) / 1024);
					fprintf(pFile_min, "%d ", (unsigned char) (ptr->im_low_curv / 1024));
					fprintf(pFile_max, "%d ", (unsigned char) (ptr->im_high_curv / 2048));
					ptr++;
				}
				fclose(pFile_x);
				fclose(pFile_y);
				fclose(pFile_xx);
				fclose(pFile_xy);
				fclose(pFile_yy);
			}
		}


	}
}
