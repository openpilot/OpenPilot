/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Copyright RIA-LAAS/CNRS, 2010
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * File:      rtsLandmarkAbstract.h
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

#ifndef __rtsLandmarkAbstract_H__
#define __rtsLandmarkAbstract_H__

/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */

#include <jmath/jblas.hpp>
#include <list>
#include "rtsBlocks.hpp"
using namespace jblas;
using namespace std;

namespace jafar {

	namespace rtslam {

		/* --------------------------------------------------------------------- */
		/* --- CLASS ----------------------------------------------------------- */
		/* --------------------------------------------------------------------- */

		/** Base class for all landmarks defined in the module
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class rtsLandmarkAbstract {
			public:

				/**
				 * Mandatory virtual destructor.
				 */
				virtual ~rtsLandmarkAbstract(void);

				/**
				 * Landmark type
				 */
				string type;

				/**
				 * Landmark ID
				 */
				int id;

				/**
				 * List of observations
				 */
				list<rtsObservationAbstract> observationsList;

				/**
				 * state structure
				 */
				State state;

				/**
				 * Descriptor
				 */
				rtsDescriptorAbstract descriptor;

				/**
				 * Reparametrize the landmark.
				 */
				virtual void reparametrize(rtsLandmarkAbstract landmark) = 0;

				/**
				 * Accesssors
				 */
				virtual void setState(vec state_);
				virtual void setStateCov(sym_mat stateCov_);
				virtual vec getState();
				virtual sym_mat getStateCov();

		};

	}
}

#endif // #ifndef __rtsLandmarkAbstract_H__
/*
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * tab-width: 2
 * c-basic-offset: 2
 * End:
 */
