/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Copyright RIA-LAAS/CNRS, 2010
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * File:      rtsObservationAbstract.h
 * Project:   RT-Slam
 * Author:    Joan Sola
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

#ifndef __rtsObservationAbstract_H__
#define __rtsObservationAbstract_H__

/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */

#include <jmath/jblas.hpp>
#include <list>
#include "rtsBlocks.hpp"
#include "rtsSensorAbstract.hpp"
#include "rtsLandmarkAbstract.hpp"
using namespace jblas;
using namespace std;

namespace jafar {

	namespace rtslam {

		/* --------------------------------------------------------------------- */
		/* --- CLASS ----------------------------------------------------------- */
		/* --------------------------------------------------------------------- */

		/** Base class for all observations defined in the module
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class rtsObservationAbstract {
			public:

				/**
				 * Mandatory virtual destructor.
				 */
				virtual ~rtsObservationAbstract(void);

				rtsSensorAbstract * sensor; // Sensor from where it was acquired
				rtsLandmarkAbstract * landmark; // Landmark observed

				/**
				 * expectation
				 */
				Expectation expectation;

				/**
				 * Measurement
				 */
				Measurement measurement;

				/**
				 * innovation
				 */
				Innovation innovation;

				/**
				 * counters
				 */
				struct counters {
						int searches;
						int matches;
						int validations;
				};

				/**
				 * Project
				 */
				virtual void project(void);

				/**
				 * Back-project
				 */
				virtual void back_project(void);

				/**
				 * Try to match
				 */
				virtual void match(void) = 0;

				/**
				 * Innovation
				 */
				virtual void computeInnovation(void){
					innovation.compute(measurement,expectation);
				}

		};

	}
}

#endif // #ifndef __rtsObservationAbstract_H__
/*
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * tab-width: 2
 * c-basic-offset: 2
 * End:
 */
