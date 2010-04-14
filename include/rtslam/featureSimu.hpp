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
 * \file featureSimu.hpp
 * File defining the simulated feature.
 * \author jmcodol@laas.fr
 * \ingroup rtslam
 */

#ifndef __FeatureSimu_H__
#define __FeatureSimu_H__

/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */


namespace jafar {
	namespace rtslam {
		using namespace std;

		/** Base class for all landmark appearances defined in the module
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class FeatureSimu: public FeatureAbstract {
			public:
				FeatureSimu() {
				}
				virtual ~FeatureSimu() {
				}
		};
	}

}

#endif // #ifndef __FeatureSimu_H__
/*
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * tab-width: 2
 * c-basic-offset: 2
 * End:
 */
