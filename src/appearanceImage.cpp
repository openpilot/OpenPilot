/*
 * \file appearanceImage.cpp
 * \date 14/04/2010
 * \author jmcodol
 * \ingroup rtslam
 */

#include "rtslam/appearanceImage.hpp"
#include "image/Image.hpp"

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

		void AppearanceImageSegment::computePatchMeans(){
			patchMeanLeft = meanOnLine(patch,
												offsetTop.x()(0),offsetTop.x()(1),
												offsetBottom.x()(0),offsetBottom.x()(1));
			patchMeanRight = meanOnLine(patch,
												offsetTop.x()(0),offsetTop.x()(1),
												offsetBottom.x()(0),offsetBottom.x()(1));
      }

	}
}
