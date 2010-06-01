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

#include "rtslam/objectAbstract.hpp"

#include <iostream>

namespace jafar {
	namespace rtslam {
		using namespace std;

		class RawAbstract: public ObjectAbstract {
			private:
			public:
				enum detect_method {
					HARRIS ///< Harris corner detector.
				};
				RawAbstract();
				virtual ~RawAbstract();
				/*
				 * Operator << for class rawAbstract.
				 * It shows some informations
				 */
				friend ostream& operator <<(ostream & s, jafar::rtslam::RawAbstract & rawA);
//				virtual bool detect(const detect_method met, feature_ptr_t & featurePtr) = 0;
		};
	}
}
#endif /* RAWABSTRACT_HPP_ */



