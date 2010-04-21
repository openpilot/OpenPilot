/**
 * test_rtslam.cpp
 *
 * \date 12/03/2010
 * \author jsola@laas.fr
 *
 *  \file test_rtslam.cpp
 *
 *  This test file acts as a main() program for the rtslam project.
 *
 *  Achievements (newest to oldest):
 *  - 2010/03/22: jsola@laas.fr: Created 1 map, 2 robots, 3 sensors, 2 landmarks, 6 observations, with parental links and print.
 *
 * \ingroup rtslam
 */

// boost unit test includes
#include <boost/test/auto_unit_test.hpp>

// jafar debug include
#include "kernel/jafarDebug.hpp"
#include "kernel/jafarTestMacro.hpp"
#include "jmath/random.hpp"

#include <iostream>
#include <boost/shared_ptr.hpp>

#include "rtslam/rtSlam.hpp"
#include "rtslam/objectAbstract.hpp"
#include "rtslam/robotAbstract.hpp"
#include "rtslam/robotConstantVelocity.hpp"
#include "rtslam/sensorPinHole.hpp"
#include "rtslam/landmarkAnchoredHomogeneousPoint.hpp"
#include "rtslam/observationPinHoleAnchoredHomogeneous.hpp"

//#include <map>

using namespace jblas;
using namespace jafar::jmath;
using namespace jafar::jmath::ublasExtra;
using namespace jafar::rtslam;



void test_rtslam01(void) {

	using namespace boost;

	size_t size_map = 100;

	map_ptr_t mapPtr(new MapAbstract(size_map));

	mapPtr->fillDiag();

	cout << endl;

	cout << "\n% OBJECT ABSTRACT \n%====================" << endl;
	ObjectAbstract O;
	O.id(0);
	cout << O << endl;


	// Robot and sensor sizes;
	std::size_t size_robCV = RobotConstantVelocity::size();
	std::size_t size_senPH = SensorPinHole::size();
	std::size_t size_lmkAHP = LandmarkAnchoredHomogeneousPoint::size();


	// Add 2 robots
	if (mapPtr->unusedStates(size_robCV)) {

		size_t rid = mapPtr->robotIds.getId();
		constvel_ptr_t robPtr(new RobotConstantVelocity(mapPtr));

		robPtr->id(rid);
		robPtr->name("SUBMARINE");
		robPtr->linkToParentMap(mapPtr);

		if (mapPtr->unusedStates(size_senPH)) {
			size_t sid = mapPtr->sensorIds.getId();
			pinhole_ptr_t senPtr(new SensorPinHole(robPtr));

			senPtr->id(sid);
			senPtr->name("FLEA");
			senPtr->linkToParentRobot(robPtr);
		}
		if (mapPtr->unusedStates(size_senPH)) {
			size_t sid = mapPtr->sensorIds.getId();
			pinhole_ptr_t senPtr(new SensorPinHole(robPtr, true));

			senPtr->id(sid);
			senPtr->name("MARLIN");
			senPtr->linkToParentRobot(robPtr);
		}
	}

	if (mapPtr->unusedStates(size_robCV)) {
		size_t rid = mapPtr->robotIds.getId();
		constvel_ptr_t robPtr(new RobotConstantVelocity(mapPtr));

		robPtr->id(rid);
		robPtr->name("AEROPLANE");
		robPtr->linkToParentMap(mapPtr);

		if (mapPtr->unusedStates(size_senPH)) {
			size_t sid = mapPtr->sensorIds.getId();
			pinhole_ptr_t senPtr(new SensorPinHole(robPtr));

			senPtr->id(sid);
			senPtr->name("VIDERE");
			senPtr->linkToParentRobot(robPtr);
		}
	}

	// Add 2 lmks
	for (size_t i = 0; i < 2; i++) {
		if (mapPtr->unusedStates(size_lmkAHP)) {
			size_t lid = mapPtr->landmarkIds.getId();
			ahp_ptr_t lmkPtr(new LandmarkAnchoredHomogeneousPoint(mapPtr));

			lmkPtr->id(lid);
			lmkPtr->name("");

			lmkPtr->linkToParentMap(mapPtr);
		}
	}

	// Add observations
	for (MapAbstract::RobotList::iterator robIter = mapPtr->robotList().begin(); robIter != mapPtr->robotList().end(); robIter++) {
		robot_ptr_t robPtr = *robIter;
		for (RobotAbstract::SensorList::iterator senIter = robPtr->sensorList().begin(); senIter != robPtr->sensorList().end(); senIter++) {
			sensor_ptr_t senPtr = *senIter;
			for(MapAbstract::LandmarkList::iterator lmkIter = mapPtr->landmarkList().begin(); lmkIter != mapPtr->landmarkList().end(); lmkIter++){
				landmark_ptr_t lmkPtr = *lmkIter;
				obs_ph_ahp_ptr_t obsPtr(new ObservationPinHoleAnchoredHomogeneousPoint(senPtr, lmkPtr));
				size_t id = 1000*senPtr->id() + lmkPtr->id();
				obsPtr->id() = id;
				//				obsPtr->linkToSensor(senPtr);
				//				obsPtr->linkToLandmark(lmkPtr);
				obsPtr->linkToParentAHP(lmkPtr);
				obsPtr->linkToParentPinHole(senPtr);

			}
		}
	}


	// Print all data
	cout << "\n% ROBOTS, SENSORS AND OBSERVATIONS \n%==================================" << endl;
	for (MapAbstract::RobotList::iterator robIter = mapPtr->robotList().begin(); robIter != mapPtr->robotList().end(); robIter++) {
		robot_ptr_t robPtr = *robIter;
		cout << *robPtr << endl;
		for (RobotAbstract::SensorList::iterator senIter = robPtr->sensorList().begin(); senIter != robPtr->sensorList().end(); senIter++) {
			sensor_ptr_t senPtr = *senIter;
			cout << *senPtr << endl;
			for(MapAbstract::LandmarkList::iterator lmkIter = mapPtr->landmarkList().begin(); lmkIter != mapPtr->landmarkList().end(); lmkIter++){
				landmark_ptr_t lmkPtr = *lmkIter;
				size_t id = 1000*senPtr->id() + lmkPtr->id();
				//cout << *senPtr->observationsPtrSet[id] << endl;
				cout << "obs id = " << id << endl;
			}
			for(SensorAbstract::ObservationList::iterator obsIter = senPtr->observationList().begin(); obsIter != senPtr->observationList().end(); obsIter++){
				cout << **obsIter << endl;
			}
		}
	}
	cout << "\n% LANDMARKS \n%==========" << endl;
	for(MapAbstract::LandmarkList::iterator lmkIter = mapPtr->landmarkList().begin(); lmkIter != mapPtr->landmarkList().end(); lmkIter++){
		landmark_ptr_t lmkPtr = *lmkIter;
		cout << *lmkPtr << endl;
	}

	cout << "\n% POINTERS \n%=============" << endl;
	cout << mapPtr << " <= mapPtr" << endl;
	cout << mapPtr->robotList()[0]->mapPtr() << " <= mapPtr->robots[0]->slamMap" << endl;
	cout << mapPtr->robotList()[0]->sensorList()[0]->robotPtr()->mapPtr() << " <= mapPtr->robots[0]->sensors[0]->robot->slamMap" << endl;
	cout << mapPtr->landmarkList()[0]->mapPtr() << " <= mapPtr->landmarks[0]->slamMap" << endl;
	cout << mapPtr->robotList()[0] << " <= mapPtr->robots[0]" << endl;
	cout << mapPtr->robotList()[0]->sensorList()[0] << " <= mapPtr->robots[0]->sensors[0]" << endl;
	cout << mapPtr->robotList()[0]->sensorList()[1] << " <= mapPtr->robots[0]->sensors[1]" << endl;
	cout << mapPtr->robotList()[1] << " <= mapPtr->robots[1]" << endl;
	cout << mapPtr->robotList()[1]->sensorList()[2] << " <= mapPtr->robots[1]->sensors[2]" << endl;
	cout << mapPtr->landmarkList()[0] << " <= mapPtr->landmarks[0]" << endl;
	cout << mapPtr->landmarkList()[1] << " <= mapPtr->landmarks[1]" << endl;
	cout << mapPtr->robotList()[0]->sensorList()[0]->observationList()[0] << " <= mapPtr->robots[0]->sensors[0]->observations[0]" << endl;
	cout << mapPtr->landmarkList()[0]->observationList()[0] << " <= mapPtr->landmarks[1]->observations[0]" << endl;

	cout << "\nTHAT'S ALL, WHAT'S WRONG?" << endl;
}

BOOST_AUTO_TEST_CASE( test_rtslam )
{
	test_rtslam01();
}

