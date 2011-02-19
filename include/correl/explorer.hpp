/* $Id$ */

#ifndef _JAFAR_CORREL_EXPLORER_HPP_
#define _JAFAR_CORREL_EXPLORER_HPP_

#include <fstream>  
#include <iostream>  

#include "kernel/jafarMacro.hpp"
#include "correl/zncc.hpp"
#include "jmath/interpol.hpp"
#include "image/roi.hpp"

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
		

		class FastTranslationMatcherZncc {
			protected:
				inline void doCorrelationFast(image::Image const& im1, image::Image& im2, int x, int y,
					cv::Rect const &patch_in_im1, cv::Rect const &hpatch_in_im1, int& bestx, int& besty, double& best_score, 
					cv::Rect const &int_in_im2, double sum1, double sumSqr1, cv::Mat const &sum2_, cv::Mat const &sumSqr2_, 
					double minScore, int partialLine, double partialSum1, double partialSumSqr1, 
					cv::Rect const &result_in_im2, cv::Mat *results);
				inline void doCorrelationRobust(image::Image const& im1, image::Image& im2, int x, int y,
					cv::Rect const &patch_in_im1, cv::Rect const &hpatch_in_im1, int& bestx, int& besty, double& best_score,
					cv::Rect const &result_in_im2, bool force, cv::Mat *results);
				
				inline void rawExploreTranslationFast(image::Image const& im1, image::Image& im2, 
					image::ConvexRoi const &roi_in_im2, int &bestx, int &besty, double &best_score, 
					double minScore, int partialLine, cv::Rect const &result_in_im2, cv::Mat *results);
				inline void rawExploreTranslationRobust(image::Image const& im1, image::Image& im2, 
					image::ConvexRoi const &roi_in_im2, int &bestx, int &besty, double &best_score, 
					cv::Rect const &result_in_im2, cv::Mat *results);
					
			private:
				double minScore;
				double partialPosition;
				
			public:

				/**
				@param minScore the minimum score under which you don't care about the result
				@param partialPosition at line (int)(im1.height*partialPosition), a test is is done to know if it is still possible to improve the score
				*/
				FastTranslationMatcherZncc(double minScore, double partialPosition):
					minScore(minScore), partialPosition(partialPosition) {}
				
				/**
				This function explores translation to find the best correlation score
				@param im1 the image representing the patch that is searched in im2 (you can set the ROI)
				@param im2 the image where the patch represented by im1 is searched
				@param roi the search region in im2 (region where the center of im1 can be)
				@param xres,yres the result position in im2 of maximum correlation
				@param results either NULL, or a pointer to a cv::Mat of type CV_64FC1 and size (roi.width()+2) * (roi.height()+2).
				It contains the result score of the correlation, in [-1,1], with special values -4 (never computed), 
				-3 (no computation because too few pixels) and -2 (computation aborted because it wouldn't improve the best score)
				@return score of best correlation
				*/
				double match(image::Image const& im1, image::Image const& im2_, image::ConvexRoi const &roi, double &xres, 
				             double &xstd, double &ystd, double &yres, cv::Mat **results_ = NULL);
		};

		
		
#if 0



		class FastTranslationMatcherZncc {
			protected:
				inline void doCorrelationFast(image::Image const& im1, image::Image& im2, int x, int y, int xmin, int ymin, 
					int width, int height, int hwidth, int hheight, int& bestx, int& besty, double& best_score, cv::Rect& intRoi, 
					double sum1, double sumSqr1, cv::Mat& sum2_, cv::Mat& sumSqr2_, double minScore, int partialLine, double partialSum1,
					double partialSumSqr1, cv::Mat *results);
				inline void doCorrelationRobust(image::Image const& im1, image::Image& im2, int x, int y, int xmin, int ymin, 
					int width, int height, int hwidth, int hheight, int& bestx, int& besty, double& best_score, cv::Mat *results);
				
				inline void rawExploreTranslationFast(image::Image const& im1, image::Image& im2, 
					image::ConvexRoi const &iroi, image::ConvexRoi const &croi, int &bestx, int &besty, double &best_score, 
					double minScore, int partialLine, cv::Mat *results);
				inline void rawExploreTranslationRobust(image::Image const& im1, image::Image& im2, 
					image::ConvexRoi const &iroi, image::ConvexRoi const &croi, int &bestx, int &besty, double &best_score, 
					cv::Mat *results);
					
			private:
				double minScore;
				double partialPosition;
				
			public:

				/**
				@param minScore the minimum score under which you don't care about the result
				@param partialPosition at line (int)(im1.height*partialPosition), a test is is done to know if it is still possible to improve the score
				*/
				FastTranslationMatcherZncc(double minScore, double partialPosition):
					minScore(minScore), partialPosition(partialPosition) {}
				
				/**
				This function explores translation to find the best correlation score
				@param im1 the image representing the patch that is searched in im2 (you can set the ROI)
				@param im2 the image where the patch represented by im1 is searched
				@param roi the search region in im2 (region where the center of im1 can be)
				@param xres,yres the result position in im2 of maximum correlation
				@param results either NULL, or a pointer to a cv::Mat of type CV_64FC1 and size (roi.width()+2) * (roi.height()+2).
				It contains the result score of the correlation, in [-1,1], with special values -4 (never computed), 
				-3 (no computation because too few pixels) and -2 (computation aborted because it wouldn't improve the best score)
				@return score of best correlation
				*/
				double match(image::Image const& im1, image::Image const& im2_, image::ConvexRoi const &roi, double &xres, double &yres, cv::Mat **results_ = NULL);
		};




#endif
		
		
		
		
		
		
		
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
		
		
		
		
		
		
		
		
		
		
		
		#define RESULTS(y,x) results[((y)-(ymin-1))*sa_w+((x)-(xmin-1))]
		#define DO_CORRELATION(im1, im2, weightMatrix, xx, yy, score, best_score, bestx, besty, roi) \
				{ \
				cv::Rect roi2 = cv::Rect((xx)-roi.width/2,(yy)-roi.height/2,roi.width,roi.height); \
				im2.setROI(roi2); \
				score = Correl::compute(im1, im2, weightMatrix); \
				RESULTS(yy,xx) = score; \
				if (score > best_score) { best_score = score; bestx = xx; besty = yy; } \
				}
		
//std::cout << "correl " << xx <<"," << yy << " " << score <<std::endl;
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
			if (sa_w < 5) xstep = 1; if (sa_h < 5) ystep = 1;
			int nresults = (sa_w+2)*(sa_h+2);
			double *results = new double[nresults]; // add 1 border for interpolation
			for(int i = 0; i < nresults; i++) results[i] = -1e6;
			
			// explore
			for(int y = ymin; y <= ymax; y += ystep)
			for(int x = xmin; x <= xmax; x += xstep)
				DO_CORRELATION(im1, im2, weightMatrix, x, y, score, best_score, bestx, besty, roi);

			// refine
// JFR_DEBUG("refine (" << bestx << "," << besty << " " << best_score << ")");
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
// JFR_DEBUG("extra interpol (" << newbestx << "," << newbesty << " " << best_score << ")");
			do {
				newbestx = newnewbestx, newbesty = newnewbesty;
				if (newbestx>0 && RESULTS(newbesty,newbestx-1)<-1e5)
					DO_CORRELATION(im1, im2, weightMatrix, newbestx-1, newbesty, score, best_score, newnewbestx, newnewbesty, roi);
				if (newbestx<im2.width()-1 && RESULTS(newbesty,newbestx+1)<-1e5)
					DO_CORRELATION(im1, im2, weightMatrix, newbestx+1, newbesty, score, best_score, newnewbestx, newnewbesty, roi);
				if (newbesty>0 && RESULTS(newbesty-1,newbestx)<-1e5)
					DO_CORRELATION(im1, im2, weightMatrix, newbestx, newbesty-1, score, best_score, newnewbestx, newnewbesty, roi);
				if (newbesty<im2.height()-1 && RESULTS(newbesty+1,newbestx)<-1e5)
					DO_CORRELATION(im1, im2, weightMatrix, newbestx, newbesty+1, score, best_score, newnewbestx, newnewbesty, roi);
			} while (newbestx != newnewbestx || newbesty != newnewbesty);
			// FIXME this could go out of bounds
// JFR_DEBUG("final : " << newnewbestx << "," << newnewbesty << " " << best_score);
			
			bestx = newbestx;
			besty = newbesty;
			
			// TODO interpolate the score as well
			// interpolate x
			
			double a1 = RESULTS(besty,bestx-1), a2 = RESULTS(besty,bestx-0), a3 = RESULTS(besty,bestx+1);
			if (a1 > -1e5 && a3 > -1e5) jmath::parabolicInterpolation(a1,a2,a3, xres); else xres = 0;
// JFR_DEBUG("interpolating " << a1 << " " << a2 << " " << a3 << " gives shift " << xres << " plus " << bestx+0.5);
			xres += bestx+0.5;
			// interpolate y
			a1 = RESULTS(besty-1,bestx), a2 = RESULTS(besty-0,bestx), a3 = RESULTS(besty+1,bestx);
			if (a1 > -1e5 && a3 > -1e5) jmath::parabolicInterpolation(a1,a2,a3, yres); else yres = 0;
// JFR_DEBUG("interpolating " << a1 << " " << a2 << " " << a3 << " gives shift " << yres << " plus " << besty+0.5);
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
