/**
 * sensorAbstract.cpp
 *
 *  Created on: 10/03/2010
 *      Author: jsola
 *
 *  \file sensorAbstract.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/sensorAbstract.hpp"
#include "rtslam/quatTools.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;


		/*
		 * Local pose constructor - only mean
		 */
		//				template<class V>
		SensorAbstract::SensorAbstract(const jblas::vec7 & _pose) :
			MapObject(0), pose(_pose) {
			categoryName("SENSOR");
		}


		/*
		 * Local pose constructor - full Gaussian.
		 */
		SensorAbstract::SensorAbstract(const Gaussian & _pose) :
			MapObject(0), pose(_pose) {
			categoryName("SENSOR");
		}


		/*
		 * Remote pose constructor.
		 */
		SensorAbstract::SensorAbstract(MapAbstract & _map, const jblas::ind_array & _ias) :
			MapObject(_map, _ias), pose(_map.filter.x, _map.filter.P, jafar::jmath::ublasExtra::ia_head(_ias, 7)) {
			categoryName("SENSOR");
		}

		void SensorAbstract::installToRobot(RobotAbstract & rob) {


			//			rob.sensorsList.push_back(this);
			robot = &rob;
		}


		/*
		 * Get sensor pose in global frame.
		 */
		void SensorAbstract::globalPose(jblas::vec7 & poseG, jblas::mat & PG_r, jblas::mat & PG_s) {
			jblas::vec7 robotPose = robot->pose.x();
			jblas::vec7 sensorPose = pose.x();
			quaternion::composeFrames(robotPose, sensorPose, poseG, PG_r, PG_s);
		}


		/*
		 * Get sensor pose in global frame, for sensor poses with LOCAL storage (see class Gaussian for doc on storage options).
		 */
		void SensorAbstract::globalPose(jblas::vec7 & poseG, jblas::mat & PG_r) {
			jblas::vec7 robotPose = robot->pose.x();
			jblas::vec7 sensorPose = pose.x();
			poseG = quaternion::composeFrames(robotPose, sensorPose);
			quaternion::composeFrames_by_dglobal(robotPose, sensorPose, PG_r);
		}


	/*
	 * Get sensor pose in global frame.
	 */
			jblas::ind_array SensorAbstract::globalPoseInMap(jblas::vec7 & poseG, jblas::mat & PG_m) {
				jblas::vec7 robotPose = robot->pose.x();
				jblas::vec7 sensorPose = pose.x();
				if (pose.storage() == pose.LOCAL){
					// Sensor is no in the map. Jacobians and indices only wrt robot.

				}else{
					// Sensor is in the map. Give composed Jacobian and indices.

				}

			}


	}
}
