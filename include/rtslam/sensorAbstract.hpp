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

#include <map>
#include <boost/shared_ptr.hpp>

#include "jmath/jblas.hpp"
#include "rtslam/rtSlam.hpp"
//include parents
#include "rtslam/mapAbstract.hpp"
#include "rtslam/mapObject.hpp"
#include "rtslam/robotAbstract.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;


		// Forward declarations of children
		class ObservationAbstract;

//		typedef boost::shared_ptr<ObservationAbstract> observation_ptr_t;
//		typedef std::map<size_t, observation_ptr_t> observations_ptr_set_t;
//		typedef boost::shared_ptr<RobotAbstract> robot_ptr_t;

		/**
		 * Base class for all parameter sets in module rtslam.
		 * \ingroup rtslam
		 */
		class ParametersAbstract {
			public:
				/**
				 * Mandatory virtual destructor.
				 */
				inline virtual ~ParametersAbstract(void) {
				}

		};



		/**
		 * Base class for all raw data in module rtslam.
		 * \ingroup rtslam
		 */
		class RawAbstract {
			public:
				/**
				 * Mandatory virtual destructor.
				 */
				inline virtual ~RawAbstract() {
				}
				/**
				 * Acquire raw data
				 */
				inline virtual void acquire() {
				}
		};


		/**
		 * Base class for all sensors defined in the module rtslam.
		 * \ingroup rtslam
		 */
		class SensorAbstract: public MapObject {

				friend std::ostream& operator <<(std::ostream & s, jafar::rtslam::SensorAbstract & sen);

			public:


				/**
				 * Parent robot
				 */
				robot_ptr_t robot;

				/**
				 * A set of observations (one per landmark)
				 */
				observations_ptr_set_t observations;

				/**
				 * Sensor pose in robot
				 */
				Gaussian pose;

				/**
				 * Empty constructor.
				 * This just defines a pose of size 7.
				 */
				SensorAbstract();

				/**
				 * Local pose constructor - only mean
				 * Creates a sensor with its own pose information
				 * \param _pose a pose vector
				 */
				SensorAbstract(const jblas::vec7 & _pose);

				/**
				 * Local pose constructor - full Gaussian.
				 * Creates a sensor with its own pose information.
				 * \param _pose a Gaussian pose
				 */
				SensorAbstract(const Gaussian & _pose);

				/**
				 * Remote pose constructor.
				 * Creates a sensor with the pose indexed in a map.
				 * \param _map the slam map
				 */
				SensorAbstract(MapAbstract & _map);

				/**
				 * Remote pose constructor, with sensor association.
				 * Creates a sensor with the pose indexed in a map,
				 * and installed on a robot.
				 * \param _map the map
				 * \param _rob the robot
				 */
				SensorAbstract(MapAbstract & _map, RobotAbstract & _rob);

				/**
				 * Selectable LOCAL or REMOTE pose constructor.
				 * Creates a sensor with the pose indexed in a map.
				 * \param _rob the robot
				 * \param inFilter flag indicating if the sensor state is part of the filter (REMOTE).
				 */
				SensorAbstract(RobotAbstract & _rob, bool inFilter);

				/**
				 * Mandatory virtual destructor.
				 */
				virtual ~SensorAbstract() {
				}

				/**
				 * Add an observation
				 */
				void addObservation(observation_ptr_t _obsPtr);

				/**
				 * Link to robot
				 */
				void linkToRobot(robot_ptr_t _robPtr);

				/*
				 * Acquire raw data
				 */
				virtual void acquireRaw() {
					// raw.acquire();
				}


				/**
				 * Get sensor pose in global frame.
				 * This function composes robot pose with sensor pose to obtain the global sensor pose.
				 * It renders the Jacobians of the composition.
				 * \param poseG the global pose
				 * \param PG_r the Jacobian wrt the robot pose
				 * \param PG_s the Jacobian wrt the sensor local pose
				 */
				void globalPose(jblas::vec7 & poseG, jblas::mat & PG_r, jblas::mat & PG_s) ;

				/**
				 * Get sensor pose in global frame, for sensor poses with LOCAL storage (see class Gaussian for doc on storage options).
				 * This function composes robot pose with sensor pose to obtain the global sensor pose.
				 * It renders the Jacobian of the composition only wrt the robot pose.
				 * \param poseG the global pose
				 * \param PG_r the Jacobian wrt the robot pose
				 */
				void globalPose(jblas::vec7 & poseG, jblas::mat & PG_r) ;

				/**
				 * Get sensor pose in global frame.
				 * This function composes robot pose with sensor pose to obtain the global sensor pose.
				 * It renders the Jacobians of the composed frame wrt all variables that are in the map (either robot only, or robot and sensor),
				 * depending on the sensor pose storage being LOCAL or REMOTE (see class Gaussian for doc on storage options).
				 * \param poseG the global pose
				 * \param PG_m the Jacobian wrt the mapped states
				 * \return an indirect array with indices to the variables in the map concerned by the Jacobian \a PG_m.
				 */
				jblas::ind_array globalPoseInMap(jblas::vec7 & poseG, jblas::mat & PG_m) ;

		};

	}
}

#endif /* SENSORABSTRACT_H_ */
