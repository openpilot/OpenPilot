/*
 * quickHarrisDetector.hpp
 *
 *     Project: jafar
 *  Created on: Jun 4, 2010
 *      Author: jsola
 */

#ifndef QUICKHARRISDETECTOR_HPP_
#define QUICKHARRISDETECTOR_HPP_

#include "image/Image.hpp"
#include "image/roi.hpp"

#include "rtslam/featurePoint.hpp"

namespace jafar{
	namespace rtslam{

		class QuickHarrisDetector {
      public:
        QuickHarrisDetector(float convolutionBoxSize = 5, float threshold = 0.0);
        virtual bool detectIn(jafar::image::Image const& image, featurepoint_ptr_t featPtr, jafar::image::ROI * roiPtr = 0 );
      private:
        void quickDerivatives(const jafar::image::Image & image, jafar::image::ROI & roi);
        bool quickConvolutionWithBestPoint(const jafar::image::ROI & roi, int pixMax[2], float & scoreMax);


        void writeHarrisImagesAsPPM(jafar::image::ROI & roi);

      private:
        float m_threshold;
        int m_derivationSize, m_convolutionSize;
        struct HQuickData {
        	int im_x, im_y, im_xx, im_xy, im_yy, im_conv_xx, im_conv_xy, im_conv_yy;
        	int int_xx, int_xy, int_yy;
        	double im_high_curv, im_low_curv;
        };
        HQuickData* m_quickData; // integral image for quick detector.

		};
	}
}




#endif /* QUICKHARRISDETECTOR_HPP_ */
