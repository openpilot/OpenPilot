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
			@param im1 the image representing the patch that is searched in im2 (you can set the ROI)
			@param im2 the image where the patch represented by im1 is searched
			@param xmin,xmax the search range along x axis in im2
			@param xstep the search step along x axis
			@param ymin,ymax the search range along x axis in im2
			@param xstep the search step along y axis
			@param xres,yres the result position in im2 of maximum correlation
			@param weightMatrix the optional weight matrix applied on correlation, must be of same size than im1 patch size ; if NULL not used
			@return score of best correlation
			*/
			static double exploreTranslation(image::Image const& im1, image::Image const& im2, int xmin, int xmax, int xstep, int ymin, int ymax, int ystep, double &xres, double &yres, float const* weightMatrix = NULL);
			
			/**
			This function explores rotation to find the best correlation score
			*/
			static double exploreRotation(image::Image const& im1, image::Image const& im2, int amin, int amax, int astep, double &ares, float const* weightMatrix = NULL);
			
			/**
			This function explores scale to find the best correlation score
			*/
			static double exploreScale(image::Image const& im1, image::Image const& im2, int smin, int smax, int sstep, double &sres, float const* weightMatrix = NULL);
			
			// TODO combinations
			
		}; // class Explorer
		

		//std::cout << xx << "," << yy << ":" << score << std::endl; \

		#define RESULTS(y,x) results[((y)-(ymin-1))*sa_w+((x)-(xmin-1))]
		#define DO_CORRELATION(im1, im2, weightMatrix, xx, yy, score, best_score, bestx, besty, roi) \
				{ \
				cv::Rect roi2 = cv::Rect((xx)-roi.width/2,(yy)-roi.height/2,roi.width,roi.height); \
				im2.setROI(roi2); \
				score = Correl::compute(im1, im2, weightMatrix); \
				RESULTS(yy,xx) = score; \
				if (score > best_score) { best_score = score; bestx = xx; besty = yy; } \
				}
		
		template<class Correl>
		double Explorer<Correl>::exploreTranslation(image::Image const& im1, image::Image const& im2_, int xmin, int xmax, int xstep, int ymin, int ymax, int ystep, double &xres, double &yres, float const* weightMatrix)
		{
			cv::Rect roi = im1.getROI();
//			image::Image im2(im2_, cv::Rect(0,0,im2_.width(),im2_.height()));
			image::Image im2(im2_);
			double score;
			double best_score = -1.;
			int bestx = -1, besty = -1;
			
			if (xmin < 0) xmin = 0; if (xmax >= im2.width ()) xmax = im2.width ()-1;
			if (ymin < 0) ymin = 0; if (ymax >= im2.height()) ymax = im2.height()-1;
			
			int sa_w = (xmax-xmin+1), sa_h = (ymax-ymin+1); // search area
			int nresults = (sa_w+2)*(sa_h+2);
			double *results = new double[nresults]; // add 1 border for interpolation
			for(int i = 0; i < nresults; i++) results[i] = -1e6;
			
			// explore
			for(int y = ymin; y <= ymax; y += ystep)
			for(int x = xmin; x <= xmax; x += xstep)
				DO_CORRELATION(im1, im2, weightMatrix, x, y, score, best_score, bestx, besty, roi);

			// refine
			// TODO refine several local maxima
			// TODO refine by dichotomy for large steps ?
			int newbestx = bestx, newbesty = besty;
			for(int y = besty-ystep+1; y <= besty+ystep-1; y++)
			for(int x = bestx-xstep+1; x <= bestx+xstep-1; x++)
			{
				if (x == bestx && y == besty) continue;
				DO_CORRELATION(im1, im2, weightMatrix, x, y, score, best_score, newbestx, newbesty, roi);
			}
			
			// ensure that all values that will be used by interpolation are computed
			int newnewbestx = newbestx, newnewbesty = newbesty;
/*			if (((newbestx == bestx-xstep+1 || newbestx == bestx+xstep-1) && (newbesty-ymin)%ystep) ||
			    ((newbesty == besty-ystep+1 || newbesty == besty+ystep-1) && (newbestx-xmin)%xstep))
			{
				if (newbestx == bestx-xstep+1) DO_CORRELATION(im1, im2, weightMatrix, newbestx-1, newbesty, score, best_score, newnewbestx, newnewbesty, roi);
				if (newbestx == bestx+xstep-1) DO_CORRELATION(im1, im2, weightMatrix, newbestx+1, newbesty, score, best_score, newnewbestx, newnewbesty, roi);
				if (newbesty == besty-ystep+1) DO_CORRELATION(im1, im2, weightMatrix, newbestx, newbesty-1, score, best_score, newnewbestx, newnewbesty, roi);
				if (newbesty == besty+ystep-1) DO_CORRELATION(im1, im2, weightMatrix, newbestx, newbesty+1, score, best_score, newnewbestx, newnewbesty, roi);
			}*/
			if (newbestx>0 && RESULTS(newbesty,newbestx-1)<-1e5)
				DO_CORRELATION(im1, im2, weightMatrix, newbestx-1, newbesty, score, best_score, newnewbestx, newnewbesty, roi);
			if (newbestx<im2.width()-1 && RESULTS(newbesty,newbestx+1)<-1e5)
				DO_CORRELATION(im1, im2, weightMatrix, newbestx+1, newbesty, score, best_score, newnewbestx, newnewbesty, roi);
			if (newbesty>0 && RESULTS(newbesty-1,newbestx)<-1e5)
				DO_CORRELATION(im1, im2, weightMatrix, newbestx, newbesty-1, score, best_score, newnewbestx, newnewbesty, roi);
			if (newbesty<im2.height()-1 && RESULTS(newbesty+1,newbestx)<-1e5)
				DO_CORRELATION(im1, im2, weightMatrix, newbestx, newbesty+1, score, best_score, newnewbestx, newnewbesty, roi);
			
			// FIXME maybe should do something if newnewbestx != newbestx or newnewbesty != newbesty ... will interpolation be right ?
			
			bestx = newbestx;
			besty = newbesty;
			
			// interpolate x
			double a1 = RESULTS(besty,bestx-1), a2 = RESULTS(besty,bestx-0), a3 = RESULTS(besty,bestx+1);
			if (a1 > -1e5 && a3 > -1e5) jmath::parabolicInterpolation(a1,a2,a3, xres); else xres = 0;
			xres += bestx+0.5;
			// interpolate y
			a1 = RESULTS(besty-1,bestx), a2 = RESULTS(besty-0,bestx), a3 = RESULTS(besty+1,bestx);
			if (a1 > -1e5 && a3 > -1e5) jmath::parabolicInterpolation(a1,a2,a3, yres); else yres = 0;
			yres += besty+0.5;
			
			delete[] results;
			return best_score;
		}
			
		template<class Correl>
		double Explorer<Correl>::exploreRotation(image::Image const& im1, image::Image const& im2, int amin, int amax, int astep, double &ares, float const* weightMatrix)
		{
			return Correl::compute(im1, im2, weightMatrix);
			// TODO do the exploration !
		}

		template<class Correl>
		double Explorer<Correl>::exploreScale(image::Image const& im1, image::Image const& im2, int smin, int smax, int sstep, double &sres, float const* weightMatrix)
		{
			return Correl::compute(im1, im2, weightMatrix);
			// TODO do the exploration !
		}
		
	} /* correl */
} /* jafar */

#endif
