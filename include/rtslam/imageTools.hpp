/*
 * \file imageTools.hpp
 *
 *     Project: jafar
 *  \date Jul 1, 2010
 *      \author jsola
 *      \ingroup rtslam
 */

#ifndef IMAGETOOLS_HPP_
#define IMAGETOOLS_HPP_

#include "rtslam/gaussian.hpp"
#include "image/Image.hpp"

namespace jafar {
	namespace image {

		template<class Vec, class Sym_mat>
		cv::Rect gauss2rect(const Vec & x, const Sym_mat & P, double n_sigmas = 3.0){
				double dx = n_sigmas * sqrt(P(0,0));
				double dy = n_sigmas * sqrt(P(1,1));
				double xmin = (int) (x(0) - dx);
				double xmax = (int) (x(0) + dx + 0.9999);
				double ymin = (int) (x(1) - dy);
				double ymax = (int) (x(1) + dy + 0.9999);

				cv::Rect rect(xmin, ymin, xmax - xmin + 1, ymax - ymin + 1);
				return rect;
		}

		cv::Rect gauss2rect(const rtslam::Gaussian & g, double sigma = 3.0){
			return gauss2rect(g.x(), g.P(), sigma);
		}

		/**
		 * Extract patch from image.
		 * \param src source image
		 * \param pix central pixel of the patch
		 * \param patch resulting patch
		 */
		void extractPatch(const Image & src, const vec2 & pix, Image & patch){
 			int shift_x = (patch.width()-1)/2;
 			int shift_y = (patch.height()-1)/2;
 			int x_src = (int)(pix(0)-0.5) - shift_x;
 			int y_src = (int)(pix(1)-0.5) - shift_y;
 			src.copy(patch, x_src, y_src, 0, 0, patch.width(), patch.height());
		}

		/**
		 * Crate and extract patch from image.
		 * \param src source image
		 * \param pix central pixel of the patch
		 * \param width patch width in pixels (must be odd number)
		 * \param height patch height in pixels (must be odd number)
		 * \return patch resulting patch
		 */
		Image extractPatch(const Image & src, const vec2 & pix, int width, int height){
			Image patch(width, height, src.depth(), src.colorSpace());
			extractPatch(src, pix, width, height);
			return patch;
		}

	}
}

#endif /* IMAGETOOLS_HPP_ */
