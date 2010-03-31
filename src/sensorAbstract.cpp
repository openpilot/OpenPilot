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
#include "boost/shared_ptr.hpp"
#include "jmath/indirectArray.hpp"
#include "rtslam/sensorAbstract.hpp"
#include "rtslam/robotAbstract.hpp"
#include "rtslam/observationAbstract.hpp"
#include "rtslam/quatTools.hpp"

// TODO this needs to go out of here - when we'll have factories
#include "rtslam/landmarkAnchoredHomogeneousPoint.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;
		using namespace boost;
		using namespace ublasExtra;


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
			MapObject(_map, 7), pose(state, jmath::ublasExtra::ia_range(0, 7)) {
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
			    MapObject(inFilter ? MapObject(*_rob.slamMap, 7) : 0), pose(inFilter ? Gaussian(state,
			        jmath::ublasExtra::ia_range(0, 7)) : Gaussian(7)) {
			categoryName("SENSOR");
		}


		/*
		 * Acquire raw data.
		 */
		void SensorAbstract::acquireRaw() {
			// TODO Acquire raw data
		}


		/**
		 * Process raw data.
		 */
		void SensorAbstract::processRaw() {
			observeKnownLmks();
			discoverNewLmks();
		}

		void SensorAbstract::observeKnownLmks() {
			for (observations_ptr_set_t::iterator obsIter = observations.begin(); obsIter != observations.end(); obsIter++) {
				observation_ptr_t obsPtr = obsIter->second;
				cout << "exploring obs: " << obsPtr->id() << endl;
			}

		}

		void SensorAbstract::discoverNewLmks() {
			map_ptr_t slamMapPtr = robot->slamMap;
			std::size_t size_lmkAHP = LandmarkAnchoredHomogeneousPoint::size();
			if (slamMapPtr->unusedStates(size_lmkAHP)) {
				landmark_ptr_t lmkPtr = newLandmark(slamMapPtr);
				cout << "    added lmk: " << lmkPtr->id() << endl;
				slamMapPtr->addObservations(lmkPtr);
			}
		}

		landmark_ptr_t SensorAbstract::newLandmark(map_ptr_t slamMapPtr) {
			shared_ptr<LandmarkAnchoredHomogeneousPoint> lmkPtr(new LandmarkAnchoredHomogeneousPoint(*slamMapPtr));
			size_t lid = slamMapPtr->landmarkIds.getId();
			lmkPtr->id(lid);
			lmkPtr->name("");
			slamMapPtr->linkToLandmark(lmkPtr);
			lmkPtr->linkToMap(slamMapPtr);
			return lmkPtr;
		}

		void SensorAbstract::linkToObservation(observation_ptr_t _obsPtr) {
			observations[_obsPtr->id()] = _obsPtr;
		}

		void SensorAbstract::linkToRobot(robot_ptr_t _robPtr) {
			robot = _robPtr;
		}


		/*
		 * Get sensor pose in global frame.
		 */
		void SensorAbstract::globalPose(jblas::vec7 & poseG, jblas::mat & PG_r, jblas::mat & PG_s) {
			jblas::vec7 robotPose = robot->pose.x();
			jblas::vec7 sensorPose = this->pose.x();
			quaternion::composeFrames(robotPose, sensorPose, poseG, PG_r, PG_s);
		}


		/*
		 * Get sensor pose in global frame, for sensor poses with LOCAL storage (see class Gaussian for doc on storage options).
		 */
		void SensorAbstract::globalPose(jblas::vec7 & poseG, jblas::mat & PG_r) {
			jblas::vec7 robotPose = robot->pose.x();
			jblas::vec7 sensorPose = this->pose.x();
			poseG = quaternion::composeFrames(robotPose, sensorPose);
			quaternion::composeFrames_by_dglobal(robotPose, sensorPose, PG_r);
		}


		/*
		 * Get sensor pose in global frame.
		 */
		jblas::ind_array SensorAbstract::globalPoseInMap(jblas::vec7 & poseG, jblas::mat & PG_rs) {
			jblas::vec7 robotPose = robot->pose.x();
			jblas::vec7 sensorPose = pose.x();
			if (pose.storage() == pose.LOCAL) {
				// Sensor is not in the map. Jacobians and indices only wrt robot.
				globalPose(poseG, PG_rs);
				jblas::ind_array ia_rs(robot->pose.ia());

				return ia_rs;
			}
			else {
				// Sensor is in the map. Give composed Jacobian and indices.
				jblas::mat PG_r(7, 7), PG_s(7, 7);
				globalPose(poseG, PG_r, PG_s);
				project(PG_rs, range(0, 7), range(0, 7)) = PG_r;
				project(PG_rs, range(0, 7), range(7, 14)) = PG_s;
				jblas::ind_array ia_rs(ia_union(robot->pose.ia(), this->pose.ia()));
				return ia_rs;
			}
		}

	}
}
