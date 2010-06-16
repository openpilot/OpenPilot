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
#include "rtslam/measurement.hpp"
#include <cv.h> // opencv 

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
				enum match_method {
					ZNCC ///< Zncc matching algorithm
				};

				virtual ~RawAbstract();

				virtual bool detect(const detect_method met, const feature_ptr_t & featPtr, jafar::image::ROI* roiPtr = 0) = 0;
				virtual bool match(const match_method met, const appearance_ptr_t & targetApp, cv::Rect &roi, Measurement & measure, const appearance_ptr_t & app) = 0;

				virtual void extractAppearance(const jblas::veci & pos, const jblas::veci & size, appearance_ptr_t & appPtr) = 0;

				//				virtual bool match(observation_ptr_t & obsPtr) = 0;

		};
	}
}
#endif /* RAWABSTRACT_HPP_ */



