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
 * \file featureAbstract.hpp
 * File defining the abstract feature.
 * \author jmcodol@laas.fr
 * \ingroup rtslam
 */

#ifndef __FeatureAbstract_H__
#define __FeatureAbstract_H__

/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */

#include "jmath/jblas.hpp"

#include "rtslam/rtSlam.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;

		/** Base class for all landmark appearances defined in the module
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class FeatureAbstract {
			public:
				int id;
				appearance_ptr_t appearancePtr;
				gaussian_ptr_t statePtr;

				FeatureAbstract() {
				}
				virtual ~FeatureAbstract() {
				}
		};

	}

}

#endif // #ifndef __FeatureAbstract_H__
/*
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * tab-width: 2
 * c-basic-offset: 2
 * End:
 */
