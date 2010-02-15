/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Copyright RIA-LAAS/CNRS, 2010
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * File:      LandmarkAbstract.h
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

/**
 * \file landmarkAbstract.hpp
 * File defining the abstract landmark class
 * \ingroup rtslam
 */

#ifndef __LandmarkAbstract_H__
#define __LandmarkAbstract_H__

/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */

#include <list>
#include <jmath/jblas.hpp>

#include "observationAbstract.hpp"
#include "blocks.hpp"
#include "mapAbstract.hpp"

namespace jafar
{

	namespace rtslam
	{

		/** Base class for all landmark descriptors defined in the module
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class DescriptorAbstract
		{
				/**
				 * Mandatory virtual destructor
				 */
				virtual ~DescriptorAbstract(void);
		};

		/* --------------------------------------------------------------------- */
		/* --- CLASS ----------------------------------------------------------- */
		/* --------------------------------------------------------------------- */

		/** Base class for all landmarks defined in the module
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class LandmarkAbstract
		{
			public:

				/**
				 * Mandatory virtual destructor.
				 */
				virtual ~LandmarkAbstract(void);

				/**
				 * Landmark type
				 */
				std::string type;

				/**
				 * Landmark ID
				 */
				std::size_t id;

				/**
				 * List of observations
				 */
				std::list<ObservationAbstract*> observationsList;

				/**
				 * Father map
				 */
				MapAbstract * map;

				/**
				 * state structure
				 */
				State state;

				/**
				 * Descriptor
				 */
				DescriptorAbstract descriptor;

				/**
				 * Reparametrize the landmark.
				 */
				virtual void reparametrize(LandmarkAbstract landmark) = 0;

				/**
				 * Accesssors
				 */
				virtual void setState(jblas::vec& state_);
				virtual void setStateCov(jblas::sym_mat& stateCov_);
				virtual vec getState();
				virtual sym_mat getStateCov();

		};

	}
}

#endif // #ifndef __LandmarkAbstract_H__
/*
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * tab-width: 2
 * c-basic-offset: 2
 * End:
 */
