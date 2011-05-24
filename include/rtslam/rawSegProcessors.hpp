/**
 * \file rawProcessors.hpp
 *
 *  Some wrappers to raw processors to be used generically
 *
 * \date 20/07/2010
 * \author croussil@laas.fr
 *
 * ## Add detailed description here ##
 *
 * \ingroup rtslam
 */

#ifndef RAWSEGPROCESSORS_HPP
#define RAWSEGPROCESSORS_HPP

#ifdef HAVE_MODULE_DSEG

#include "dseg/DirectSegmentsTracker.hpp"
#include "dseg/SegmentHypothesis.hpp"
#include "dseg/SegmentsSet.hpp"
#include "dseg/ConstantVelocityPredictor.hpp"
#include "dseg/RtslamPredictor.hpp"
#include "dseg/HierarchicalDirectSegmentsDetector.hpp"
#include "dseg/GradientStatsDescriptor.hpp"

#include "rtslam/rawImage.hpp"
#include "rtslam/sensorPinhole.hpp"
#include "rtslam/descriptorImageSeg.hpp"

namespace jafar {
namespace rtslam {

	boost::weak_ptr<RawImage> lastDsegImage;
	dseg::PreprocessedImage preprocDsegImage;

	boost::weak_ptr<RawImage> lastDsegImageForDetection;
	dseg::PyramidSP* preprocDsegPyramid;

	float meanOnLine(const image::Image& img, int x1, int y1, int x2, int y2)
	{
		int x0 = x1;
		int y0 = y1;
		int dx = abs(x2 - x1);
		int dy = abs(y2 - y1);
		int sx = (x1 < x2) ? 1 : -1;
		int sy = (y1 < y2) ? 1 : -1;
		float err = dx - dy;
		float _2err;

		int sum = 0;
		int nbpoints = 0;

		const uchar* pix = img.data();

		if(x0 >= 0 && x0 < (int)img.width() &&
			y0 >= 0 && y0 < (int)img.height())
		{
			sum += *(pix + x0 * img.width() + y0); // TODO : check if it works
			nbpoints++;
		}
		while(x0 != x2 || y0 != y2)
		{
			_2err = 2*err;

			if(_2err > -dy)
			{
				err -= dy;
				x0 += sx;
			}
			if(_2err < dx)
			{
				err  += dx;
				y0 += sy;
			}

			if(x0 >= 0 && x0 < (int)img.width() &&
				y0 >= 0 && y0 < (int)img.height())
			{
				sum += *(pix + x0 * img.width() + y0); // TODO : check if it works
				nbpoints++;
			}
		}

		return float(sum)/nbpoints;
	}

	// compute the mean of all pixels inside the polygon v1 v2 v3 v4, the order is important, the polygon must be convex
	float meanOnPoly(const boost::shared_ptr<Image> & image, const jblas::vec2& v1, const jblas::vec2& v2, const jblas::vec2& v3, const jblas::vec2& v4)
	{
		float sum = 0.0;
		int nbmeasure = 0;

		const jblas::vec2* vertices[4] = {&v1,&v2,&v3,&v4};
		int uppermost = 0;

		int left_edges[] = {-1, -1, -1, -1,};
		int right_edges[] = {-1, -1, -1, -1,};

		// Get uppermost vertex
		for(int i = 1 ; i < 4 ; i++) {
			if((*vertices[i])[1] > (*vertices[uppermost])[1])
				uppermost = i;
		}

		// Compute left and right
		left_edges[0] = uppermost;
		right_edges[0] = uppermost;
		int left = (uppermost+3)%4;
		int right = (uppermost+1)%4;
		if((*vertices[left])[0] < (*vertices[right])[0]) {
			left_edges[1] = left;
			right_edges[1] = right;
		} else {
			left_edges[1] = right;
			right_edges[1] = left;
		}

		left = 1;
		right = 1;
		// Compute the rest of the left and right edges
		while(left < 3) // Left
		{
			// current edge is between left_edges[left -1] and left_edges[left]
			// since we draw a quad the next vertex adjacent to left_edges[left] is
			// such that (left_edges[left -1] - next)%2 != 0
			int next = ((left_edges[left -1] + 2)% 4);

			// if the next vertex is higher than the previous one we reached the bottom
			if((*vertices[next])[1] > (*vertices[left_edges[left]])[1])
				break;

			left++;
			left_edges[left] = next;
		}
		while(right < 3) // Right
		{
			int next = ((right_edges[right -1] + 2)% 4);

			if((*vertices[next])[1] > (*vertices[right_edges[right]])[1])
				break;

			right++;
			right_edges[right] = next;
		}

		left = 0;
		right = 0;
		// Draw the scanlines
		int line = (*vertices[uppermost])[1];
		bool done = false;
		while(!done)
		{
			// Update current edge
			// Left
			while(line < (*vertices[left_edges[left+1]])[1] && !done)
			{
				if(left < 2 && left_edges[left+2] >= 0)
					left++;
				else
					done = true;
			}
			// Right
			while(line < (*vertices[right_edges[right+1]])[1] && !done)
			{
				if(right < 2 && right_edges[right+2] >= 0)
					right++;
				else
					done = true;
			}

			if(!done)
			{
				// Compute scanline extremities
				int tl = left_edges[left];
				int bl = left_edges[left+1];
				int tr = right_edges[right];
				int br = right_edges[right+1];

				float left_alpha = (float)(line - (*vertices[bl])[1]) / (float)((*vertices[tl])[1] - (*vertices[bl])[1]);
				float right_alpha = (float)(line - (*vertices[br])[1]) / (float)((*vertices[tr])[1] - (*vertices[br])[1]);

				float col_beg = left_alpha * (*vertices[tl])[0] +  (1 - left_alpha) * (*vertices[bl])[0];
				float col_end = right_alpha * (*vertices[tr])[0] + (1 - right_alpha) * (*vertices[br])[0];

				if(col_beg - int(col_beg) > 0.5)
					col_beg = int(col_beg) + 1;
				else
					col_beg = int(col_beg);

				if(col_end - int(col_end) > 0.5)
					col_end = int(col_end) + 1;
				else
					col_end = int(col_end);

				// Measure on line
				for(int x=col_beg ; x<=col_end; x++)
				{
					if(x<0 || x>=image->width() || line<0 || line>=image->height())
						continue;

					sum += image->data()[line*image->width() + x];
					// image->data()[line*image->width() + x] = 255 - image->data()[y*image->width() + x]; // DEBUG : invert color in order to visualise the area
					nbmeasure++;
				}

				// Update line
				line--;
			}
		}

		return (nbmeasure > 0) ? sum / nbmeasure : 0;
	}

	class DsegMatcher
   {
      private:
         dseg::DirectSegmentsTracker matcher;
         dseg::RtslamPredictor predictor;

      public:
         struct matcher_params_t {
            // RANSAC
            int maxSearchSize;
            double lowInnov;      ///<     search region radius for first RANSAC consensus
            double threshold;     ///<     matching threshold
            double mahalanobisTh; ///< Mahalanobis distance for outlier rejection
            double relevanceTh; ///< Mahalanobis distance for no information rejection
            double measStd;       ///<       measurement noise std deviation
            double measVar;       ///<       measurement noise variance
         } params;

      private :
			void projectExtremities(const vec4& meas, const vec4& exp, vec4& newMeas, float* stdRatio) const
			{
            // extract predicted points
            vec2 P1 = subrange(exp,0,2);
				vec2 P2 = subrange(exp,2,4);
				double P12_2 = (P2(0) - P1(0))*(P2(0) - P1(0)) // Square(distance(P1,P2))
							  +   (P2(1) - P1(1))*(P2(1) - P1(1));
				double P12 = sqrt(P12_2);
            // extract measured line
            vec2 L1 = subrange(meas,0,2);
            vec2 L2 = subrange(meas,2,4);
				double L12_2 = (L2(0) - L1(0))*(L2(0) - L1(0)) // Square(distance(L1,L2))
							+   (L2(1) - L1(1))*(L2(1) - L1(1));
				double L12 = sqrt(L12_2);

				// compute predicted center
				vec2 Pc = (P1 + P2) / 2;
				// project on measured line
				double u = (((Pc(0) - L1(0))*(L2(0) - L1(0)))
							  +((Pc(1) - L1(1))*(L2(1) - L1(1))))
							  /(L12_2);
				vec2 Lc = L1 + u*(L2 - L1);

				// compute measured orientation
				double angle = atan2(L2(1) - L1(1), L2(0) - L1(0));

				// compute extremities
				newMeas[0] = Lc[0] - P12 * cos(angle) / 2;
				newMeas[1] = Lc[1] - P12 * sin(angle) / 2;
				newMeas[2] = Lc[0] + P12 * cos(angle) / 2;
				newMeas[3] = Lc[1] + P12 * sin(angle) / 2;
/*
            // TODO : be carefull L1 != L2
            // project predicted points on line
            double u = (((P1(0) - L1(0))+(L2(0) - L1(0)))
                       +((P1(1) - L1(1))+(L2(1) - L1(1))))
                       /(norm_1(L1 - L2) * norm_1(L1 - L2));
            subrange(newMeas,0,2) = L1 + u*(L2 - L1);

				u = (((P2(0) - L2(0))+(L1(0) - L2(0)))
					 +((P2(1) - L2(1))+(L1(1) - L2(1))))
                /(norm_1(L1 - L2) * norm_1(L1 - L2));
				subrange(newMeas,2,4) = L2 + u*(L1 - L2);
*/

				*stdRatio = P12 / L12;
         }

      public:
         DsegMatcher(double lowInnov, double threshold, double mahalanobisTh, double relevanceTh, double measStd):
            matcher(), predictor()
         {
            params.lowInnov = lowInnov;
            params.threshold = threshold;
            params.mahalanobisTh = mahalanobisTh;
            params.relevanceTh = relevanceTh;
            params.measStd = measStd;
         }

         void match(const boost::shared_ptr<RawImage> & rawPtr, const appearance_ptr_t & targetApp, const image::ConvexRoi & roi, Measurement & measure, appearance_ptr_t & app)
			{
				if(rawPtr != lastDsegImage.lock())
				{
					matcher.preprocessImage(*(rawPtr->img),preprocDsegImage);
					lastDsegImage = rawPtr;
				}

				app_img_seg_ptr_t targetAppSpec = SPTR_CAST<AppearanceImageSegment>(targetApp);
				app_img_seg_ptr_t appSpec = SPTR_CAST<AppearanceImageSegment>(app);

            dseg::SegmentsSet setin, setout;

            setin.addSegment(targetAppSpec->hypothesis());
				matcher.trackSegment(preprocDsegImage,setin,&predictor,setout);

            if(setout.count() > 0) {
					vec4 pred;
					vec4 obs;
					vec4 projected;
					float ratio;

					pred(0) = targetAppSpec->hypothesis()->x1();
					pred(1) = targetAppSpec->hypothesis()->y1();
					pred(2) = targetAppSpec->hypothesis()->x2();
					pred(3) = targetAppSpec->hypothesis()->y2();

					obs(0) = setout.segmentAt(0)->x1();
					obs(1) = setout.segmentAt(0)->y1();
					obs(2) = setout.segmentAt(0)->x2();
					obs(3) = setout.segmentAt(0)->y2();

					vec2 normal;
					normal(0) = obs(1) - obs(3);
					normal(1) = obs(2) - obs(0);
					vec2 top,bottom;
					top[0]    = obs[0]; top[1]    = obs[1];
					bottom[0] = obs[2]; bottom[1] = obs[3];

					jmath::ublasExtra::normalize(normal);
					normal *= 5; // measure area width

					appSpec->patchMeanLeft = meanOnPoly(rawPtr->img, top, top + normal, bottom + normal, bottom);
					appSpec->patchMeanRight = meanOnPoly(rawPtr->img, top, top - normal, bottom - normal, bottom);

					projectExtremities(obs,pred, projected, &ratio);
					measure.x() = projected;
					measure.std(params.measStd * ratio);

					// Compute matchScore according to predicted and observed means
					float obsLeft = appSpec->patchMeanLeft;
					float obsRight = appSpec->patchMeanRight;
					float predLeft = targetAppSpec->patchMeanLeft;
					float predRight = targetAppSpec->patchMeanRight;

					float correlLeft = min(obsLeft,predLeft) / max(obsLeft,predLeft);
					float correlRight = min(obsRight,predRight) / max(obsRight,predRight);
					//measure.matchScore = 1;
					measure.matchScore = max(correlLeft, correlRight);

               appSpec->setHypothesis(setout.segmentAt(0));
				}
            else {
               measure.matchScore = 0;
            }
         }
   };

   class HDsegDetector
   {
      private:
			dseg::HierarchicalDirectSegmentsDetector detector;
         boost::shared_ptr<DescriptorFactoryAbstract> descFactory;

      public:
         struct detector_params_t {
				int patchSize;  ///<       descriptor patch size
				// RANSAC
            double measStd;       ///<       measurement noise std deviation
            double measVar;       ///<       measurement noise variance
            // HDSEG
            int hierarchyLevel;
         } params;

      public:
			HDsegDetector(int patchSize, int hierarchyLevel, double measStd,
            boost::shared_ptr<DescriptorFactoryAbstract> const &descFactory):
            detector(), descFactory(descFactory)
         {
				params.patchSize = patchSize;
				params.hierarchyLevel = hierarchyLevel;
            params.measStd = measStd;
            params.measVar = measStd * measStd;
         }

			bool detect(const boost::shared_ptr<RawImage> & rawData, const image::ConvexRoi &roi, boost::shared_ptr<FeatureImageSegment> & featPtr)
         {
            bool ret = false;
				featPtr.reset(new FeatureImageSegment());
            featPtr->measurement.std(params.measStd);

				if(rawData != lastDsegImageForDetection.lock())
				{
					preprocDsegPyramid = detector.computePyramid(*(rawData->img));
					lastDsegImageForDetection = rawData;
				}

				dseg::SegmentsSet set;
				detector.detectSegment(preprocDsegPyramid, *(rawData->img.get()), &roi, set);

				if(set.count() > 0)
				{
					int bestId = -1;

					double bestSqrLength = -1;
					for(size_t i=0 ; i<set.count() ; i++)
					{
						const dseg::SegmentHypothesis* seg = set.segmentAt(i);

						double dx = seg->x1() - seg->x2();
						double dy = seg->y1() - seg->y2();
						double sqrLength = sqrt(dx*dx + dy*dy);
						sqrLength *= seg->gradientDescriptor().meanGradients();

						// If this segment is longer than the previous best
						if(sqrLength > bestSqrLength)
						{
								vec2 v1,v2;

								v1[0] = seg->x1();
								v1[1] = seg->y1();
								v2[0] = seg->x2();
								v2[1] = seg->y2();

								if(roi.isIn(v1) || roi.isIn(v2))
								{
									// Consider this segment as
									bestId = i;
									bestSqrLength = sqrLength;
								}
						}
					}

					if(bestId >= 0)
					{
						featPtr->measurement.x(0) = set.segmentAt(bestId)->x1();
						featPtr->measurement.x(1) = set.segmentAt(bestId)->y1();
						featPtr->measurement.x(2) = set.segmentAt(bestId)->x2();
						featPtr->measurement.x(3) = set.segmentAt(bestId)->y2();
						featPtr->measurement.matchScore = 1;

						featPtr->appearancePtr.reset(new AppearanceImageSegment(params.patchSize, params.patchSize, JfrImage_CS_GRAY, set.segmentAt(bestId)));

						// extract appearance
						vec pix = featPtr->measurement.x();
						vec2 center;
						center[0] = ( pix[0] + pix[2] )/2;
						center[1] = ( pix[1] + pix[3] )/2;

						boost::shared_ptr<AppearanceImageSegment> appPtr = SPTR_CAST<AppearanceImageSegment>(featPtr->appearancePtr);
						rawData->img->extractPatch(appPtr->patch, (int)center(0), (int)center(1), params.patchSize, params.patchSize);
						appPtr->offsetTop.x()(0) = pix(0) - ((int)center(0) - params.patchSize);
						appPtr->offsetTop.x()(1) = pix(1) - ((int)center(1) - params.patchSize);
						appPtr->offsetTop.P() = jblas::zero_mat(2); // by definition this is our landmark projection

						appPtr->offsetBottom.x()(0) = pix(2) - ((int)center(0) - params.patchSize);
						appPtr->offsetBottom.x()(1) = pix(3) - ((int)center(1) - params.patchSize);
						appPtr->offsetBottom.P() = jblas::zero_mat(2); // by definition this is our landmark projection

						vec2 normal;
						normal(0) = pix(1) - pix(3);
						normal(1) = pix(2) - pix(0);
						vec2 top,bottom;
						top[0]    = pix[0]; top[1]    = pix[1];
						bottom[0] = pix[2]; bottom[1] = pix[3];

						jmath::ublasExtra::normalize(normal);
						normal *= 5; // measure area width

						appPtr->patchMeanLeft = meanOnPoly(rawData->img, top, top + normal, bottom + normal, bottom);
						appPtr->patchMeanRight = meanOnPoly(rawData->img, top, top - normal, bottom - normal, bottom);

						ret = true;
					}
				}

				return ret;
         }

			void fillDataObs(const boost::shared_ptr<FeatureImageSegment> & featPtr, boost::shared_ptr<ObservationAbstract> & obsPtr)
         {
            // extract observed appearance
				app_img_seg_ptr_t app_src = SPTR_CAST<AppearanceImageSegment>(featPtr->appearancePtr);
				app_img_seg_ptr_t app_dst = SPTR_CAST<AppearanceImageSegment>(obsPtr->observedAppearance);
            app_dst->setHypothesis(app_src->hypothesis());
				app_src->patch.copyTo(app_dst->patch);
				app_dst->offsetTop = app_src->offsetTop;
				app_dst->offsetBottom = app_src->offsetBottom;

				app_dst->patchMeanLeft = app_src->patchMeanLeft;
				app_dst->patchMeanRight = app_src->patchMeanRight;

            // create descriptor
            descriptor_ptr_t descPtr(descFactory->createDescriptor());
            obsPtr->landmarkPtr()->setDescriptor(descPtr);
         }
   };

}}

#endif //HAVE_MODULE_DSEG

#endif // RAWSEGPROCESSORS_HPP
