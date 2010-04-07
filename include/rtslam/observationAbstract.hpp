/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Copyright RIA-LAAS/CNRS, 2010
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * File:      observationAbstract.h
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
 * \file observationAbstract.hpp
 * File defining the abstract observation class
 * \author jsola@laas.fr
 * \ingroup rtslam
 */

#ifndef __ObservationAbstract_H__
#define __ObservationAbstract_H__

/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */

#include <jmath/jblas.hpp>

#include "rtslam/rtSlam.hpp"
// include parents
#include "rtslam/objectAbstract.hpp"
#include "rtslam/sensorAbstract.hpp"
#include "rtslam/landmarkAbstract.hpp"

#include "rtslam/gaussian.hpp"
#include "rtslam/innovation.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;


		/** Base class for all landmark appearances defined in the module
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class AppearanceAbstract {
			public:
				/**
				 * Mandatory virtual destructor
				 */
				virtual ~AppearanceAbstract() {
				}
		};

		typedef boost::shared_ptr<AppearanceAbstract> appearance_ptr_t;


		/**
		 * Base class for all observations defined in the module rtslam.
		 * \author jsola
		 *
		 * In this class, the Jacobians are sparse. The states that contribute to the observation are available through an indirect array
		 * - this->ia_rsl
		 *
		 * so that we have, for example:
		 * - expectation.x() = h( project(x, ia_rsl) )
		 * - expectation.P() = EXP_rsl * project(P, ia_rsl, ia_rsl) * EXP_rsl'
		 *
		 * \ingroup rtslam
		 */
		class ObservationAbstract: public ObjectAbstract {

				friend std::ostream& operator <<(std::ostream & s, jafar::rtslam::ObservationAbstract & obs);

			public:



				/**
				 * Size constructor
				 * \param _size size of measurement space (used for measurement, expectation and innovation).
				 */
				ObservationAbstract(const sensor_ptr_t & _senPtr, const landmark_ptr_t & _lmkPtr, const size_t _size, const size_t size_nonobs = 0);

				/**
				 * Sizes constructor
				 */
				ObservationAbstract(const sensor_ptr_t & _senPtr, const landmark_ptr_t & _lmkPtr, const size_t _size_meas, const size_t _size_exp, const size_t _size_inn,
				    const size_t _size_nonobs = 0);

				virtual ~ObservationAbstract() {
				}

				// Links
				sensor_ptr_t sensorPtr; ///<       Mother Sensor where it was acquired from
				landmark_ptr_t landmarkPtr; ///<   Father Landmark where it points to

				// Data
				Expectation expectation;
				Measurement measurement;
				Innovation innovation;

				ind_array ia_rsl; ///<          Ind. array of indices to the map
				jblas::mat EXP_rsl; ///<        Jacobian of the expectation wrt the states of robot, sensor and landmark.
				jblas::mat INN_meas; ///<       The Jacobian of the innovation wrt the measurement.
				jblas::mat INN_exp; ///<        The Jacobian of the innovation wrt the expectation.
				jblas::mat INN_rsl; ///<        The Jacobian of the innovation wrt the states of robot, sensor and landmark.
				jblas::vec nonObs; ///<         Expected value of the non-observable part.


				/**
				 * Counters
				 */
				struct counters {
						int nSearch; ///< Number of searches
						int nMatch; ///< number of matches
						int nInlier; ///< Number of times declared inlier
				} counters;

				/**
				 * Events
				 */
				struct events {
						bool visible; ///< Landmark is visible
						bool measured; ///< Feature is measured
						bool matched; ///< Feature is matched
						bool updated; ///< Landmark is updated
				} events;

				void link(const sensor_ptr_t & _senPtr, const landmark_ptr_t & _lmkPtr); ///< Link to sensor and landmark

				/**
				 * Project and get Jacobians.
				 *
				 * All variables are part of the class, or are accessible by the class.
				 *
				 * This projects the landmark into the sensor space, and gives the Jacobians of this projection
				 * wrt the states that contributed to the projection (those of the robot, eventually the sensor, and the landmark).
				 * These states are also available through the indirect_array \a ia_rsl, updated by this function.
				 */
				virtual void project_func() = 0;

				/**
				 * Project and get expectation covariances
				 */
				void project();

				/**
				 * Is visible
				 * \return true if visible
				 */
				virtual bool isVisible() {
					return events.visible;
				}


				/**
				 * Back-project
				 */
				virtual void backProject_func() = 0;

		};

	}

}

#endif // #ifndef __ObservationAbstract_H__
/*
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * tab-width: 2
 * c-basic-offset: 2
 * End:
 */
