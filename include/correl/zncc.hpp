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
				This function computes the ZNCC between two images of the same depth in two regions of interest that can be of different
				size if on the border of theimage, and can apply a spatially distributed weight. It ignores pixels with extremum value
				(eg 0 and 255 for char), and ignores parts outside of images, but requires at least half of pixels not ignored
				to return non 0.
				@param im1 the second image ; the roi can be set (with Image::setROI)
				@param im2 the second image ; the roi can be set (with Image::setROI) and must have the same size than the first one
				@param weightMatrix the weight matrix that must have the same size than the regions of interest ; if NULL not used
				@return ZNCC correlation score between 0 and 1
			*/
			static double compute(image::Image const& im1, image::Image const& im2, float const* weightMatrix = NULL);
			static double compute8noborne(image::Image const& im1, image::Image const& im2);
			/**
			* This function return the best ZNCC between two images, the second images get rotated
			* @param im1 the first image ; the roi can be set (with set_roi(CvRect))
			* @param im2 the second image ; the roi can be set (with set_roi(CvRect)) and must have the same size than the first one
			* @param rotationStep the step used for the applied rotation
			*/
			static double exploreRotation(image::Image const* im1, image::Image const* im2, int rotationStep);
			
		}; // class Zncc
		

		/**
		Compared to Zncc, remove some robustness features, but add a lot of optimizations
		*/
		class FastZncc {
			template<int depth, typename worktype>
			static inline double computeTpl(image::Image const& im1, image::Image const& im2, double sum1, double sumSqr1, double sum2, double sumSqr2, double minScore, int partialLine, double partialSum1, double partialSumSqr1, double partialSum2, double partialSumSqr2);
		public:

			/**
			This function computes the ZNCC between two images of the same depth in two regions of interest that must have the
			same size, and use precomputed sums and squaredSums (that should provide from integral images), and some other
			optimizations.
			@param im1_ the first image
			@param im2_ the second image
			@param sum1 the sum of all the pixels of the first image
			@param sumSqr1 the sum of the square of all the pixels of the first image
			@param sum2 the sum of all the pixels of the second image
			@param sumSqr2 the sum of the square of all the pixels of the second image
			@param minScore the minimum score for which we want a result (if the real score is below, 0 will be returned)
			@param halfLine the line number at which the half test will be done: if in the best case it is 
			impossible to reach minScore, then correlation is stopped and 0 is returned
			@param halfSumSqr1 the sum of the square of the pixels of the first image from halfLine+1 to the end of the image
			*/
			static inline double compute(image::Image const& im1, image::Image const& im2, double sum1, double sumSqr1, double sum2, double sumSqr2, double minScore, int partialLine, double partialSum1, double partialSumSqr1, double partialSum2, double partialSumSqr2);
			
			/**
			This version can more easily be inlined, which can increase the speed by 10%, but only manages CV_8U images
			*/
			static inline double computeChar(image::Image const& im1, image::Image const& im2, double sum1, double sumSqr1, double sum2, double sumSqr2, double minScore, int partialLine, double partialSum1, double partialSumSqr1, double partialSum2, double partialSumSqr2)
			{
				JFR_PRECOND(im1.depth() == im2.depth(), "The depth of both images is different");
				JFR_PRECOND(im1.depth() == CV_8U, "The depth of images is not 8 bits");
				return computeTpl<CV_8U, uint8_t>(im1,im2,sum1,sumSqr1,sum2,sumSqr2,minScore,partialLine,partialSum1,partialSumSqr1,partialSum2,partialSumSqr2);
			}
		}; // class Zncc



		///////////////////////////////////////////////////////////////////////////
		// IMPLEMENTATIONS

		template<int depth, typename worktype>
		double FastZncc::computeTpl(image::Image const& im1, image::Image const& im2, double sum1, double sumSqr1, double sum2, double sumSqr2, double minScore, int partialLine, double partialSum1, double partialSumSqr1, double partialSum2, double partialSumSqr2)
		{
			// preconds
			JFR_PRECOND( im1.depth() == depth, "Image 1 depth is different from the template parameter" );
			JFR_PRECOND( im2.depth() == depth, "Image 2 depth is different from the template parameter" );
			JFR_PRECOND( im1.channels() == im2.channels(), "The channels number of both images are different" );
			JFR_PRECOND( im1.size()  == im2.size() , "The size of images or roi are different (" << im1.width() << "," << im1.height() << " != " << im2.width() << "," << im2.height() << ")" );
			
// std::cout << "zncc with sum1 " << sum1 << ", sumSqr1 " << sumSqr1 << ", sum2 " << sum2 << ", sumSqr2 " << sumSqr2 << ", halfLine " << halfLine << std::endl;
			
			int height = im1.height();
			int width = im1.width();
			int step1 = im1.step1()/sizeof(worktype) - width;
			int step2 = im2.step1()/sizeof(worktype) - width;
			
			int count = height*width;
			double mean1 = sum1/(double)count;
			double mean2 = sum2/(double)count;
			double mean12 = mean1*mean2;
			double sigma12 = sqrt((sumSqr1 - count*mean1*mean1)*(sumSqr2 - count*mean2*mean2));
			
			worktype const* im1ptr = reinterpret_cast<worktype const*>(im1.data());
			worktype const* im2ptr = reinterpret_cast<worktype const*>(im2.data());
			
			// start the loops
			double zncc_sum = 0.;
			double best_score1 = 1.0;
			//double best_score2 = 1.0;
			for(int i = 0; i < height; ++i, im1ptr += step1, im2ptr += step2) 
			{
				for(int j = 0; j < width; ++j) 
					zncc_sum += *(im1ptr++) * *(im2ptr++);

				if (i == partialLine) // if ((((zncc_sum+halfSumSqr1)/count - mean12) / sigma12) < minScore) return -1;
				{
					int partialCount = (height-partialLine-1)*width;
					double current = (zncc_sum - mean2*(sum1-partialSum1) - mean1*(sum2-partialSum2) + (count-partialCount)*mean12);
					// this first bound seems more efficient at the beginning, at height/4
					double best_remain1 = sqrt((partialSumSqr2 + partialCount*mean2*mean2 - 2*mean2*partialSum2) *
					                           (partialSumSqr1 + partialCount*mean1*mean1 - 2*mean1*partialSum1));
					// this second bound seems globally less efficient, but more efficient at the end, after height/2
//					double best_remain2 = sqrt(partialSumSqr1*partialSumSqr2) - mean2*partialSum1 - mean1*partialSum2 + partialCount*mean12;
					
					best_score1 = (current + best_remain1) / (sigma12);
//					best_score2 = (current + best_remain2) / (sigma12);
					if (best_score1 < minScore) return -2;
//					if (best_score2 < minScore) return -1;
				}
				//if (i == halfLineB) if ((((zncc_sum+halfSumSqr1B)/count - mean12) / sigma12) < minScore) return -1;
			}
			
			// finish
// std::cout << "fast: zncc_sum " << zncc_sum << ", count " << count << ", mean12 " << mean12 << ", sigma12 " << sigma12 << std::endl;
			double score = (sigma12 < 1e-6 ? -1 : (zncc_sum - count*mean12) / (sigma12));
			
//std::cout << im2.getROI() << ": score " << score << ", estimated at " << partialLine << ": " << best_score1 << " / " << best_score2 << std::endl;
//JFR_ASSERT(score <= best_score1+1e-6, "score1 est not upper bound");
//JFR_ASSERT(score <= best_score2+1e-6, "score2 est not upper bound");
//JFR_ASSERT(best_score1 <= 1+1e-6, "score1 est greater than 1");
////JFR_ASSERT(best_score2 <= 1+1e-6, "score2 est greater than 1");


			JFR_ASSERT(score >= -1.01, "");
			return score;
		}


		double FastZncc::compute(image::Image const& im1, image::Image const& im2, double sum1, double sumSqr1, double sum2, double sumSqr2, double minScore, int partialLine, double partialSum1, double partialSumSqr1, double partialSum2, double partialSumSqr2)
		{
			JFR_PRECOND(im1.depth() == im2.depth(), "The depth of both images is different");
			switch(im1.depth())
			{
	// 			case CV_1U:
	// 				if (weightMatrix == NULL)
	// 					return computeTpl<CV_1U, bool,bool,0,1,true,false>(im1,im2);
	// 				else
	// 					return computeTpl<CV_1U, bool,bool,0,1,true,true>(im1,im2,weightMatrix);
				case CV_8U:
						return computeTpl<CV_8U, uint8_t>(im1,im2,sum1,sumSqr1,sum2,sumSqr2,minScore,partialLine,partialSum1,partialSumSqr1,partialSum2,partialSumSqr2);
				case CV_8S:
						return computeTpl<CV_8S, int8_t>(im1,im2,sum1,sumSqr1,sum2,sumSqr2,minScore,partialLine,partialSum1,partialSumSqr1,partialSum2,partialSumSqr2);
				case CV_16U:
						return computeTpl<CV_16U, uint16_t>(im1,im2,sum1,sumSqr1,sum2,sumSqr2,minScore,partialLine,partialSum1,partialSumSqr1,partialSum2,partialSumSqr2);
				case CV_16S:
						return computeTpl<CV_16S, int16_t>(im1,im2,sum1,sumSqr1,sum2,sumSqr2,minScore,partialLine,partialSum1,partialSumSqr1,partialSum2,partialSumSqr2);
				case CV_32F:
						return computeTpl<CV_32F, float>(im1,im2,sum1,sumSqr1,sum2,sumSqr2,minScore,partialLine,partialSum1,partialSumSqr1,partialSum2,partialSumSqr2);
				case CV_64F:
						return computeTpl<CV_64F, double>(im1,im2,sum1,sumSqr1,sum2,sumSqr2,minScore,partialLine,partialSum1,partialSumSqr1,partialSum2,partialSumSqr2);
				default:
					JFR_PRECOND(false, "Unknown image depth");
					return FP_NAN;
			}
		}


		
	} /* correl */
} /* jafar */

#endif
