/* $Id$ */

#ifndef _JAFAR_CORREL_EXPLORER_HPP_
#define _JAFAR_CORREL_EXPLORER_HPP_

#include <fstream>  
#include <iostream>  

#include <kernel/jafarMacro.hpp>
#include "correl/zncc.hpp"

namespace jafar {
	namespace correl {

		/*! Explorer class.
		*
		* \ingroup correl
		*/
		template<class Correl>
		class Explorer {
		public:

			/**
			This function explores translation to find the best correlation score
			*/
			static double exploreTranslation(image::Image const* im1, image::Image const* im2, int xmin, int xmax, int xstep, int ymin, int ymax, int ystep, float &xres, float &yres, float const* weightMatrix = NULL);
			
			/**
			This function explores rotation to find the best correlation score
			*/
			static double exploreRotation(image::Image const* im1, image::Image const* im2, int amin, int amax, int astep, float &ares, float const* weightMatrix = NULL);
			
		}; // class Explorer
		

		template<class Correl>
		double Explorer<Correl>::exploreTranslation(image::Image const* im1, image::Image const* im2, int xmin, int xmax, int xstep, int ymin, int ymax, int ystep, float &xres, float &yres, float const* weightMatrix)
		{
			return Correl::compute(im1, im2, weightMatrix);
			// TODO do the exploration !
		}
			
		template<class Correl>
		double Explorer<Correl>::exploreRotation(image::Image const* im1, image::Image const* im2, int amin, int amax, int astep, float &ares, float const* weightMatrix)
		{
			return Correl::compute(im1, im2, weightMatrix);
			// TODO do the exploration !
		}

		
	} /* correl */
} /* jafar */

#endif
