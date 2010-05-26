/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Copyright RIA-LAAS/CNRS, 2010
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * File:      featureAbstract.h
 * Project:   RT-Slam
 * Author:    Jean-Marie CODOL
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
 * \file featurePointSimu.hpp
 * File defining the simulated feature.
 * \author jmcodol@laas.fr
 * \ingroup rtslam
 */

#ifndef __FeaturePOINTSimu_H__
#define __FeaturePOINTSimu_H__

#include "rtslam/featureAbstract.hpp"
#include "boost/shared_ptr.hpp"

/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */


namespace jafar {
	namespace rtslam {
		using namespace std;

//		class RawImageSimu;
//		typedef boost::shared_ptr<RawImageSimu> rawimagesimu_ptr_t2;


		class FeaturePoint;
		typedef boost::shared_ptr<FeaturePoint> featurepointsimu_ptr_t;

		/** Base class for all landmark appearances defined in the module
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class FeaturePoint: public FeatureAbstract {
			public:
				FeaturePoint() : FeatureAbstract(2) {
				}
				virtual ~FeaturePoint() {
				}
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
