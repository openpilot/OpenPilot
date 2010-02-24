/*
 * SensorAbstract.h
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
#include "blocks.hpp"
#include "robotAbstract.hpp"
#include "observationAbstract.hpp"

namespace jafar {
	namespace rtslam {


		/**
		 * Base class for all raw data in module rtslam.
		 */
		class RawAbstract {
				/**
				 * Mandatory virtual destructor.
				 */
				virtual ~RawAbstract(void);
				/**
				 * Acquire raw data
				 */
				virtual void acquire() = 0;

		};

		/** Base class for all sensors defined in the module rtslam.
		 *
		 * @ingroup rtslam
		 */
		class SensorAbstract {
			public:

				/**
				 * Mandatory virtual destructor.
				 */
				virtual ~SensorAbstract(void);

				/**
				 * Sensor name
				 */
				std::string name;

				/**
				 * Sensor type
				 */
				std::string type;

				/**
				 * Father robot
				 */
				RobotAbstract* robot;

				/**
				 * Flag for including pose in map for calibration
				 */
				storage_type pose_storage_type;

				/**
				 * pose (local or remote in Map)
				 */
				Pose pose;

				/**
				 * parameters
				 */
				ParametersAbstract parameters;

				/**
				 * Raw data
				 */
				RawAbstract raw;

				/**
				 * Observations list
				 */
				std::list<ObservationAbstract*> observationsList;

				/**
				 * Acquire raw data
				 */
				virtual void acquireRaw(){raw.acquire();};

				/**
				 * Project all landmarks
				 */
				void projectAllLandmarks(void);

				/**
				 * Select most informative observations to update
				 */
				std::list<size_t> selectMostInformative(size_t numOfLmks);

				/**
				 * Try to match landmarks
				 */
				virtual std::list<size_t> match(std::list<size_t>) = 0;

				/**
				 *
				 */
		};

	}
}

#endif /* SENSORABSTRACT_H_ */
