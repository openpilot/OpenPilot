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
#include "rtslam/objectAbstract.hpp"
#include "rtslam/gaussian.hpp"
#include "rtslam/measurement.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;

		/** Base class for all landmark appearances defined in the module
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class FeatureAbstract : public ObjectAbstract {
			public:
				appearance_ptr_t appearancePtr;
				Measurement measurement;

				FeatureAbstract(): measurement(0) {}
				FeatureAbstract(size_t size, appearance_ptr_t appearancePtr_) : 
					appearancePtr(appearancePtr_), measurement(size)
				{
					measurement.matchScore = 0.0;
				}

				virtual ~FeatureAbstract() {
				}

				virtual std::string categoryName() {
					return "FEATURE";
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
