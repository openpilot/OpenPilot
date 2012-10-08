
#include "correl/zncc.hpp"

#include <stdint.h>

namespace jafar {
namespace correl {

	// a bit faster than normal implementation, but a bit less precise (but it should be ok even for a full image)
	// all ifs on boolean template parameters cost nothing
	template<int depth, typename worktype, typename bornetype, bornetype borneinf, bornetype bornesup, bool useBornes, bool useWeightMatrix>
	double Zncc::computeTpl(image::Image const& im1_, image::Image const& im2_, float const* weightMatrix)
	{
		// preconds
		JFR_PRECOND( im1_.depth() == depth, "Image 1 depth is different from the template parameter" );
		JFR_PRECOND( im2_.depth() == depth, "Image 2 depth is different from the template parameter" );
		JFR_PRECOND( im1_.channels() == im2_.channels(), "The channels number of both images are different" );
		JFR_PRECOND( !useWeightMatrix || weightMatrix, "Template parameter tells to use weightMatrix but no one is given" );
		
		// adjust ROIs to match size, assuming that it is reduced when set out of the image
		// FIXME weightMatrix should be a cv::Mat in order to have a ROI too, and to adjust it
		cv::Size size1; cv::Rect roi1 = im1_.getROI(size1);
		cv::Size size2; cv::Rect roi2 = im2_.getROI(size2);
		int dw = roi1.width - roi2.width, dh = roi1.height - roi2.height;
		if (dw != 0)
		{
			cv::Rect &roiA = (dw<0 ? roi1 : roi2), &roiB = (dw<0 ? roi2 : roi1);
			cv::Size &sizeA = (dw<0 ? size1 : size2);
			if (roiA.x == 0) { roiB.x += dw; roiB.width -= dw; } else
			if (roiA.x+roiA.width == sizeA.width) { roiB.width -= dw; }
		}
		if (dh != 0)
		{
			cv::Rect &roiA = (dh<0 ? roi1 : roi2), &roiB = (dh<0 ? roi2 : roi1);
			cv::Size &sizeA = (dh<0 ? size1 : size2);
			if (roiA.y == 0) { roiB.y += dh; roiB.height -= dh; } else
			if (roiA.y+roiA.height == sizeA.height) { roiB.height -= dh; }
		}
		image::Image im1(im1_); im1.setROI(roi1);
		image::Image im2(im2_); im2.setROI(roi2);

		// some variables initialization
		int height = im1.height();
		int width = im1.width();
		int step1 = im1.step1() - width;
		int step2 = im2.step1() - width;
		
		double mean1 = 0., mean2 = 0.;
		double sigma1 = 0., sigma2 = 0., sigma12 = 0.;
		double zncc_sum = 0.;
		double zncc_count = 0.;
		double zncc_total = 0.;
		
		worktype const* im1ptr = reinterpret_cast<worktype const*>(im1.data());
		worktype const* im2ptr = reinterpret_cast<worktype const*>(im2.data());
		
		float const* wptr = weightMatrix;
		double w;
		
		// start the loops
		for(int i = 0; i < height; ++i) 
		{
			for(int j = 0; j < width; ++j) 
			{
				worktype im1v = *(im1ptr++);
				worktype im2v = *(im2ptr++);
				if (useWeightMatrix) w = *(wptr++); else w = 1;
				if (useBornes) zncc_total += w;
				
//std::cout << "will correl ? " << useBornes << ", " << (int)im1v << ", " << (int)im2v << std::endl;
				if (!useBornes || (im1v != borneinf && im1v != bornesup && im2v != borneinf && im2v != bornesup))
				{
//std::cout << "correl one pixel" << std::endl;
#if 0
					double im1vw, im2vw;
					if (useWeightMatrix)
						{ im1vw = im1v * w; im2vw = im2v * w; } else
						{ im1vw = im1v;     im2vw = im2v;     }
					zncc_count += w;
					mean1 += im1vw;
					mean2 += im2vw;
					sigma1 += im1v * im1vw;
					sigma2 += im2v * im2vw;
					zncc_sum += im1v * im2vw;
#else
					zncc_count += w;
					mean1 += im1v * w;
					mean2 += im2v * w;
					sigma1 += im1v * im1v * w;
					sigma2 += im2v * im2v * w;
					zncc_sum += im1v * im2v * w;
#endif
				}
			}
			im1ptr += step1;
			im2ptr += step2;
		}
		
		if (useBornes) if (zncc_count / zncc_total < 0.75)
			{ /*std::cout << "zncc failed: " << zncc_count << "," << zncc_total << std::endl;*/ return -3; }
		
		// finish
		mean1 /= zncc_count;
		mean2 /= zncc_count;
		sigma1 = sqrt(sigma1/zncc_count - mean1*mean1);
		sigma2 = sqrt(sigma2/zncc_count - mean2*mean2);
		sigma12 = sigma1*sigma2;
// std::cout << "normal: zncc_sum " << zncc_sum << ", count " << zncc_count << ", mean12 " << mean1*mean2 << ", sigma12 " << sigma1*sigma2 << std::endl;
		zncc_sum = (sigma12 < 1e-6 ? -1 : (zncc_sum/zncc_count - mean1*mean2) / sigma12);
		
		JFR_ASSERT(zncc_sum >= -1.01, "");
		return zncc_sum;
	}


	double Zncc::compute8noborne(image::Image const& im1, image::Image const& im2)
	{
		JFR_PRECOND(im1.depth() == im2.depth(), "The depth of both images is different");
		JFR_PRECOND(im1.depth() == CV_8U, "The depth of images must be CV_8U");
		return computeTpl<CV_8U, uint8_t,uint8_t,0,255,false,false>(im1,im2);
	}

	double Zncc::compute(image::Image const& im1, image::Image const& im2, float const* weightMatrix)
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
				if (weightMatrix == NULL)
					return computeTpl<CV_8U, uint8_t,uint8_t,0,255,true,false>(im1,im2);
				else
					return computeTpl<CV_8U, uint8_t,uint8_t,0,255,true,true>(im1,im2,weightMatrix);
			case CV_8S:
				if (weightMatrix == NULL)
					return computeTpl<CV_8S, int8_t,int8_t, -128,127,true,false>(im1,im2);
				else
					return computeTpl<CV_8S, int8_t,int8_t, -128,127,true,true>(im1,im2,weightMatrix);
			case CV_16U:
				if (weightMatrix == NULL)
					return computeTpl<CV_16U, uint16_t,uint16_t, 0,65535,true,false>(im1,im2);
				else
					return computeTpl<CV_16U, uint16_t,uint16_t, 0,65535,true,true>(im1,im2,weightMatrix);
			case CV_16S:
				if (weightMatrix == NULL)
					return computeTpl<CV_16S, int16_t,int16_t, -32768,32767,true,false>(im1,im2);
				else
					return computeTpl<CV_16S, int16_t,int16_t, -32768,32767,true,true>(im1,im2,weightMatrix);
			case CV_32F:
				if (weightMatrix == NULL) // bool and no borne because cannot use a float as a template parameter, and anyway would be useless here
					return computeTpl<CV_32F, float,bool, 0,0,false,false>(im1,im2);
				else
					return computeTpl<CV_32F, float,bool, 0,0,false,true>(im1,im2,weightMatrix);
			case CV_64F:
				if (weightMatrix == NULL) // bool and no borne because cannot use a float as a template parameter, and anyway would be useless here
					return computeTpl<CV_64F, double,bool, 0,0,false,false>(im1,im2);
				else
					return computeTpl<CV_64F, double,bool, 0,0,false,true>(im1,im2,weightMatrix);
			default:
				JFR_PRECOND(false, "Unknown image depth");
				return FP_NAN;
		}
	}

	#if 1
	// FIXME : test / improve it to really manage rois
	double Zncc::exploreRotation(image::Image const* im1, image::Image const* im2, int rotationStep)
	{
		cv::Rect roi2 = im2->getROI();
//		int dim = (roi2->width > roi2->height ? roi2->width : roi2->height)*1.5;// 1.5 > sqrt(2)
//		image::Image* im2bis = new image::Image(dim, dim, im2->depth(), im2->colorSpace());
		image::Image* im2bis = new image::Image(roi2.width, roi2.height, im2->depth(), im2->colorSpace());
		double bestZncc = compute(*im1,*im2);
	//   double tempBestAngle = 0.;
		for(int angle = rotationStep; angle < 360; angle += rotationStep)
		{
			float m[6];
			int w = im2bis->width();
			int h = im2bis->height();
			float radangle = angle * CV_PI/180.;
			m[0] = (float)(cos(radangle));
			m[1] = (float)(sin(radangle));
			m[2] = w * 0.5f;
			m[3] = -m[1];
			m[4] = m[0];
			m[5] = h * 0.5f;
			CvMat M1 = cvMat( 2, 3, CV_32F, m );
			cv::Mat M(&M1);
			im2bis->setTo(0);
			image::getQuadrangleSubPix( *im2, *im2bis, M);
			double zncc = compute(*im1,*im2bis);
			if(zncc > bestZncc) { bestZncc = zncc; /*tempBestAngle = radangle;*/}
		}
	//   JFR_DEBUG(tempBestAngle);
		delete im2bis;
		return bestZncc;
	}
	#endif


	#if 0
	// the normal algorithm in two steps, for reference, maybe we should provide it

	{
			// Compute the mean
			worktype const* im1ptr = reinterpret_cast<worktype const*>(im1->data());
			worktype const* im2ptr = reinterpret_cast<worktype const*>(im2->data());
			im1ptr += roi1.y*im1->step()+roi1.x;
			im2ptr += roi2.y*im2->step()+roi2.x;
			
			double mean1 = 0.;
			double mean2 = 0.;
			int zncc_count = 0;
			
			for(int i = 0; i < roi1.height; ++i) 
			{
				for(int j = 0; j < roi1.width; ++j) 
				{
					worktype im1v = *(im1ptr++);
					worktype im2v = *(im2ptr++);
					if(im1v != borneinf && im1v != bornesup && im2v != borneinf && im2v != bornesup )
					{
						mean1 += im1v;
						mean2 += im2v;
						zncc_count++;
					}
				}
				im1ptr += roi1_step;
				im2ptr += roi2_step;
			}
			mean1 /= zncc_count;
			mean2 /= zncc_count; 
		
			// Compute the sigma and the zncc sum
			im1ptr = reinterpret_cast<worktype const*>(im1->data());
			im2ptr = reinterpret_cast<worktype const*>(im2->data());
			im1ptr += roi1.y*im1->step()+roi1.x;
			im2ptr += roi2.y*im2->step()+roi2.x;
			
			double sigma1 = 0.;
			double sigma2 = 0.;
			double zncc_sum = 0.;
			
			for(int i = 0; i < roi1.height; ++i) 
			{
				for(int j = 0; j < roi1.width; ++j) 
				{
					worktype im1v = *(im1ptr++);
					worktype im2v = *(im2ptr++);
					if(im1v != borneinf && im1v != bornesup && im2v != borneinf && im2v != bornesup )
					{
						sigma1 += details::pow2( im1v - mean1 );
						sigma2 += details::pow2( im2v - mean2 );
						zncc_sum += im1v * im2v;
					}
				}
				im1ptr += roi1_step;
				im2ptr += roi2_step;
			}
			sigma1 = sqrt(sigma1 / zncc_count);
			sigma2 = sqrt(sigma2 / zncc_count);
			zncc_sum = zncc_sum / zncc_count - mean1 * mean2;
			
			return zncc_sum / (sigma1 * sigma2);
	}
	#endif



}}
