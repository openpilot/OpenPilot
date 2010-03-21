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
#include "rtslam/robotAbstract.hpp"
#include "rtslam/quatTools.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;


		/*
		 * Operator << for class SensorAbstract.
		 * It shows different information of the sensor.
		 */
		std::ostream& operator <<(std::ostream & s, jafar::rtslam::SensorAbstract & sen) {
			s << sen.categoryName() << " " << sen.id() << ": ";
			if (sen.name().size() > 0)
				s << sen.name() << ", ";
			s << "of type " << sen.type() << std::endl;
			s << ".pose :  " << sen.pose << endl;
			s << ".robot: [ " << sen.robot->id() << " ]";
			return s;
		}


		/**
		 * Empty constructor.
		 * This just defines a pose of size 7.
		 */
		SensorAbstract::SensorAbstract() :
			MapObject(0), pose(7) {
			categoryName("SENSOR");
		}

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
		SensorAbstract::SensorAbstract(MapAbstract & _map) :
			MapObject(_map, 7),
			pose(state, jmath::ublasExtra::ia_range(0,7))
		{
			categoryName("SENSOR");
		}


//		/*
//		 * Remote pose constructor, with robot association.
//		 */
//		SensorAbstract::SensorAbstract(MapAbstract & _map, RobotAbstract & _rob) :
//			MapObject(_map, 7),
//			pose(state, jmath::ublasExtra::ia_range(0,7))
//		{
//			categoryName("SENSOR");
//		}


		/*
		 * Selectable LOCAL or REMOTE pose constructor.
		 */
		SensorAbstract::SensorAbstract(RobotAbstract & _rob, bool inFilter = false) :
			//          #check        # sensor in filter                           : # not in filter
			MapObject(inFilter ? MapObject(*_rob.slamMap, 7)                       : 0),
			pose     (inFilter ? Gaussian(state, jmath::ublasExtra::ia_range(0,7)) : Gaussian(7))
		{
			categoryName("SENSOR");
		}

		void SensorAbstract::addObservation(observation_ptr_t _obsPtr){
//			observationsSet[_obsPtr->id()] = _obsPtr;
		}

		void SensorAbstract::linkToRobot(robot_ptr_t _robPtr){
			robot = _robPtr;
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
			if (pose.storage() == pose.LOCAL) {
				// Sensor is no in the map. Jacobians and indices only wrt robot.
				jblas::ind_array ia(robot->pose.ia());

				return ia;
			}
			else {
				// Sensor is in the map. Give composed Jacobian and indices.
				jblas::ind_array ia(14);
				return ia;
			}
		}

	}
}
