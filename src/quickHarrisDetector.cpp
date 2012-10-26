/*
 * \file quickHarrisDetector.cpp
 * \date 04/06/2010
 * \author jsola
 * \ingroup rtslam
 */

#include "jmath/misc.hpp"
#include "image/roi.hpp"
#include "rtslam/quickHarrisDetector.hpp"


namespace jafar {
	namespace rtslam {
		using namespace std;
		using namespace image;

		QuickHarrisDetector::QuickHarrisDetector(
		    int convolutionBoxSize, float threshold, float edge
#if GAUSSIAN_MASK_APPROX
		    , int convolutionGaussianApproxCoeffsNumber
#endif
			) :
			m_derivationSize(1), m_convolutionSize(convolutionBoxSize),
			    m_threshold(threshold), m_edge(edge)
		{
			shift_conv = (m_convolutionSize - 1) / 2;
#if GAUSSIAN_MASK_APPROX
			m_nConvCoeffs = convolutionGaussianApproxCoeffsNumber;
			
			shift_convs = new int[m_nConvCoeffs];
			for(int i = 0; i < m_nConvCoeffs; ++i)
				shift_convs[i] = (shift_conv+1) * (m_nConvCoeffs-i) / m_nConvCoeffs - 1;
			JFR_ASSERT(shift_convs[0] == shift_conv, "");
			
			nConvCoeffs = m_nConvCoeffs;
			nConvCoeffsCenter = false;
			if (shift_convs[m_nConvCoeffs-1] == 0)
				{ nConvCoeffs--; nConvCoeffsCenter = true; }
				
			// compute convolution coeffs
			convCoeffs = new int[m_nConvCoeffs];
			double *sumConvCoeffs = new double[m_nConvCoeffs];
			for(int k = 0; k < m_nConvCoeffs; ++k) sumConvCoeffs[k] = 0.0;
			normCoeff = 0.0;
			if (m_nConvCoeffs == 1)
				{ convCoeffs[0] = 1; normCoeff = jmath::sqr(shift_conv*2+1); }
			else
			{
				double sigma = (shift_conv+0.5) / 2.0; // we want 2.0 * sigma on the border of the mask (95%)
				for(int i = 0; i <= shift_conv; ++i) for(int j = 0; j <= shift_conv; ++j) for(int k = 0; k < m_nConvCoeffs; ++k)
				{
					if (k != m_nConvCoeffs-1 && (i <= shift_convs[k+1] && j <= shift_convs[k+1])) continue;
					int nUse = 1; if (i != 0) nUse *= 2; if (j != 0) nUse *= 2;
					sumConvCoeffs[k] += nUse * jmath::evalGaussian(sigma, i, j);
					//std::cout << "pix " << i << "," << j << " in " << k << " used " << nUse << " times has coeff " << evalGaussian(sigma, i, j) << std::endl;
					break;
				}
				int lastConvCoeff = 0, lastConvCoeff_tmp;
				for(int k = 0; k < m_nConvCoeffs; ++k)
				{
					int npix = jmath::sqr(shift_convs[k]*2+1);
					if (k != m_nConvCoeffs-1) npix -= jmath::sqr(shift_convs[k+1]*2+1);
					sumConvCoeffs[k] /= npix;

					// quantify
					convCoeffs[k] = jmath::round(sumConvCoeffs[k]*1024);
					//std::cout << "*k " << k << " shift " << shift_convs[k] << " sum " << sumConvCoeffs[k]*npix << " npix " << npix << " coeff " << convCoeffs[k];
					normCoeff += convCoeffs[k] * npix;
					lastConvCoeff_tmp = convCoeffs[k];
					convCoeffs[k] -= lastConvCoeff;
					lastConvCoeff = lastConvCoeff_tmp;
					//std::cout << " compensated " << convCoeffs[k] << std::endl;
				}
			}
			
			delete[] sumConvCoeffs;
			
			int_upLeft = new HQuickData*[nConvCoeffs];
			int_upRight = new HQuickData*[nConvCoeffs];
			int_downLeft = new HQuickData*[nConvCoeffs];
			int_downRight = new HQuickData*[nConvCoeffs];
#else
			normCoeff = jmath::sqr(shift_conv*2+1);
#endif
		}
		
		QuickHarrisDetector::~QuickHarrisDetector()
		{
#if GAUSSIAN_MASK_APPROX
			delete[] shift_convs;
			delete[] convCoeffs;
			
			delete[] int_upLeft;
			delete[] int_upRight;
			delete[] int_downLeft;
			delete[] int_downRight;
#endif
		}

		bool QuickHarrisDetector::detectIn(const jafar::image::Image & image, feat_img_pnt_ptr_t featPtr, const image::ConvexRoi *roiPtr) {
			//	JFR_PRED_ERROR( image.colorSpace() == JfrImage_CS_GRAY, FdetectException, FdetectException::INVALID_COLORSPACE,"QuickHarrisDetector::detectIn image must be of the same colorspace and in Greyscale");
			image::ConvexRoi localRoi;

			if (roiPtr == 0) {
				localRoi.init(cv::Rect(0,0,image.width(),image.height()));
			} else {
				localRoi = *roiPtr;
			}

			int pixBest[2];
			float scoreBest;

			m_quickData = new HQuickData[localRoi.w() * localRoi.h()]; // processing data structure with integral convolution images

			quickDerivatives(image, localRoi);
			bool success =
			    quickConvolutionWithBestPoint(localRoi, pixBest, scoreBest);
			// writeHarrisImagesAsPPM(localRoi);

			delete[] m_quickData;

			if (success) {
				featPtr->setup(pixBest[0]+0.5, pixBest[1]+0.5, scoreBest);
			}
//			cout << "Harris [" << scoreBest << "]: (" << pixBest[0] << "," << pixBest[1] << ")" << endl;
			return success;
		}

		void QuickHarrisDetector::quickDerivatives(
		    const jafar::image::Image & image, image::ConvexRoi & roi) {
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
			int iMin = roi.y() + shift_derv;
			int iMax = roi.y() + roi.h() - shift_derv;
			
			// FIXME take into account possible convex part of roi x(i) and w(i) by moving this into the loop
			int jMin = roi.x() + shift_derv;
			int jMax = roi.x() + roi.w() - shift_derv;

			int i, j; // i, j: image coordinates
			int ri; // ri: vertical roi coordinate
			for (i = iMin; i < iMax; i++) {
				ri = i - roi.y();

				pix_center = image.data() + (i * image.step()) + jMin;
				pix_right = pix_center + 1;
				pix_left = pix_center - 1;
				pix_down = pix_center + image.step();
				pix_up = pix_center - image.step();

				int_center = m_quickData + (ri * roi.w()) + shift_derv;
				int_up = int_center - roi.w();
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


		bool QuickHarrisDetector::quickConvolutionWithBestPoint(
		    const image::ConvexRoi & roi, int pixMax[2], float & scoreMax) {
			
			int shift_derv = 1;
			int shift_derv_conv = shift_derv + shift_conv;

			// bounds
			int riMin = shift_derv_conv;
			int riMax = roi.h() - shift_derv_conv;
			int rjMin = shift_derv_conv;
			int rjMax = roi.w() - shift_derv_conv;

			// data structure pointers
			HQuickData* int_center;
#if GAUSSIAN_MASK_APPROX
#else
			HQuickData* int_upLeft;
			HQuickData* int_upRight;
			HQuickData* int_downLeft;
			HQuickData* int_downRight;
#endif

			float im_low_curv_max = 0.; // maximum smallest eigenvalue
			float im_high_curv_max = 0.;

			int sm, df; // sum and difference
			float sr, corner_ratio;
			int ri, rj; // roi coordinates

			for (ri = riMin; ri < riMax; ri++) {

				int_center = m_quickData + (ri * roi.w()) + rjMin;

#if GAUSSIAN_MASK_APPROX
				for(int i = 0; i < nConvCoeffs; ++i)
				{
					int_upLeft[i] = int_center - (shift_convs[i] * (roi.w()+1));
					int_upRight[i] = int_center - (shift_convs[i] * (roi.w()-1));
					int_downLeft[i] = int_center + (shift_convs[i] * (roi.w()-1));
					int_downRight[i] = int_center + (shift_convs[i] * (roi.w()+1));
				}
#else
				int_upLeft = int_center - (shift_conv * roi.w()) - shift_conv;
				int_upRight = int_center - (shift_conv * roi.w()) + shift_conv;
				int_downLeft = int_center + (shift_conv * roi.w()) - shift_conv;
				int_downRight = int_center + (shift_conv * roi.w()) + shift_conv;
#endif

				for (rj = rjMin; rj < rjMax; rj++) {

#if GAUSSIAN_MASK_APPROX
					int_center->im_conv_xx = int_center->im_conv_xy = int_center->im_conv_yy = 0.;
					for(int i = 0; i < nConvCoeffs; ++i)
					{
						int_center->im_conv_xx += convCoeffs[i]*(
							int_downRight[i]->int_xx - int_upRight[i]->int_xx -
							int_downLeft[i]->int_xx + int_upLeft[i]->int_xx);
						int_center->im_conv_xy += convCoeffs[i]*(
							int_downRight[i]->int_xy - int_upRight[i]->int_xy -
							int_downLeft[i]->int_xy + int_upLeft[i]->int_xy);
						int_center->im_conv_yy += convCoeffs[i]*(
							int_downRight[i]->int_yy - int_upRight[i]->int_yy -
							int_downLeft[i]->int_yy + int_upLeft[i]->int_yy);
					} 
					if (nConvCoeffsCenter)
					{
						int_center->im_conv_xx += convCoeffs[nConvCoeffs]*int_center->im_xx;
						int_center->im_conv_xy += convCoeffs[nConvCoeffs]*int_center->im_xy;
						int_center->im_conv_yy += convCoeffs[nConvCoeffs]*int_center->im_yy;
					}
#else
					int_center->im_conv_xx = int_downRight->int_xx - int_upRight->int_xx
					    - int_downLeft->int_xx + int_upLeft->int_xx;
					int_center->im_conv_xy = int_downRight->int_xy - int_upRight->int_xy
					    - int_downLeft->int_xy + int_upLeft->int_xy;
					int_center->im_conv_yy = int_downRight->int_yy - int_upRight->int_yy
					    - int_downLeft->int_yy + int_upLeft->int_yy;
#endif
					
					// get eigenvalues: EIG/eig = I_xx + I_yy +/- sqrt((Ixx - I_yy)^2 + 4*I_xy^2)
					sm = int_center->im_conv_xx + int_center->im_conv_yy;
					df = int_center->im_conv_xx - int_center->im_conv_yy;

					sr = sqrt((double) ((double) df * (double) df + 4
					    * ((double) int_center->im_conv_xy)
					    * ((double) int_center->im_conv_xy)));

					int_center->im_high_curv = (double) sm + sr; // Smallest eigenvalue.
					int_center->im_low_curv = (double) sm - sr; // Largest eigenvalue.

					if (int_center->im_low_curv!=0.)
					// detect and write pixel corresponding to strongest corner, with score.
					corner_ratio = int_center->im_high_curv / int_center->im_low_curv; // CAUTION should be high/low
					else corner_ratio=m_edge;

					if (corner_ratio < m_edge) {
						if (int_center->im_low_curv > im_low_curv_max) {
							im_low_curv_max = int_center->im_low_curv;
							im_high_curv_max = int_center->im_high_curv;
							pixMax[0] = roi.x() + rj;
							pixMax[1] = roi.y() + ri;
						}
					}

					int_center++;
#if GAUSSIAN_MASK_APPROX
					for(int i = 0; i < nConvCoeffs; ++i)
					{
						int_upLeft[i]++;
						int_upRight[i]++;
						int_downLeft[i]++;
						int_downRight[i]++;
					}
#else
					int_upLeft++;
					int_upRight++;
					int_downLeft++;
					int_downRight++;
#endif

				}
			}

			// normalized score: over the size of the derivative and convolution windows
			scoreMax = sqrt(im_low_curv_max / normCoeff);

			//std::cout << "- at " << pixMax[0] << "," << pixMax[1] << " : high "  << im_high_curv_max << " low " << im_low_curv_max << " ; ratio " << im_high_curv_max/im_low_curv_max << " score " << scoreMax << std::endl;
			
#if FILTER_VIRTUAL_POINTS
/*
accept:
_|_   (4)        |_      _/    _     (2)      _   _   _  (1 or 2)
 |                              \             \   /
reject:
  |_     (3)    .  (0)
  |
*/
			
			int_center = m_quickData + ((pixMax[1]-roi.y()) * roi.w()) + (pixMax[0]-roi.x());
			
			int indexes[8][2] = {{-1,-1},{-1,0},{-1,1},{0,1},{1,1},{1,0},{1,-1},{0,-1}};
			int radius = m_convolutionSize/2;
			for(int i = 0; i < 8; ++i) for(int j = 0; j < 2; ++j) indexes[i][j] *= radius;
			double thres = im_low_curv_max * 0.75;
			
			int npeaks = 0;
			bool lastpeaked = false;
			bool firstpeaked = false;
			for(int i = 0; i < 8; ++i)
			{
				int_upLeft = int_center + indexes[i][1] * roi.w() + indexes[i][0];
				
				if (int_upLeft->im_high_curv > thres)
				{
					if (i == 0) firstpeaked = true; else
					if (i == 7) lastpeaked = lastpeaked || firstpeaked;
					
					if (!lastpeaked) npeaks++; lastpeaked = true;
					
				} else
					lastpeaked = false;
std::cout << "  high " << int_upLeft->im_high_curv << " "  << " low " << int_upLeft->im_low_curv << std::endl;
			}
			bool ok = (npeaks == 1 || npeaks == 2 || npeaks == 4);
			
			std::cout << "  high " << int_center->im_high_curv << " low " << int_center->im_low_curv << " npeaks " << npeaks << " ok " << ok << std::endl;
#endif
			return (scoreMax > m_threshold);

		}

		void QuickHarrisDetector::writeHarrisImagesAsPPM(image::ConvexRoi & roi) {
			FILE *pFile_x = fopen("/home/jsola/im_x.ppm", "w");
			FILE *pFile_y = fopen("/home/jsola/im_y.ppm", "w");
			FILE *pFile_xx = fopen("/home/jsola/im_xx.ppm", "w");
			FILE *pFile_xy = fopen("/home/jsola/im_xy.ppm", "w");
			FILE *pFile_yy = fopen("/home/jsola/im_yy.ppm", "w");
			FILE *pFile_min = fopen("/home/jsola/im_min.ppm", "w");
			FILE *pFile_max = fopen("/home/jsola/im_max.ppm", "w");
			HQuickData * ptr = m_quickData;
			if (pFile_x != NULL) {
				fprintf(pFile_x, "P2\n%d %d\n255\n", roi.w(), roi.h());
				fprintf(pFile_y, "P2\n%d %d\n255\n", roi.w(), roi.h());
				fprintf(pFile_xx, "P2\n%d %d\n255\n", roi.w(), roi.h());
				fprintf(pFile_xy, "P2\n%d %d\n255\n", roi.w(), roi.h());
				fprintf(pFile_yy, "P2\n%d %d\n255\n", roi.w(), roi.h());
				fprintf(pFile_min, "P2\n%d %d\n255\n", roi.w(), roi.h());
				fprintf(pFile_max, "P2\n%d %d\n255\n", roi.w(), roi.h());
				for (int i = 0; i < (roi.h() * roi.w()); i++) {
					fprintf(pFile_x, "%d ", 128 + (ptr->im_x) / 2);
					fprintf(pFile_y, "%d ", 128 + (ptr->im_y) / 2);
					fprintf(pFile_xx, "%d ", 128 + (ptr->im_conv_xx) / 2048);
					fprintf(pFile_xy, "%d ", 128 + (ptr->im_conv_xy) / 2048);
					fprintf(pFile_yy, "%d ", 128 + (ptr->im_conv_yy) / 2048);
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
