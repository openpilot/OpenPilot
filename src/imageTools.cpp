#include "rtslam/imageTools.hpp"

namespace jafar {
	namespace image {


cv::Rect gauss2rect(const rtslam::Gaussian & g, double sigma){
	return gauss2rect(g.x(), g.P(), sigma);
}

void extractPatch(const Image & src, const jblas::vec2 & pix, Image & patch) {
 			int shift_x = (patch.width()-1)/2;
 			int shift_y = (patch.height()-1)/2;
 			int x_src = (int)(pix(0)-0.5) - shift_x;
 			int y_src = (int)(pix(1)-0.5) - shift_y;
 			src.copy(patch, x_src, y_src, 0, 0, patch.width(), patch.height());
}

Image extractPatch(const Image & src, const jblas::vec2 & pix, int width, int height){
			Image patch(width, height, src.depth(), src.colorSpace());
			extractPatch(src, pix, width, height);
			return patch;
		}

	}
}
