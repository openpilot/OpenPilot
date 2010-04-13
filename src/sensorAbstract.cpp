/**
 * sensorAbstract.cpp
 *
 * \date 10/03/2010
 * \author jsola@laas.fr
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

// \todo this needs to go out of here - when we'll have factories
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
			s << ".robot: [ " << sen.robotPtr->id() << " ]";
			if (sen.pose.storage() == sen.pose.REMOTE)
				s << endl << " ia_rs: " << sen.ia_globalPose;
			return s;
		}


		SensorAbstract::SensorAbstract(const robot_ptr_t & _robPtr, const bool inFilter = false) :
			//          #check       ? # sensor in filter                                 : # not in filter
			MapObject(inFilter       ? MapObject(_robPtr->mapPtr, 7)                  : 0),
			robotPtr(_robPtr),
			pose(inFilter            ? Gaussian(state, jmath::ublasExtra::ia_range(0, 7)) : Gaussian(7)),
			ia_globalPose(inFilter   ? ia_union(_robPtr->pose.ia(), pose.ia())            : pose.ia())
		{
			categoryName("SENSOR");
		}


		/*
		 * Acquire raw data.
		 */
		void SensorAbstract::acquireRaw() {
			// \todo Acquire raw data
		}


		/**
		 * Process raw data.
		 */
		void SensorAbstract::processRaw() {
			cout << "processing raw" << endl;
			observeKnownLmks();
			discoverNewLmks();
		}

		void SensorAbstract::observeKnownLmks() {
			for (observations_ptr_set_t::iterator obsIter = observationsPtrSet.begin(); obsIter != observationsPtrSet.end(); obsIter++) {
				cout << "processing raw" << endl;

				observation_ptr_t obsPtr = obsIter->second;
				cout << "exploring obs: " << obsPtr->id() << endl;
			}

		}

		void SensorAbstract::discoverNewLmks() {
			map_ptr_t mapPtr = robotPtr->mapPtr;
			std::size_t size_lmkAHP = LandmarkAnchoredHomogeneousPoint::size();
			if (mapPtr->unusedStates(size_lmkAHP)) {
				landmark_ptr_t lmkPtr = newLandmark(mapPtr);
				cout << "    added lmk: " << lmkPtr->id() << endl;
				mapPtr->addObservations(lmkPtr);
			}
		}

		landmark_ptr_t SensorAbstract::newLandmark(map_ptr_t & mapPtr) {
			shared_ptr<LandmarkAnchoredHomogeneousPoint> lmkPtr(new LandmarkAnchoredHomogeneousPoint(mapPtr));
			size_t lid = mapPtr->landmarkIds.getId();
			lmkPtr->id(lid);
			lmkPtr->name("");
			mapPtr->linkToLandmark(lmkPtr);
			lmkPtr->linkToMap(mapPtr);
			return lmkPtr;
		}

		void SensorAbstract::linkToObservation(const observation_ptr_t & _obsPtr) {
			observationsPtrSet[_obsPtr->id()] = _obsPtr;
		}

		void SensorAbstract::linkToRobot(const robot_ptr_t & _robPtr) {
			robotPtr = _robPtr;
		}

		/*
		 * Get sensor pose in global frame.
		 */
		vec7 SensorAbstract::globalPose() {
			vec7 globPose;
			jblas::vec7 robotPose = robotPtr->pose.x();
			jblas::vec7 sensorPose = pose.x();
			globPose = quaternion::composeFrames(robotPose, sensorPose);
			return globPose;
		}

		/*
		 * Get sensor pose in global frame.
		 */
		void SensorAbstract::globalPose(jblas::vec7 & senGlobalPos, jblas::mat & SG_rs) {
			jblas::vec7 robotPose = robotPtr->pose.x();
			jblas::vec7 sensorPose = pose.x();

			if (pose.storage() == pose.LOCAL) {
				// Sensor is not in the map. Jacobian only wrt robot.
				senGlobalPos = quaternion::composeFrames(robotPose, sensorPose);
				quaternion::composeFrames_by_dglobal(robotPose, sensorPose, SG_rs);
			}
			else {
				// Sensor is in the map. Give composed Jacobian.
				jblas::mat PG_r(7, 7), PG_s(7, 7);
				quaternion::composeFrames(robotPose, sensorPose, senGlobalPos, PG_r, PG_s);
				project(SG_rs, range(0, 7), range(0, 7)) = PG_r;
				project(SG_rs, range(0, 7), range(7, 14)) = PG_s;
			}
		}

	}
}
