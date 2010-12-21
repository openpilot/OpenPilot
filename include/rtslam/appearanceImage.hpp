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

#include "jmath/jblas.hpp"
#include "image/Image.hpp"
#include "rtslam/appearanceAbstract.hpp"
#include "boost/shared_ptr.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;

		class AppearanceImagePoint;
		typedef boost::shared_ptr<AppearanceImagePoint> app_img_pnt_ptr_t;

		/** Appearence for matching
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class AppearanceImagePoint: public AppearanceAbstract {
			public:
				image::Image patch;
				unsigned int patchSum;
				unsigned int patchSquareSum;
				jblas::vec2 offset; ///< offset between the center of the patch and the real position of the feature
			public:
				AppearanceImagePoint(const image::Image& patch, jblas::vec2 const &offset);
				AppearanceImagePoint(int width, int height, int depth):
					patch(width, height, depth, JfrImage_CS_GRAY) {
//					cout << "Created patch with " << width << "x" << height << " pixels; depth: " << depth << "; color space: " << JfrImage_CS_GRAY << endl;
				}
				virtual ~AppearanceImagePoint();
				virtual AppearanceAbstract* clone();

			private:
				void computePatchIntegrals();
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
