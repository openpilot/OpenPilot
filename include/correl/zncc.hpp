/* $Id$ */

#ifndef _JAFAR_CORREL_ZNCC_HPP_
#define _JAFAR_CORREL_ZNCC_HPP_

#include <fstream>  
#include <iostream>  

#include <kernel/jafarMacro.hpp>
#include "image/Image.hpp"

namespace jafar {
	namespace correl {

		// TODO : SSD, can use integral image for zncc, 
		
		/*! Zncc class.
		*
		* \ingroup correl
		*/
		class Zncc {
			template<int depth, typename worktype, typename bornetype, bornetype borneinf, bornetype bornesup, bool useBornes, bool useWeightMatrix>
			static double computeTpl(image::Image const& im1, image::Image const& im2, float const* weightMatrix = NULL);
		public:

			/**
				This function computes the ZNCC between two images of the same depth in two regions of interest that must have the
				same size, and can apply a spatially distributed weight. It ignores pixels with extremum value
				(eg 0 and 255 for char), and ignores parts outside of images, but requires at least half of pixels not ignored
				to return non 0.
				@param im1 the second image ; the roi can be set (with Image::setROI)
				@param im2 the second image ; the roi can be set (with Image::setROI) and must have the same size than the first one
				@param weightMatrix the weight matrix that must have the same size than the regions of interest ; if NULL not used
				@return ZNCC correlation score between 0 and 1
			*/
			static double compute(image::Image const& im1, image::Image const& im2, float const* weightMatrix = NULL);
			/**
			* This function return the best ZNCC between two images, the second images get rotated
			* @param im1 the first image ; the roi can be set (with set_roi(CvRect))
			* @param im2 the second image ; the roi can be set (with set_roi(CvRect)) and must have the same size than the first one
			* @param rotationStep the step used for the applied rotation
			*/
			static double exploreRotation(image::Image const* im1, image::Image const* im2, int rotationStep);
			
		}; // class Zncc
		


		
	} /* correl */
} /* jafar */

#endif
