/**
 * test_rtslam.cpp
 *
 *  Created on: 12/03/2010
 *      Author: jsola
 *
 *  \file test_rtslam.cpp
 *
 *  This test file acts as a main() program for the rtslam project.
 *
 *  Achievements (newest to oldest):
 *  - 2010/03/22: jsola: Created 1 map, 2 robots, 3 sensors, 2 landmarks, 6 observations, with parental links and print.
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
		mapPtr->linkToRobot(robPtr);
		robPtr->linkToMap(mapPtr);

		if (mapPtr->unusedStates(size_senPH)) {
			size_t sid = mapPtr->sensorIds.getId();
			pinhole_ptr_t senPtr(new SensorPinHole(robPtr));

			senPtr->id(sid);
			senPtr->name("FLEA");
			robPtr->linkToSensor(senPtr);
			senPtr->linkToRobot(robPtr);
		}
		if (mapPtr->unusedStates(size_senPH)) {
			size_t sid = mapPtr->sensorIds.getId();
			pinhole_ptr_t senPtr(new SensorPinHole(robPtr, true));

			senPtr->id(sid);
			senPtr->name("MARLIN");
			robPtr->linkToSensor(senPtr);
			senPtr->linkToRobot(robPtr);
		}
	}

	if (mapPtr->unusedStates(size_robCV)) {
		size_t rid = mapPtr->robotIds.getId();
		constvel_ptr_t robPtr(new RobotConstantVelocity(mapPtr));

		robPtr->id(rid);
		robPtr->name("AEROPLANE");
		mapPtr->linkToRobot(robPtr);
		robPtr->linkToMap(mapPtr);

		if (mapPtr->unusedStates(size_senPH)) {
			size_t sid = mapPtr->sensorIds.getId();
			pinhole_ptr_t senPtr(new SensorPinHole(robPtr));

			senPtr->id(sid);
			senPtr->name("VIDERE");
			robPtr->linkToSensor(senPtr);
			senPtr->linkToRobot(robPtr);
		}
	}

	// Add 2 lmks
	for (size_t i = 0; i < 2; i++) {
		if (mapPtr->unusedStates(size_lmkAHP)) {
			size_t lid = mapPtr->landmarkIds.getId();
			ahp_ptr_t lmkPtr(new LandmarkAnchoredHomogeneousPoint(mapPtr));

			lmkPtr->id(lid);
			lmkPtr->name("");

			mapPtr->linkToLandmark(lmkPtr);
			lmkPtr->linkToMap(mapPtr);
		}
	}

	// Add observations
	robots_ptr_set_t::iterator robIter;
	sensors_ptr_set_t::iterator senIter;
	landmarks_ptr_set_t::iterator lmkIter;

	for (robIter = mapPtr->robotsPtrSet.begin(); robIter != mapPtr->robotsPtrSet.end(); robIter++) {
		robot_ptr_t robPtr = robIter->second;
		for (senIter = robPtr->sensorsPtrSet.begin(); senIter != robPtr->sensorsPtrSet.end(); senIter++) {
			sensor_ptr_t senPtr = senIter->second;
			for(lmkIter = mapPtr->landmarksPtrSet.begin(); lmkIter != mapPtr->landmarksPtrSet.end(); lmkIter++){
				landmark_ptr_t lmkPtr = lmkIter->second;
				obs_ph_ahp_ptr_t obsPtr(new ObservationPinHoleAnchoredHomogeneousPoint(senPtr, lmkPtr));
				size_t id = 1000*senPtr->id() + lmkPtr->id();
				obsPtr->id() = id;
				//				obsPtr->linkToSensor(senPtr);
				//				obsPtr->linkToLandmark(lmkPtr);
				senPtr->linkToObservation(obsPtr);
				lmkPtr->linkToObservation(obsPtr);
			}
		}
	}


	// Print all data
	cout << "\n% ROBOTS, SENSORS AND OBSERVATIONS \n%==================================" << endl;
	for (robIter = mapPtr->robotsPtrSet.begin(); robIter != mapPtr->robotsPtrSet.end(); robIter++) {
		robot_ptr_t robPtr = robIter->second;
		cout << *robPtr << endl;
		for (senIter = robPtr->sensorsPtrSet.begin(); senIter != robPtr->sensorsPtrSet.end(); senIter++) {
			sensor_ptr_t senPtr = senIter->second;
			cout << *senPtr << endl;
			for(lmkIter = mapPtr->landmarksPtrSet.begin(); lmkIter != mapPtr->landmarksPtrSet.end(); lmkIter++){
				landmark_ptr_t lmkPtr = lmkIter->second;
				size_t id = 1000*senPtr->id() + lmkPtr->id();
				cout << *senPtr->observationsPtrSet[id] << endl;
			}
		}
	}
	cout << "\n% LANDMARKS \n%==========" << endl;
	for (lmkIter = mapPtr->landmarksPtrSet.begin(); lmkIter != mapPtr->landmarksPtrSet.end(); lmkIter++) {
		landmark_ptr_t lmkPtr = lmkIter->second;
		cout << *lmkPtr << endl;
	}

	cout << "\n% POINTERS \n%=============" << endl;
	cout << mapPtr << " <= mapPtr" << endl;
	cout << mapPtr->robotsPtrSet[1]->mapPtr << " <= mapPtr->robots[1]->slamMap" << endl;
	cout << mapPtr->robotsPtrSet[1]->sensorsPtrSet[1]->robotPtr->mapPtr << " <= mapPtr->robots[1]->sensors[1]->robot->slamMap" << endl;
	cout << mapPtr->landmarksPtrSet[1]->mapPtr << " <= mapPtr->landmarks[1]->slamMap" << endl;
	cout << mapPtr->robotsPtrSet[1] << " <= mapPtr->robots[1]" << endl;
	cout << mapPtr->robotsPtrSet[1]->sensorsPtrSet[1] << " <= mapPtr->robots[1]->sensors[1]" << endl;
	cout << mapPtr->robotsPtrSet[1]->sensorsPtrSet[2] << " <= mapPtr->robots[1]->sensors[2]" << endl;
	cout << mapPtr->robotsPtrSet[2] << " <= mapPtr->robots[2]" << endl;
	cout << mapPtr->robotsPtrSet[2]->sensorsPtrSet[3] << " <= mapPtr->robots[2]->sensors[3]" << endl;
	cout << mapPtr->landmarksPtrSet[1] << " <= mapPtr->landmarks[1]" << endl;
	cout << mapPtr->landmarksPtrSet[2] << " <= mapPtr->landmarks[2]" << endl;
	cout << mapPtr->robotsPtrSet[1]->sensorsPtrSet[1]->observationsPtrSet[1001] << " <= mapPtr->robots[1]->sensors[1]->observations[1001]" << endl;
	cout << mapPtr->landmarksPtrSet[1]->observationsPtrSet[1001] << " <= mapPtr->landmarks[2]->observations[1001]" << endl;

	cout << "\nTHAT'S ALL, WHAT'S WRONG?" << endl;
}

BOOST_AUTO_TEST_CASE( test_rtslam )
{
	test_rtslam01();
}

