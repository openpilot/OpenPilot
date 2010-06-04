/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Copyright RIA-LAAS/CNRS, 2010
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * File:      rawAbstract.hpp
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
 *	The Raws are used to store informations from sensors (exemple: image from camera,
 *	    gps pseudo-distances from GPS sensor, ...).
 *
 *
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/**
 * \file rawAbstract.hpp
 * File defining the abstract raw-datas class
 * \author jmcodol@laas.fr
 * \ingroup rtslam
 */

#ifndef RAWABSTRACT_HPP_
#define RAWABSTRACT_HPP_

/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */

#include <iostream>

#include "image/roi.hpp"

#include "rtslam/objectAbstract.hpp"


namespace jafar {
	namespace rtslam {
		using namespace std;

		class RawAbstract: public ObjectAbstract {

				/*
				 * Operator << for class rawAbstract.
				 * It shows some informations
				 */
				friend ostream& operator <<(ostream & s, jafar::rtslam::RawAbstract & rawA);

			public:
				enum detect_method {
					HARRIS ///< Harris corner detector.
				};

				virtual ~RawAbstract();

				virtual bool detect(const detect_method met, feature_ptr_t & featPtr, jafar::image::ROI* roiPtr = 0) = 0;

				//				virtual bool match(observation_ptr_t & obsPtr) = 0;

		};
	}
}
#endif /* RAWABSTRACT_HPP_ */



