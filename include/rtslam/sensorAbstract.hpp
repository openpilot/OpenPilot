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

namespace jafar
{
	namespace rtslam
	{

		/* --------------------------------------------------------------------- */
		/* --- CLASS ----------------------------------------------------------- */
		/* --------------------------------------------------------------------- */

		/** Base class for all sensors defined in the module
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class SensorAbstract
		{
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
				 * Flag for including pose in map for calibration
				 */
				storage_type poseInMap;

				/**
				 * Father robot
				 */
				RobotAbstract* robot;

				/**
				 * pose (local or remote in Map)
				 */
				Pose pose;

				/**
				 * parameters
				 */
				struct parameters
				{
				};

				/**
				 * Observations list
				 */
				std::list<ObservationAbstract*> observationsList;

				/**
				 * Accessors
				 */
				virtual void setPose(jblas::vec& pose_);
				virtual void setPoseCov(jblas::sym_mat& poseCov_);
				virtual vec getPose();
				virtual sym_mat getPoseCov();
				virtual void setState(jblas::vec& state_);
				virtual void setStateCov(jblas::sym_mat& stateCov_);
				virtual vec getState();
				virtual sym_mat getStateCov();

				/**
				 * Acquire raw data
				 */
				virtual void acquire();

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
				virtual std::list<size_t> match(std::list<size_t> ) = 0;

				/**
				 *
				 */
		};

	}
}

#endif /* SENSORABSTRACT_H_ */
