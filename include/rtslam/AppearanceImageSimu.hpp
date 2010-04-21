/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Copyright RIA-LAAS/CNRS, 2010
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * File:      AppreatenceImageSimu.h
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
 * \file AppearanceImageSimu.hpp
 * File defining the simulated appearence.
 * \author jmcodol@laas.fr
 * \ingroup rtslam
 */

#ifndef __AppearenceImageSimu_H__
#define __AppearenceImageSimu_H__

/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */

#include "rtslam/appearanceAbstract.hpp"
#include "boost/shared_ptr.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;

		class AppearenceImageSimu;
		typedef boost::shared_ptr<AppearenceImageSimu> appearenceimagesimu_ptr_t;

		/** Appearence for matching
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class AppearenceImageSimu: public AppearanceAbstract {
			public:
				AppearenceImageSimu();
				virtual ~AppearenceImageSimu();
		};
	}

}

#endif // #ifndef __AppearenceImageSimu_H__
/*
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * tab-width: 2
 * c-basic-offset: 2
 * End:
 */
