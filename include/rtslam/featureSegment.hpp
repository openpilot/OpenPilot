/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Copyright RIA-LAAS/CNRS, 2011
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * File:      featureSegment.hpp
 * Project:   RT-Slam
 * Author:    Benjamin HAUTBOIS
 *
 * Version control
 * ===============
 *
 *  $Id$
 *
 * Description
 * ============
 *
 *
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/**
 * \file featureSegment.hpp
 * File defining the segment feature.
 * \author bhautboi@laas.fr
 * \ingroup rtslam
 */

#ifndef __FeatureSEGMENT_H__
#define __FeatureSEGMENT_H__

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


      class FeatureImageSegment;
      typedef boost::shared_ptr<FeatureImageSegment> feat_img_seg_ptr_t;

      /** Base class for all landmark appearances defined in the module
       * rtslam.
       *
       * @ingroup rtslam
       */
      class FeatureImageSegment: public FeatureAbstract {
         public:
            FeatureImageSegment() : FeatureAbstract(4, appearance_ptr_t()) {
            }
            FeatureImageSegment(int width, int height, int depth)  : FeatureAbstract(4, appearance_ptr_t(new AppearanceImageSegment(width,height,depth))) {
            }
            virtual ~FeatureImageSegment() {
            }
            void setup(double u1, double v1, double u2, double v2, double quality);
      };
   }

}

#endif // #ifndef __FeatureSEGMENT_H__
/*
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * tab-width: 2
 * c-basic-offset: 2
 * End:
 */
