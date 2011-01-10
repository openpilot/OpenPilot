/*
 * \file quickHarrisDetector.hpp
 *
 *     Project: jafar
 *  Created on: Jun 4, 2010
 *      \Author: jsola
 *
 *  \ingroup rtslam
 */

#ifndef QUICKHARRISDETECTOR_HPP_
#define QUICKHARRISDETECTOR_HPP_

#include "image/Image.hpp"
#include "image/roi.hpp"

#include "rtslam/featurePoint.hpp"


/*
 * STATUS: in progress, do not use for now
 * Approximate a gaussian mask with integral images, in order to
 * improve precision (getting points closer to the real corner).
 * Disabled for now because it hasn't proved to be really more efficient.
 */
#define GAUSSIAN_MASK_APPROX 0

namespace jafar{
	namespace rtslam{

		/**
		 * Quick Harris detector class.
		 * \ingroup rtslam
		 * \author Joan Sola jsola@laas.fr
		 *
		 * This class detects the strongest Harris point inside a given region of interest.
		 *
		 * The class is called Quick because the algorithm is accelerated using 5 strategies:
		 *  - the use of a simple derivative mask [-1 0 1] that avoids products and minimizes sums.
		 *  - the use of a square convolution mask of fixed amplitude = 1.
		 *  - this mask allows us to compute the convolution via integral images of I_xx, I_xy and I_yy.
		 *  - we extract the best point, so we don't need to perform theresholding and sub-maxima suppression (both expensive).
		 *  - we purposely use small regions of interest.
		 *
		 * Because of simplifications 1. 2. and 3., the result is sub-optimal in the sense
		 * of Harris standards, but it gives strong corner points that can
		 * be tracked by means of correlation (zncc for example).
		 */
		class QuickHarrisDetector {
      public:
        QuickHarrisDetector(int convolutionBoxSize = 5, float threshold = 15.0, float edge = 2.0
#if GAUSSIAN_MASK_APPROX
          , int convolutionGaussianApproxCoeffsNumber = 3
#endif
          );
        ~QuickHarrisDetector();
        virtual bool detectIn(image::Image const& image, feat_img_pnt_ptr_t featPtr, const image::ConvexRoi * roiPtr = 0 );
      private:
        void quickDerivatives(const image::Image & image, image::ConvexRoi & roi);
        bool quickConvolutionWithBestPoint(const image::ConvexRoi & roi, int pixMax[2], float & scoreMax);

        void writeHarrisImagesAsPPM(image::ConvexRoi & roi);

      private:
        int m_derivationSize, m_convolutionSize;
        float m_threshold;
        float m_edge;
        struct HQuickData {
        	int im_x, im_y, im_xx, im_xy, im_yy, im_conv_xx, im_conv_xy, im_conv_yy;
        	int int_xx, int_xy, int_yy;
        	double im_high_curv, im_low_curv;
        };
        HQuickData* m_quickData; // integral image for quick detector.

			private:
				double normCoeff;
				int m_nConvCoeffs;
				int shift_conv;
				int *shift_convs;
				int *convCoeffs;
				int nConvCoeffs;
				bool nConvCoeffsCenter;

				HQuickData** int_upLeft;
				HQuickData** int_upRight;
				HQuickData** int_downLeft;
				HQuickData** int_downRight;
		};
	}
}




#endif /* QUICKHARRISDETECTOR_HPP_ */
