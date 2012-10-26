
#include "kernel/timingTools.hpp"
#include "jmath/misc.hpp"

#include "correl/explorer.hpp"

namespace jafar {
namespace correl {


		/**
		FastTranslationMatcherZncc::doCorrelationFast
		@param im1 
		@param im1 
		@param x,y the correlation must be done with the center of im1 at position x,y in im2
		@param patch_in_im1
		@param hpatch_in_im1
		@param bestx,besty
		@param best_score
		@param ...
		*/
		void FastTranslationMatcherZncc::doCorrelationFast(image::Image const& im1, image::Image& im2, int x, int y,
			cv::Rect const &patch_in_im1, cv::Rect const &hpatch_in_im1, int& bestx, int& besty, double& best_score, 
			cv::Rect const &int_in_im2, double sum1, double sumSqr1, cv::Mat const &sum2_, cv::Mat const &sumSqr2_, 
			double minScore, int partialLine, double partialSum1, double partialSumSqr1, 
			cv::Rect const &result_in_im2, cv::Mat *results)
		{
			double &res = results->at<double>(y - result_in_im2.y, x - result_in_im2.x);
			if (res > -3.5) { JFR_ASSERT(bestx >= 0 && besty >= 0 && best_score > -3.5, ""); return; } // already computed
			cv::Rect patch_in_im2(x-hpatch_in_im1.width, y-hpatch_in_im1.height, patch_in_im1.width, patch_in_im1.height);
			cv::Rect patch_in_int2(patch_in_im2.x-int_in_im2.x, patch_in_im2.y-int_in_im2.y, patch_in_im1.width, patch_in_im1.height);
			im2.setROI(patch_in_im2);
			double sum2 = sum2_.at<double>(patch_in_int2.y                     ,patch_in_int2.x                    ) +
			              sum2_.at<double>(patch_in_int2.y+patch_in_int2.height,patch_in_int2.x+patch_in_int2.width) -
			              sum2_.at<double>(patch_in_int2.y+patch_in_int2.height,patch_in_int2.x                    ) -
			              sum2_.at<double>(patch_in_int2.y                     ,patch_in_int2.x+patch_in_int2.width);
			JFR_ASSERT(sum2 >= 0, "must be positive");
			double sumSqr2 = sumSqr2_.at<double>(patch_in_int2.y                     ,patch_in_int2.x                    ) +
			                 sumSqr2_.at<double>(patch_in_int2.y+patch_in_int2.height,patch_in_int2.x+patch_in_int2.width) -
			                 sumSqr2_.at<double>(patch_in_int2.y+patch_in_int2.height,patch_in_int2.x                    ) -
			                 sumSqr2_.at<double>(patch_in_int2.y                     ,patch_in_int2.x+patch_in_int2.width);
			JFR_ASSERT(sumSqr2 >= 0, "must be positive");
			double partialSum2 = 0, partialSumSqr2 = 0;
			if (partialLine >= 0)
			{
				partialSum2 = sum2_.at<double>(patch_in_int2.y+partialLine+1       ,patch_in_int2.x                    ) +
				              sum2_.at<double>(patch_in_int2.y+patch_in_int2.height,patch_in_int2.x+patch_in_int2.width) -
				              sum2_.at<double>(patch_in_int2.y+patch_in_int2.height,patch_in_int2.x                    ) -
				              sum2_.at<double>(patch_in_int2.y+partialLine+1       ,patch_in_int2.x+patch_in_int2.width);
				JFR_ASSERT(partialSum2 >= 0, "must be positive");
				partialSumSqr2 = sumSqr2_.at<double>(patch_in_int2.y+partialLine+1       ,patch_in_int2.x                    ) +
				                 sumSqr2_.at<double>(patch_in_int2.y+patch_in_int2.height,patch_in_int2.x+patch_in_int2.width) -
				                 sumSqr2_.at<double>(patch_in_int2.y+patch_in_int2.height,patch_in_int2.x                    ) -
				                 sumSqr2_.at<double>(patch_in_int2.y+partialLine+1       ,patch_in_int2.x+patch_in_int2.width);
				JFR_ASSERT(partialSumSqr2 >= 0, "must be positive");
			}
			double score = FastZncc::computeChar(im1, im2, sum1, sumSqr1, sum2, sumSqr2,
				minScore, partialLine, partialSum1, partialSumSqr1, partialSum2, partialSumSqr2);
#ifndef JFR_NDEBUG
			//double score2 = Zncc::compute(im1, im2, NULL);
			//JFR_ASSERT(std::abs(score-score2) <= 0.01 || std::abs(score+2) <= 0.01, "fast and robust don't agree " << score << " vs " << score2);
#endif
			res = score;
// JFR_DEBUG("doCorrelationFast " << x << "," << y << "," << score);
			if (score > best_score) { best_score = score; bestx = x; besty = y; }
		}
			
			
		/**
		FastTranslationMatcherZncc::rawExploreTranslationFast
		*/
		void FastTranslationMatcherZncc::rawExploreTranslationFast(image::Image const& im1, image::Image& im2, 
			image::ConvexRoi const &roi_in_im2, int &bestx, int &besty, double &best_score, 
			double minScore, int partialLine, cv::Rect const &result_in_im2, cv::Mat *results)
		{
//  JFR_DEBUG("rawExploreTranslationFast im1 " << im1.getROI() << " im2 " << roi_in_im2);
			// inits
			cv::Rect patch_in_im1(0, 0, im1.width(),im1.height());
			cv::Rect hpatch_in_im1(0, 0, patch_in_im1.width/2, patch_in_im1.height/2);
			int step = im1.step1() - patch_in_im1.width;
			
			// compute integrals im1
			unsigned int sum1_ = 0, sumSqr1_ = 0;
			unsigned int partialSum1_ = 0, partialSumSqr1_ = 0;
			const unsigned char *ptr = im1.data();
			for(int y = 0; y < patch_in_im1.height; ++y, ptr+=step) {
				for(int x = 0; x < patch_in_im1.width; ++x, ++ptr) {
					sum1_ += *ptr;
					sumSqr1_ += (*ptr)*(*ptr);
				}
				if (y == partialLine) { partialSum1_ = sum1_; partialSumSqr1_ = sumSqr1_; }
			}
			partialSum1_ = sum1_-partialSum1_;
			partialSumSqr1_ = sumSqr1_-partialSumSqr1_;
			double sum1=sum1_, sumSqr1=sumSqr1_;
			double partialSum1= partialSum1_, partialSumSqr1=partialSumSqr1_;
			// compute integrals im2
			cv::Mat sum2_, sumSqr2_;
			cv::Rect int_in_im2(roi_in_im2.x()-hpatch_in_im1.width, roi_in_im2.y()-hpatch_in_im1.height, 
			                    roi_in_im2.w()+2*hpatch_in_im1.width, roi_in_im2.h()+2*hpatch_in_im1.height);
			im2.setROI(int_in_im2);
			integral(im2, sum2_, sumSqr2_, CV_64FC1);

			// explore
			int y1 = roi_in_im2.y(), y2 = y1+roi_in_im2.h()-1;
			for(int y = y1; y <= y2; ++y)
			{
				int x1 = roi_in_im2.x(y), x2 = x1+roi_in_im2.w(y)-1;
				for(int x = x1; x <= x2; ++x)
				{
					doCorrelationFast(im1, im2, x, y, patch_in_im1, hpatch_in_im1, bestx, besty, best_score, 
						int_in_im2, sum1, sumSqr1, sum2_, sumSqr2_, minScore, partialLine, partialSum1, partialSumSqr1,
						result_in_im2, results);
					if (best_score > minScore) minScore = best_score;
				}
			}
		}

		/**
		FastTranslationMatcherZncc::doCorrelationRobust
		*/
		void FastTranslationMatcherZncc::doCorrelationRobust(image::Image const& im1, image::Image& im2, int x, int y,
			cv::Rect const &patch_in_im1, cv::Rect const &hpatch_in_im1, int& bestx, int& besty, double& best_score,
			cv::Rect const &result_in_im2, bool force, cv::Mat *results)
		{
			double &res = results->at<double>(y - result_in_im2.y, x - result_in_im2.x);
			if (res > (force?-1.5:-3.5)) { JFR_ASSERT(bestx >= 0 && besty >= 0 && best_score > -3.5, ""); return; } // already computed
			cv::Rect patch_in_im2(x-hpatch_in_im1.width, y-hpatch_in_im1.height, patch_in_im1.width, patch_in_im1.height);
			im2.setROI(patch_in_im2);
			double score = Zncc::compute8noborne(im1, im2);
			res = score;
// JFR_DEBUG("doCorrelationRobust " << x << "," << y << "," << score);
			if (score > best_score) { best_score = score; bestx = x; besty = y; }
		}


		/**
		FastTranslationMatcherZncc::rawExploreTranslationRobust
		@param im1
		@param im2
		@param iroi 
		@param croi
		@param bestx
		@param besty
		@param best_score
		@param results
		*/
		void FastTranslationMatcherZncc::rawExploreTranslationRobust(image::Image const& im1, image::Image& im2, 
			image::ConvexRoi const &roi_in_im2, int &bestx, int &besty, double &best_score, 
			cv::Rect const &result_in_im2, cv::Mat *results)
		{
//  JFR_DEBUG("rawExploreTranslationRobust im1 " << im1.getROI() << " im2 " << roi_in_im2);
			// inits
			cv::Rect patch_in_im1(0, 0, im1.width(),im1.height());
			cv::Rect hpatch_in_im1(0, 0, patch_in_im1.width/2, patch_in_im1.height/2);

			// explore
			int y1 = roi_in_im2.y(), y2 = y1+roi_in_im2.h()-1;
			for(int y = y1; y <= y2; ++y)
			{
				int x1 = roi_in_im2.x(y), x2 = x1+roi_in_im2.w(y)-1;
				for(int x = x1; x <= x2; ++x)
					doCorrelationRobust(im1, im2, x, y, patch_in_im1, hpatch_in_im1, bestx, besty, best_score, result_in_im2, false, results);
			}
		}

		/**
		FastTranslationMatcherZncc::match
		*/
		double FastTranslationMatcherZncc::match(image::Image const& im1, image::Image const& im2_, 
			image::ConvexRoi const &roi_in_im2, double &xres, double &yres, double &xstd, double &ystd, cv::Mat **results_)
		{
			double ambiguity_thres = 0.02;
			///-- init
// JFR_DEBUG("### FastTranslationMatcherZncc::match im1 " << im1.getROI() << " im2 " << roi_in_im2);
			// current roi
			cv::Rect im2_in_im2(0,0,im2_.width(),im2_.height());
			image::ConvexRoi roi1_in_im2 = roi_in_im2 * im2_in_im2;
			xres = roi1_in_im2.x() + roi1_in_im2.w()/2.;
			yres = roi1_in_im2.y() + roi1_in_im2.h()/2.;
			xstd = ystd = 0.;
			if (roi1_in_im2.w() == 0 || roi1_in_im2.h() == 0) return 0.;
			// global result tab
			cv::Rect patch_in_im1(0, 0, im1.width(),im1.height());
			cv::Rect hpatch_in_im1(0, 0, patch_in_im1.width/2, patch_in_im1.height/2);
			cv::Rect rect1_in_im2(roi1_in_im2.x(), roi1_in_im2.y(), roi1_in_im2.w(), roi1_in_im2.h());
			cv::Rect result_in_im2(rect1_in_im2.x-1, rect1_in_im2.y-1, rect1_in_im2.width+2, rect1_in_im2.height+2);
			// init results matrix
			cv::Mat *results;
			if (results_ == NULL) 
				results = new cv::Mat(result_in_im2.height, result_in_im2.width, CV_64FC1); else  // add 1 pixel border for interpolation
				results = *results_;
			*results = -4; // init all elements
			// global vars
			image::Image im2(im2_);
			int bestx = -1, besty = -1;
			xstd = ystd = -0.125;
			double best_score = -4.;
			
			///-- for dangerous areas, call the classical robust explorer and reduce the roi
			// this will be one line with GeneralRoi... (non convex)
// JFR_DEBUG("### fill dangerous areas");
			int out;
			if ((out = rect1_in_im2.x - hpatch_in_im1.width ) < 0) {
				image::ConvexRoi myroi(rect1_in_im2.x, rect1_in_im2.y, -out, rect1_in_im2.height ); myroi *= roi1_in_im2;
				rawExploreTranslationRobust(im1, im2, myroi, bestx, besty, best_score, result_in_im2, results); }
			if ((out = rect1_in_im2.y - hpatch_in_im1.height) < 0) {
				image::ConvexRoi myroi(rect1_in_im2.x, rect1_in_im2.y, rect1_in_im2.width, -out ); myroi *= roi1_in_im2;
				rawExploreTranslationRobust(im1, im2, myroi, bestx, besty, best_score, result_in_im2, results); }
			if ((out = im2_in_im2.width - (rect1_in_im2.x+rect1_in_im2.width+hpatch_in_im1.width)) < 0) {
				image::ConvexRoi myroi(rect1_in_im2.x+rect1_in_im2.width+out, rect1_in_im2.y, -out, rect1_in_im2.height); myroi *= roi1_in_im2;
				rawExploreTranslationRobust(im1, im2, myroi, bestx, besty, best_score, result_in_im2, results); }
			if ((out = im2_in_im2.height - (rect1_in_im2.y+rect1_in_im2.height+hpatch_in_im1.height)) < 0) {
				image::ConvexRoi myroi(rect1_in_im2.x, rect1_in_im2.y+rect1_in_im2.height+out, rect1_in_im2.width, -out); myroi *= roi1_in_im2;
				rawExploreTranslationRobust(im1, im2, myroi, bestx, besty, best_score, result_in_im2, results); }
			// TODO use robust too if some part of im2 in croi is masked (0)
			// now reduce the current roi
			cv::Rect remainRoi(hpatch_in_im1.width, hpatch_in_im1.height, im2_in_im2.width-2*hpatch_in_im1.width, im2_in_im2.height-2*hpatch_in_im1.height);
			image::ConvexRoi roi2_in_im2 = roi1_in_im2 * remainRoi;
			cv::Rect rect2_in_im2(roi2_in_im2.x(), roi2_in_im2.y(), roi2_in_im2.w(), roi2_in_im2.h());

			///-- decide which optimizations should be done
// JFR_DEBUG("### compute optimizations (" << bestx << "," << besty << "," << best_score << ")");
			bool doFastSearch = false, doHalfResSearch = false; // we do halfResSearch only if we already do fastSearch
			{
				// based on im1
				bool doFastSearch1 = true, doHalfResSearch1 = true;
				{
					doFastSearch1    = (patch_in_im1.width >= 3 && patch_in_im1.height >= 3);
					doHalfResSearch1 = (patch_in_im1.width >= 5 && patch_in_im1.height >= 5);
				}
				// based on im2
				bool doFastSearch2 = true, doHalfResSearch2 = true;
				if (doFastSearch1 || doHalfResSearch1)
				{
					int npix = roi2_in_im2.count();
					double ratio = (npix == 0 ? 0 : npix / (double)roi2_in_im2.size());
					if (npix >= 5000) { doFastSearch2 = true          ; doHalfResSearch2 = true;           } else
					if (npix >= 1000) { doFastSearch2 = true          ; doHalfResSearch2 = (ratio >= 0.1); } else
					if (npix >=  500) { doFastSearch2 = (ratio >= 0.1); doHalfResSearch2 = (ratio >= 0.5); } else
					if (npix >=    9) { doFastSearch2 = (ratio >= 0.5); doHalfResSearch2 = false;          } else
														{ doFastSearch2 = false         ; doHalfResSearch2 = false;          }
				}
				// fusion
				doFastSearch = doFastSearch1 && doFastSearch2;
				doHalfResSearch = doHalfResSearch1 && doHalfResSearch2;
			}

			///-- process
			if (doHalfResSearch)
			{
// JFR_DEBUG("### do full half search (" << bestx << "," << besty << "," << best_score << ")");
				// scale patch: scale to approx 0.5 ensuring that the resulting patch size is odd
				cv::Rect nhpatch_in_im1(0, 0, patch_in_im1.width/4, patch_in_im1.height/4);
				cv::Rect npatch_in_im1(nhpatch_in_im1.width, nhpatch_in_im1.height, nhpatch_in_im1.width*2+1, nhpatch_in_im1.height*2+1);
				int partialLine = npatch_in_im1.height*partialPosition;
				double fw = npatch_in_im1.width/(double)patch_in_im1.width;
				double fh = npatch_in_im1.height/(double)patch_in_im1.height;
				image::Image imA(npatch_in_im1.width, npatch_in_im1.height, CV_8U, JfrImage_CS_GRAY);
				im1.resize(imA, CV_INTER_LINEAR);
				
				// scale image search area: we just want the scale of im2 to be the closest possible to the scale of im1
				cv::Rect int_in_im2(rect2_in_im2.x-hpatch_in_im1.width, rect2_in_im2.y-hpatch_in_im1.height, 
				                    rect2_in_im2.width+2*hpatch_in_im1.width, rect2_in_im2.height+2*hpatch_in_im1.height);
				cv::Rect nint_in_im2(jmath::round(int_in_im2.x*fw), jmath::round(int_in_im2.y*fh),
				                     jmath::round(int_in_im2.width*fw), jmath::round(int_in_im2.height*fh));
				cv::Rect nrect2_in_im2(nint_in_im2.x+nhpatch_in_im1.width, nint_in_im2.y+nhpatch_in_im1.height,
				                       nint_in_im2.width-2*nhpatch_in_im1.width, nint_in_im2.height-2*nhpatch_in_im1.height);
				image::ConvexRoi nroi2_in_im2 = roi2_in_im2;
				nroi2_in_im2.scaleTo(nrect2_in_im2);
				
				int nbestx = -1, nbesty = -1;
				double nbest_score = -4;
				
				im2.setROI(int_in_im2);
				image::Image imB(nint_in_im2.width, nint_in_im2.height, CV_8U, JfrImage_CS_GRAY);
				im2.resize(imB, CV_INTER_LINEAR);
				image::ConvexRoi nroi2_in_imB = nroi2_in_im2 - cv::Point(nint_in_im2.x, nint_in_im2.y);
				cv::Rect nrect2_in_imB(nroi2_in_imB.x(), nroi2_in_imB.y(), nroi2_in_imB.w(), nroi2_in_imB.h());
				
				//cv::Rect nresult_in_im2(nrect2_in_im2.x-1, nrect2_in_im2.y-1, nrect2_in_im2.width+2, nrect2_in_im2.height+2);
				cv::Rect nresult_in_imB(nrect2_in_imB.x-1, nrect2_in_imB.y-1, nrect2_in_imB.width+2, nrect2_in_imB.height+2);
				cv::Mat *nresults = new cv::Mat(nresult_in_imB.height, nresult_in_imB.width, CV_64FC1);
				*nresults = -4;
				double currentMinScore = minScore;
// kernel::Chrono chrono;
				rawExploreTranslationFast(imA, imB, nroi2_in_imB, nbestx, nbesty, nbest_score, currentMinScore, partialLine, nresult_in_imB, nresults);
// std::cout << "half search took " << chrono.elapsed() << std::endl;
				
// JFR_DEBUG("### do refine search (" << nbestx << "," << nbesty << "," << nbest_score << ")");
				if (nbestx != -1 && nbesty != -1)
				{
					const int refineDist = 1;
					image::ConvexRoi rroi2_in_im2(jmath::round((nbestx-nrect2_in_imB.x)/fw) + rect2_in_im2.x - refineDist,
																				jmath::round((nbesty-nrect2_in_imB.y)/fh) + rect2_in_im2.y - refineDist,
																				2*refineDist+1, 2*refineDist+1); // refine roi
					rroi2_in_im2 *= result_in_im2;
					rawExploreTranslationRobust(im1, im2, rroi2_in_im2, bestx, besty, best_score, result_in_im2, results);
				}
				
				im2.resetROI();
				delete nresults;
			} else
			if (doFastSearch)
			{
// JFR_DEBUG("### do full fast search (" << bestx << "," << besty << "," << best_score << ")");
				int partialLine = patch_in_im1.height*partialPosition;
				double currentMinScore = std::max(minScore, best_score);
				rawExploreTranslationFast(im1, im2, roi2_in_im2, bestx, besty, best_score, currentMinScore, partialLine, result_in_im2, results);
			} else
			{
// JFR_DEBUG("### do full robust search (" << bestx << "," << besty << "," << best_score << ")");
				rawExploreTranslationRobust(im1, im2, roi2_in_im2, bestx, besty, best_score, result_in_im2, results);
			}

			if (bestx == -1 || besty == -1) // the score was nowhere higher than the min score
			{
				if (results_ == NULL) delete results;
				return 0.;
			}

			///-- ensure that all values that will be used by interpolation are computed
			// it should never happen that newnewbestx != newbestx or newnewbesty != newbesty, but in some particular
			// cases half res search may be wrong
// JFR_DEBUG("### ensure interpol available (" << bestx << "," << besty << "," << best_score << ")");
			int nbestx = bestx, nbesty = besty;
			do {
				bestx = nbestx, besty = nbesty;
				if (bestx > result_in_im2.x                        && results->at<double>((besty  )-result_in_im2.y,(bestx-1)-result_in_im2.x) < -1.5)
					doCorrelationRobust(im1, im2, bestx-1, besty  , patch_in_im1, hpatch_in_im1, nbestx, nbesty, best_score, result_in_im2, true, results);
				if (bestx < result_in_im2.x+result_in_im2.width-1  && results->at<double>((besty  )-result_in_im2.y,(bestx+1)-result_in_im2.x) < -1.5)
					doCorrelationRobust(im1, im2, bestx+1, besty  , patch_in_im1, hpatch_in_im1, nbestx, nbesty, best_score, result_in_im2, true, results);
				if (besty > result_in_im2.y                        && results->at<double>((besty-1)-result_in_im2.y,(bestx  )-result_in_im2.x) < -1.5)
					doCorrelationRobust(im1, im2, bestx  , besty-1, patch_in_im1, hpatch_in_im1, nbestx, nbesty, best_score, result_in_im2, true, results);
				if (besty < result_in_im2.y+result_in_im2.height-1 && results->at<double>((besty+1)-result_in_im2.y,(bestx  )-result_in_im2.x) < -1.5)
					doCorrelationRobust(im1, im2, bestx  , besty+1, patch_in_im1, hpatch_in_im1, nbestx, nbesty, best_score, result_in_im2, true, results);
			} while (bestx != nbestx || besty != nbesty);
			
// JFR_DEBUG("### interpolate (" << bestx << "," << besty << "," << best_score << ")");
			bool ambiguous;

			///-- interpolate
			double a1, a2, a3, best_score_x, best_score_y;
			// interpolate x
			best_score_x = best_score; xres = 0; a1 = a2 = a3 = -2;
			if (bestx > result_in_im2.x)
				a1 = results->at<double>((besty  )-result_in_im2.y,(bestx-1)-result_in_im2.x);
			a2 = results->at<double>((besty  )-result_in_im2.y,(bestx  )-result_in_im2.x);
			if (bestx < result_in_im2.x+result_in_im2.width-1)
				a3 = results->at<double>((besty  )-result_in_im2.y,(bestx+1)-result_in_im2.x);
			ambiguous = false;
			if (a1 > -1.5 && a3 > -1.5)
			{
				jmath::parabolicInterpolation(a1,a2,a3, xres, best_score_x, xstd);
				if (a1 > a2-ambiguity_thres && a3 > a2-ambiguity_thres) ambiguous = true;
			} else ambiguous = true;
			xres += bestx+0.5;
			
			if (ambiguous)
				xstd = 1e6;
			else 
				xstd = -1.0/(2*xstd);
			
			// interpolate y
			best_score_y = best_score; yres = 0; a1 = a2 = a3 = -2;
			if (besty > result_in_im2.y)
				a1 = results->at<double>((besty-1)-result_in_im2.y,(bestx  )-result_in_im2.x);
			a2 = results->at<double>((besty  )-result_in_im2.y,(bestx  )-result_in_im2.x);
			if (besty < result_in_im2.y+result_in_im2.height-1)
				a3 = results->at<double>((besty+1)-result_in_im2.y,(bestx  )-result_in_im2.x);
			ambiguous = false;
			if (a1 > -1.5 && a3 > -1.5)
			{
				jmath::parabolicInterpolation(a1,a2,a3, yres, best_score_y, ystd);
				if (a1 > a2-ambiguity_thres && a3 > a2-ambiguity_thres) ambiguous = true;
			} else ambiguous = true;
			yres += besty+0.5;
			if (ambiguous)
				ystd = 1e6;
			else
				ystd = -1.0/(2*ystd);
				

			// merge score interpolations
			// we could do better, with 3+3+2 interpolations, but it's too much work
			//best_score = std::max(best_score_x, best_score_y); // approx #1, minoring
			best_score = std::min(1.0,best_score_x+best_score_y-best_score); // approx #2, should be closer, but not guaranted to be minoring
			
// JFR_DEBUG("### end (" << xres << "," << yres << "," << best_score << ")");
			if (results_ == NULL) delete results;
			return best_score;
		}


}}


