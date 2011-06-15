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

#ifdef HAVE_MODULE_DSEG

#include "rtslam/featureAbstract.hpp"
#include "rtslam/appearanceImage.hpp"
#include "rtslam/appearanceSegment.hpp"
#include "boost/shared_ptr.hpp"

/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */


namespace jafar {
   namespace rtslam {

//		class RawImageSimu;
//		typedef boost::shared_ptr<RawImageSimu> rawimagesimu_ptr_t2;

      class FeatureSegment;
      typedef boost::shared_ptr<FeatureSegment> feat_seg_ptr_t;

      class FeatureSegment: public FeatureAbstract {
         public:
            FeatureSegment() : FeatureAbstract(4, appearance_ptr_t()) {
            }
            virtual ~FeatureSegment() {
            }
            void setup(double u1, double v1, double u2, double v2, double quality);
      };


      class FeatureImageSegment;
      typedef boost::shared_ptr<FeatureImageSegment> feat_img_seg_ptr_t;

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

#endif /* HAVE_MODULE_DSEG */

#endif // #ifndef __FeatureSEGMENT_H__
/*
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * tab-width: 2
 * c-basic-offset: 2
 * End:
 */
