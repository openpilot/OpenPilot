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
		 * \ingroup rtslam
		 */
		class ObservationAbstract: public ObjectAbstract {

				friend std::ostream& operator <<(std::ostream & s, jafar::rtslam::ObservationAbstract & obs);

			public:

				virtual ~ObservationAbstract() {
				}


				/**
				 * Size constructor
				 * \param _size size of measurement space (used for measurement, expectation and innovation).
				 */
				ObservationAbstract(const size_t _size);

				/**
				 * Sizes constructor
				 */
				ObservationAbstract(const size_t _size_meas, const size_t _size_exp, const size_t _size_inn);

				sensor_ptr_t sensor; ///<			   Mother Sensor where it was acquired from
				landmark_ptr_t landmark; ///< 	 Father Landmark where it points to

				Expectation expectation;
				Measurement measurement;
				Innovation innovation;

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

				void linkToSensor(sensor_ptr_t _sensorPtr); ///<  Link to sensor
				void linkToLandmark(landmark_ptr_t _lmkPtr); ///< Link to landmark

				/**
				 * Project and get Jacobians.
				 *
				 * All variables are part of the class, or are accessible by the class.
				 *
				 * This projects the landmark into the sensor space, and gives the Jacobians of this projection
				 * wrt the states that contributed to the projection (those of the robot, eventually the sensor, and the landmark).
				 * These states are also available through the indirect_array \a ia_rsl, updated by this function.
				 */
				virtual void project_func(){}

				/**
				 * Project and get expectation covariances
				 */
				void project() {
					project_func();
					expectation.P() = ublasExtra::prod_JPJt(
							ublas::project(landmark->slamMap->filter.P(), expectation.ia_rsl, expectation.ia_rsl),
							expectation.EXP_rsl);
				}


				/**
				 * Is visible
				 * \return true if visible
				 */
				virtual bool isVisible() {
					return events.visible;
				}


				//				/**
				//				 * Back-project
				//				 */
				//				virtual void back_project() = 0;

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
