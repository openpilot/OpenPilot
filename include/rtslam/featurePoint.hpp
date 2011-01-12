/**
 * \file featurePoint.hpp
 * \author jmcodol
 * \ingroup rtslam
 */

#ifndef __FeaturePOINTSimu_H__
#define __FeaturePOINTSimu_H__

#include "rtslam/featureAbstract.hpp"
#include "rtslam/appearanceImage.hpp"
#include "boost/shared_ptr.hpp"

/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */


namespace jafar {
	namespace rtslam {

//		class RawImageSimu;
//		typedef boost::shared_ptr<RawImageSimu> rawimagesimu_ptr_t2;


		class FeatureImagePoint;
		typedef boost::shared_ptr<FeatureImagePoint> feat_img_pnt_ptr_t;

		/** Base class for all landmark appearances defined in the module
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class FeatureImagePoint: public FeatureAbstract {
			public:
				FeatureImagePoint() : FeatureAbstract(2, appearance_ptr_t()) {
				}
				FeatureImagePoint(int width, int height, int depth)  : FeatureAbstract(2, appearance_ptr_t(new AppearanceImagePoint(width,height,depth))) {
				}
				virtual ~FeatureImagePoint() {
				}
				void setup(double u, double v, double quality);
		};
	}

}

#endif // #ifndef __FeaturePOINTSimu_H__
/*
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * tab-width: 2
 * c-basic-offset: 2
 * End:
 */
