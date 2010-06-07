/* $Id$ */

#ifndef _JAFAR_CORREL_EXPLORER_HPP_
#define _JAFAR_CORREL_EXPLORER_HPP_

#include <fstream>  
#include <iostream>  

#include <kernel/jafarMacro.hpp>
#include "correl/zncc.hpp"
#include "jmath/interpol.hpp"

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
			static double exploreTranslation(image::Image const* im1, image::Image* im2, int xmin, int xmax, int xstep, int ymin, int ymax, int ystep, double &xres, double &yres, float const* weightMatrix = NULL);
			
			/**
			This function explores rotation to find the best correlation score
			*/
			static double exploreRotation(image::Image const* im1, image::Image const* im2, int amin, int amax, int astep, double &ares, float const* weightMatrix = NULL);
			
			/**
			This function explores scale to find the best correlation score
			*/
			static double exploreScale(image::Image const* im1, image::Image const* im2, int smin, int smax, int sstep, double &sres, float const* weightMatrix = NULL);
			
			// TODO combinations
			
		}; // class Explorer
		

		#define DO_CORRELATION(im1, im2, weightMatrix, xx, yy, score, best_score, bestx, besty, roi) \
				{ \
				CvRect roi = cvRect(xx-roi.width/2,yy-roi.height/2,roi.width,roi.height); \
				im2->setROI(roi); \
				score = Correl::compute(im1, im2, weightMatrix); \
				results[(yy-(ymin))*sa_w+(xx-(xmin))] = score; \
				if (score > best_score) { best_score = score; bestx = xx; besty = yy; } \
				}
		
		template<class Correl>
		double Explorer<Correl>::exploreTranslation(image::Image const* im1, image::Image* im2, int xmin, int xmax, int xstep, int ymin, int ymax, int ystep, double &xres, double &yres, float const* weightMatrix)
		{
			CvRect roi = im1->getROI();
			double score;
			double best_score = 0.;
			int bestx = -1, besty = -1;
			
			int sa_w = (xmax-xmin+1), sa_h = (ymax-ymin+1); // search area
			double *results = new double[sa_w*sa_h];
			
			// explore
			for(int x = xmin; x <= xmax; x += xstep)
			for(int y = ymin; y <= ymax; y += ystep)
				DO_CORRELATION(im1, im2, weightMatrix, x, y, score, best_score, bestx, besty, roi);

			// refine
			// TODO refine several local maxima
			// TODO refine by dichotomy for large steps ?
			int newbestx = bestx, newbesty = besty;
			for(int x = bestx-xstep+1; x <= bestx+xstep-1; x++)
			for(int y = besty-ystep+1; y <= besty+ystep-1; y++)
			{
				if (x == bestx && y == besty) continue;
				DO_CORRELATION(im1, im2, weightMatrix, x, y, score, best_score, newbestx, newbesty, roi);
			}
			
			// ensure that all values that will be used by interpolation are computed
			int newnewbestx = newbestx, newnewbesty = newbesty;
			if (((newbestx == bestx-xstep+1 || newbestx == bestx+xstep-1) && (newbesty-ymin)%ystep) ||
			    ((newbesty == besty-ystep+1 || newbesty == besty+ystep-1) && (newbestx-xmin)%xstep))
			{
				if (newbestx == bestx-xstep+1) DO_CORRELATION(im1, im2, weightMatrix, newbestx-1, newbesty, score, best_score, newnewbestx, newnewbesty, roi);
				if (newbestx == bestx+xstep-1) DO_CORRELATION(im1, im2, weightMatrix, newbestx+1, newbesty, score, best_score, newnewbestx, newnewbesty, roi);
				if (newbesty == besty-ystep+1) DO_CORRELATION(im1, im2, weightMatrix, newbestx, newbesty-1, score, best_score, newnewbestx, newnewbesty, roi);
				if (newbesty == besty+ystep-1) DO_CORRELATION(im1, im2, weightMatrix, newbestx, newbesty+1, score, best_score, newnewbestx, newnewbesty, roi);
			}
			
			// FIXME maybe should do something if newnewbestx != newbestx or newnewbesty != newbesty
			
			bestx = newbestx;
			besty = newbesty;
			
			// interpolate
			if (bestx > xmin && bestx < xmax)
				jmath::parabolicInterpolation(
					results[(besty-ymin)*sa_w+(bestx-1-xmin)], 
					results[(besty-ymin)*sa_w+(bestx-0-xmin)], 
					results[(besty-ymin)*sa_w+(bestx+1-xmin)], xres);
			else xres = 0;
			xres += bestx;
			
			if (besty > ymin && besty < ymax)
				jmath::parabolicInterpolation(
					results[(besty-1-ymin)*sa_w+(bestx-xmin)], 
					results[(besty-0-ymin)*sa_w+(bestx-xmin)], 
					results[(besty+1-ymin)*sa_w+(bestx-xmin)], yres);
			else yres = 0;
			yres += besty;
			
			// FIXME maybe should interpolate anyway

			delete results;
			return best_score;
		}
			
		template<class Correl>
		double Explorer<Correl>::exploreRotation(image::Image const* im1, image::Image const* im2, int amin, int amax, int astep, double &ares, float const* weightMatrix)
		{
			return Correl::compute(im1, im2, weightMatrix);
			// TODO do the exploration !
		}

		template<class Correl>
		double Explorer<Correl>::exploreScale(image::Image const* im1, image::Image const* im2, int smin, int smax, int sstep, double &sres, float const* weightMatrix)
		{
			return Correl::compute(im1, im2, weightMatrix);
			// TODO do the exploration !
		}
		
	} /* correl */
} /* jafar */

#endif
