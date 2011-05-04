/*
 * \file appearanceImage.cpp
 * \date 14/04/2010
 * \author jmcodol
 * \ingroup rtslam
 */

#include "rtslam/appearanceImage.hpp"
#include "image/Image.hpp"
#include "jmath/ublasExtra.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;

		AppearanceImagePoint::AppearanceImagePoint(const image::Image& patch, Gaussian const &offset):
			offset(offset)
		{
			patch.copyTo(this->patch);
			computePatchIntegrals();
		}

		AppearanceImagePoint::~AppearanceImagePoint() {
		}

		AppearanceAbstract* AppearanceImagePoint::clone()
		{
			AppearanceImagePoint* app = new AppearanceImagePoint(patch.width(), patch.height(), patch.depth());
			patch.copy(app->patch,0,0,0,0);
			app->patchSum = patchSum;
			app->patchSquareSum = patchSquareSum;
			app->offset = offset;
			return app;
		}

		void AppearanceImagePoint::computePatchIntegrals(){
			patchSum = 0;
			patchSquareSum = 0;
			uchar* pix = patch.data();
			for (int u = 0; u < patch.width()*patch.height(); u++){
				patchSum += *pix;
				patchSquareSum += (*pix) * (*pix);
				pix ++;
			}
		}

      /**
       *
       *  AppearanceImageSegment
       *
      **/

		AppearanceImageSegment::AppearanceImageSegment(const image::Image& patch, Gaussian const &offsetTop, Gaussian const& offsetBottom, dseg::SegmentHypothesis* _hypothesis):
			offsetTop(offsetTop), offsetBottom(offsetBottom)
      {
         patch.copyTo(this->patch);
			computePatchMeans();
			if(_hypothesis != NULL)m_hypothesis.addSegment(_hypothesis);
		}

      AppearanceImageSegment::~AppearanceImageSegment() {
      }

      AppearanceAbstract* AppearanceImageSegment::clone()
      {
			AppearanceImageSegment* app = new AppearanceImageSegment(patch.width(), patch.height(), patch.depth(), hypothesis());
         patch.copy(app->patch,0,0,0,0);
			app->patchMeanLeft = patchMeanLeft;
			app->patchMeanRight = patchMeanRight;
			app->offsetTop = offsetTop;
			app->offsetBottom = offsetBottom;
			return app;
      }

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
		float meanOnPoly(image::Image& patch, const jblas::vec2& v1, const jblas::vec2& v2, const jblas::vec2& v3, const jblas::vec2& v4)
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
					int y = line + patch.height() / 2;
					int center = patch.width() / 2;
					for(int x=col_beg + center ; x<=col_end + center; x++)
					{
						if(x<0 || x>=patch.width() || y<0 || y>=patch.height())
							continue;

						sum += patch.data()[y*patch.width() + x];
						// patch.data()[y*patch.width() + x] = 255 - patch.data()[y*patch.width() + x]; // DEBUG : invert color in order to visualise the area
						nbmeasure++;
					}

					// Update line
					line--;
				}
			}

			return (nbmeasure > 0) ? sum / nbmeasure : 0;
		}

		void AppearanceImageSegment::computePatchMeans(){
			jblas::vec2 normal;
			normal(0) = offsetTop.x()(1) - offsetBottom.x()(1);
			normal(1) = offsetBottom.x()(0) - offsetTop.x()(0);

			jmath::ublasExtra::normalize(normal);
			normal *= 5; // measure area width

			patchMeanLeft = meanOnPoly(patch, offsetTop.x(), offsetTop.x() + normal, offsetBottom.x() + normal, offsetBottom.x());
			patchMeanRight = meanOnPoly(patch, offsetTop.x(), offsetTop.x() - normal, offsetBottom.x() - normal, offsetBottom.x());
      }

	}
}
