/**
 * \file imageTools.hpp
 *
 *  \date 01/07/2010
 *  \author jsola
 *  \ingroup rtslam
 */

#ifndef IMAGETOOLS_HPP_
#define IMAGETOOLS_HPP_

#include "rtslam/gaussian.hpp"
#include "image/Image.hpp"
#include "jmath/jblas.hpp"

namespace jafar {
	namespace image {

		template<class Vec>
		cv::Rect gauss2rect(const Vec & x, double dx, double dy){
				double xmin = (int) (x(0) - dx);
				double xmax = (int) (x(0) + dx);
				double ymin = (int) (x(1) - dy);
				double ymax = (int) (x(1) + dy);

				cv::Rect rect(xmin, ymin, xmax - xmin + 1, ymax - ymin + 1);
				return rect;
		}

		
		template<class Vec, class Sym_mat>
		cv::Rect gauss2rect(const Vec & x, const Sym_mat & P, double n_sigmas = 3.0){
				double dx = n_sigmas * sqrt(P(0,0));
				double dy = n_sigmas * sqrt(P(1,1));
				return gauss2rect(x, dx, dy);
		}

		cv::Rect gauss2rect(const rtslam::Gaussian & g, double sigma = 3.0);

		/**
		 * Extract patch from image.
		 * \param src source image
		 * \param pix central pixel of the patch
		 * \param patch resulting patch
		 */
		void extractPatch(const Image & src, const jblas::vec2 & pix, Image & patch);

		/**
		 * Crate and extract patch from image.
		 * \param src source image
		 * \param pix central pixel of the patch
		 * \param width patch width in pixels (must be odd number)
		 * \param height patch height in pixels (must be odd number)
		 * \return patch resulting patch
		 */
		Image extractPatch(const Image & src, const jblas::vec2 & pix, int width, int height);
		
	}
}

#endif /* IMAGETOOLS_HPP_ */
