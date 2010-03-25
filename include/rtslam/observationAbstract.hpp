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
				 */
				ObservationAbstract(size_t _size);

				/**
				 * Sizes constructor
				 */
				ObservationAbstract(size_t _size_meas, size_t _size_exp, size_t _size_inn);

				//				/**
				//				 * Sensor and landmark constructor
				//				 */
				//				ObservationAbstract(SensorAbstract & _sen, LandmarkAbstract & _lmk);

				/**
				 *  Mother Sensor where it was acquired from
				 */
				sensor_ptr_t sensor;

				/**
				 * Father Landmark where it points to
				 */
				landmark_ptr_t landmark;

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

				/**
				 * Associate to sensor and landmark.
				 * This sets several parameters such as identifiers and pointers to sensor and landmark ancestors.
				 * \param sen the sensor
				 * \param lmk the landmark
				 */
				inline void associate(sensor_ptr_t senPtr, landmark_ptr_t lmkPtr);

				/**
				 * Link to sensor.
				 */
				void linkToSensor(sensor_ptr_t _sensorPtr);

				/**
				 * Link to landmark
				 */
				void linkToLandmark(landmark_ptr_t _lmkPtr);

				/**
				 * Project and get Jacobians
				 */
				//				virtual void project() = 0;

				/**
				 * Is visible
				 * \return true if visible
				 */
				virtual bool isVisible() {
					return events.visible;
				}


				/**
				 * match
				 */
				//				virtual void match();

				/**
				 * Individual consistency check
				 */
				//				virtual void isIndividuallyConsistent();

				/**
				 * Back-project
				 */
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
