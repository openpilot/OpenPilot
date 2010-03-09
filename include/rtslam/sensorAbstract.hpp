/*
 * sensorAbstract.h
 *
 *  Created on: Jan 28, 2010
 *      Author: jsola
 */

/**
 * \file sensorAbstract.hpp
 * File defining the abstract sensor class
 * \ingroup rtslam
 */

#ifndef SENSORABSTRACT_H_
#define SENSORABSTRACT_H_

/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */

#include <list>

#include "jmath/jblas.hpp"
#include "rtslam/blocks.hpp"
#include "rtslam/robotAbstract.hpp"
#include "rtslam/observationAbstract.hpp"
#include "rtslam/mapAbstract.hpp"

namespace jafar {
	namespace rtslam {

		// Forward declarations
		// TODO: check if this is OK.
		class RobotAbstract;
		class ObservationAbstract;

		/**
		 * Base class for all raw data in module rtslam.
		 * \ingroup rtslam
		 */
		class RawAbstract {
			public:
				/**
				 * Mandatory virtual destructor.
				 */
				inline virtual ~RawAbstract(void) {
				}
				/**
				 * Acquire raw data
				 */
				inline virtual void acquire(void) {
				}
		};

		/**
		 * Base class for all sensors defined in the module rtslam.
		 * \ingroup rtslam
		 */
		class SensorAbstract {

			public:

				// Mandatory virtual destructor.
				inline virtual ~SensorAbstract(void) {
				}

				size_t id;
				std::string name;
				std::string type;

				RobotAbstract* robot;
				std::list<ObservationAbstract*> observationsList;

				Gaussian pose;

				/**
				 * Local pose constructor - only mean
				 * Creates a sensor with its own pose information
				 * \param _pose a pose vector
				 */
				//				template<class V>
				SensorAbstract(const jblas::vec & _pose) :
					pose(_pose) {
				}

				/**
				 * Local pose constructor - full Gaussian.
				 * Creates a sensor with its own pose information.
				 * \param _pose a Gaussian pose
				 */
				SensorAbstract(const Gaussian & _pose) :
					pose(_pose) {
				}

				/**
				 * Remote pose constructor.
				 * Creates a sensor with the pose indexed in a Gaussian map.
				 * \param map the map
				 * \param ias the indirect array of indices to the map
				 */
				SensorAbstract(MapAbstract & map, const jblas::ind_array & ias) :
					pose(map.filter.x, map.filter.P, ias) {
				}

				/*
				 * Acquire raw data
				 */
				virtual void acquireRaw() {
					// raw.acquire();
				}

				//				/*
				//				 * Project all landmarks
				//				 */
				//				void projectAllLandmarks(void);
				//
				//				/*
				//				 * Select most informative observations to update
				//				 */
				//				std::list<size_t> selectMostInformative(size_t numOfLmks);
				//
				//				/*
				//				 * Try to match landmarks
				//				 */
				//				virtual std::list<size_t> match(std::list<size_t>) = 0;

				/**
				 * Operator << for class SensorAbstract.
				 * It shows information of the sensor.
				 */
				friend std::ostream& operator <<(std::ostream & s, jafar::rtslam::SensorAbstract & sen) {
					s << "SENSOR " << sen.id << ": " << sen.name << " of type " << sen.type << endl;
					s << ".pose:  " << sen.pose << endl;
					return s;
				}

		};

	}
}

#endif /* SENSORABSTRACT_H_ */
