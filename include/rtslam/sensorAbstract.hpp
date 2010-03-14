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
//include parents
#include "rtslam/robotAbstract.hpp"
//#include "rtslam/mapAbstract.hpp"

namespace jafar {
	namespace rtslam {

		using namespace std;

		// Forward declarations of children
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
		class SensorAbstract: public MapObject {

			public:

				// Mandatory virtual destructor.
				inline virtual ~SensorAbstract(void) {
				}

				RobotAbstract* robot;
				std::list<ObservationAbstract*> observationsList;

				Gaussian pose;

				/**
				 * Local pose constructor - only mean
				 * Creates a sensor with its own pose information
				 * \param _pose a pose vector
				 */
				//				template<class V>
				SensorAbstract(const jblas::vec & _pose);

				/**
				 * Local pose constructor - full Gaussian.
				 * Creates a sensor with its own pose information.
				 * \param _pose a Gaussian pose
				 */
				SensorAbstract(const Gaussian & _pose);

				/**
				 * Remote pose constructor.
				 * Creates a sensor with the pose indexed in a map.
				 * \param map the map
				 * \param ias the indirect array of indices to the map
				 */
				SensorAbstract(MapAbstract & _map, const jblas::ind_array & _ias);

				/**
				 * Install sensor in robot.
				 * \param rob the robot.
				 */
				void installToRobot(RobotAbstract & rob);

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
				void poseGlobal(jblas::vec7 & poseG, jblas::mat & PG_r, jblas::mat & PG_s);

				/**
				 * Get sensor pose in global frame, for sensor poses with LOCAL storage (see class Gaussian for doc on storage options).
				 * This function composes robot pose with sensor pose to obtain the global sensor pose.
				 * It renders the Jacobian of the composition only wrt the robot pose.
				 * \param poseG the global pose
				 * \param PG_r the Jacobian wrt the robot pose
				 */
				void poseGlobal(jblas::vec7 & poseG, jblas::mat & PG_r);

				/**
				 * Get sensor pose in global frame.
				 * This function composes robot pose with sensor pose to obtain the global sensor pose.
				 * It renders the Jacobians of the composed frame wrt all variables that are in the map (either robot only, or robot and sensor),
				 * depending on the sensor pose storage being LOCAL or REMOTE (see class Gaussian for doc on storage options).
				 * \param poseG the global pose
				 * \param PG_m the Jacobian wrt the mapped states
				 * \return an indirect array with indices to the variables in the map concerned by the Jacobian \a PG_m.
				 */
				jblas::ind_array poseGlobal(jblas::vec7 & poseG, jblas::mat PG_m);

				/**
				 * Operator << for class SensorAbstract.
				 * It shows different information of the sensor.
				 */
				friend std::ostream& operator <<(std::ostream & s, jafar::rtslam::SensorAbstract & sen) {
					s << sen.categoryName() << " " << sen.id() << " of type " << sen.type() << std::endl;
					s << ".pose :  " << sen.pose << std::endl;
					return s;
				}

		};

	}
}

#endif /* SENSORABSTRACT_H_ */
