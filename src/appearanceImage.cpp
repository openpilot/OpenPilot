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

      AppearanceImageSegment::AppearanceImageSegment(const image::Image& patch, Gaussian const &offset):
         offset(offset)
      {
         patch.copyTo(this->patch);
         computePatchIntegrals();
      }

      AppearanceImageSegment::~AppearanceImageSegment() {
      }

      AppearanceAbstract* AppearanceImageSegment::clone()
      {
         AppearanceImageSegment* app = new AppearanceImageSegment(patch.width(), patch.height(), patch.depth());
         patch.copy(app->patch,0,0,0,0);
         app->patchSum = patchSum;
         app->patchSquareSum = patchSquareSum;
         app->offset = offset;
         return app;
      }

      void AppearanceImageSegment::computePatchIntegrals(){
         patchSum = 0;
         patchSquareSum = 0;
         uchar* pix = patch.data();
         for (int u = 0; u < patch.width()*patch.height(); u++){
            patchSum += *pix;
            patchSquareSum += (*pix) * (*pix);
            pix ++;
         }
      }

	}
}
