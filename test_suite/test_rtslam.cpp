/**
 * test_rtslam.cpp
 *
 *  Created on: 12/03/2010
 *      Author: jsola
 *
 *  \file test_rtslam.cpp
 *
 *  ## Add a description here ##
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

#include "rtslam/objectAbstract.hpp"
#include "rtslam/robotAbstract.hpp"
#include "rtslam/robotConstantVelocity.hpp"
#include "rtslam/sensorPinHole.hpp"
#include "rtslam/landmarkAnchoredHomogeneousPoint.hpp"

//#include <map>

using namespace jblas;
using namespace jafar::jmath;
using namespace jafar::jmath::ublasExtra;
using namespace jafar::rtslam;

void fillMapSeq(MapAbstract & map) {
	size_t size_map = map.max_size;
	for (size_t i = 0; i < size_map; i++) {
		map.x(i) = i;
		for (size_t j = 0; j < size_map; j++)
			map.P(i, j) = i + 100 * j;
	}
}

void fillMapDiag(MapAbstract & map) {
	size_t size_map = map.max_size;
	for (size_t i = 0; i < size_map; i++) {
		map.x(i) = i;
		map.P(i, i) = i;
	}
}

void fillMapRndm(MapAbstract & map) {
	randVector(map.x());
	randMatrix(map.P());
}


void test_rtslam01(void) {

	using namespace boost;

	size_t size_map = 100;

	shared_ptr<MapAbstract> slamMapPtr(new MapAbstract(size_map));

	fillMapDiag(*slamMapPtr);

	cout << endl;

	cout << "\n% OBJECT ABSTRACT \n%====================" << endl;
	ObjectAbstract O;
	O.id(0);
	cout << O << endl;


	// Robot and sensor sizes;
	std::size_t size_robCV = Robot3DConstantVelocity::size();
	std::size_t size_senPH = SensorPinHole::size();
	std::size_t size_lmkAHP = LandmarkAnchoredHomogeneousPoint::size();


	// Add 2 robots
	if (slamMapPtr->unusedStates(size_robCV)) {

		size_t rid = slamMapPtr->robotIds.getId();
		shared_ptr<Robot3DConstantVelocity> robPtr(new Robot3DConstantVelocity(*slamMapPtr));

		slamMapPtr->robots[rid] = robPtr; // robot is now in map
		robPtr->slamMap = slamMapPtr; // robot points now to map too
		robPtr->id(rid);
		robPtr->name("SUBMARINE");

		if (slamMapPtr->unusedStates(size_senPH)) {
			size_t sid = slamMapPtr->sensorIds.getId();
			shared_ptr<SensorPinHole> senPtr(new SensorPinHole(*robPtr));

			robPtr->sensors[sid] = senPtr;
			senPtr->robot = robPtr;
			senPtr->id(sid);
			senPtr->name("FLEA");
		}
		if (slamMapPtr->unusedStates(size_senPH)) {
			size_t sid = slamMapPtr->sensorIds.getId();
			shared_ptr<SensorPinHole> senPtr(new SensorPinHole(*robPtr, true));

			robPtr->sensors[sid] = senPtr;
			senPtr->robot = robPtr;
			senPtr->id(sid);
			senPtr->name("MARLIN");
		}
	}

	if (slamMapPtr->unusedStates(size_robCV)) {
		size_t rid = slamMapPtr->robotIds.getId();
		shared_ptr<Robot3DConstantVelocity> robPtr(new Robot3DConstantVelocity(*slamMapPtr));

		slamMapPtr->robots[rid] = robPtr;
		robPtr->slamMap = slamMapPtr;
		robPtr->id(rid);
		robPtr->name("AEROPLANE");

		if (slamMapPtr->unusedStates(size_senPH)) {
			size_t sid = slamMapPtr->sensorIds.getId();
			shared_ptr<SensorPinHole> senPtr(new SensorPinHole(*robPtr));

			robPtr->sensors[sid] = senPtr;
			senPtr->robot = robPtr;
			senPtr->id(sid);
			senPtr->name("VIDERE");
		}
	}

	// Add 2 lmks
	for (size_t i = 0; i < 2; i++) {
		if (slamMapPtr->unusedStates(size_lmkAHP)) {
			size_t lid = slamMapPtr->landmarkIds.getId();
			shared_ptr<LandmarkAnchoredHomogeneousPoint> lmkPtr(new LandmarkAnchoredHomogeneousPoint(*slamMapPtr));

			slamMapPtr->landmarks[lid] = lmkPtr;
			lmkPtr->slamMap = slamMapPtr;
			lmkPtr->id(lid);
			lmkPtr->name("");
		}
	}

	// Print all data
	MapAbstract::robotsSet_t::iterator robIter;
	RobotAbstract::sensorsSet_t::iterator senIter;
	MapAbstract::landmarksSet_t::iterator lmkIter;

	cout << "\n% ROBOTS AND SENSORS \n%====================" << endl;
	for (robIter = slamMapPtr->robots.begin(); robIter != slamMapPtr->robots.end(); robIter++) {
		MapAbstract::robot_t robPtr = robIter->second;
		cout << *robPtr << endl;
		for (senIter = robPtr->sensors.begin(); senIter != robPtr->sensors.end(); senIter++) {
			RobotAbstract::sensor_t senPtr = senIter->second;
			cout << *senPtr << endl;
		}
	}
	cout << "\n% LANDMARKS \n%==========" << endl;
	for (lmkIter = slamMapPtr->landmarks.begin(); lmkIter != slamMapPtr->landmarks.end(); lmkIter++) {
		MapAbstract::landmark_t lmkPtr = lmkIter->second;
		cout << *lmkPtr << endl;
	}

	cout << "\n% POINTERS \n%=============" << endl;
	cout << slamMapPtr << " <= slamMapPtr" << endl;
	cout << slamMapPtr->robots[1]->slamMap << " <= slamMapPtr->robots[1]->slamMap" << endl;
	cout << slamMapPtr->robots[1]->sensors[1]->robot->slamMap << " <= slamMapPtr->robots[1]->sensors[1]->robot->slamMap"
	    << endl;
	cout << slamMapPtr->landmarks[1]->slamMap << " <= slamMapPtr->landmarks[1]->slamMap" << endl;
	cout << slamMapPtr->robots[1] << " <= slamMapPtr->robots[1]" << endl;
	cout << slamMapPtr->robots[1]->sensors[1] << " <= slamMapPtr->robots[1]->sensors[1]" << endl;
	cout << slamMapPtr->robots[1]->sensors[2] << " <= slamMapPtr->robots[1]->sensors[2]" << endl;
	cout << slamMapPtr->robots[2] << " <= slamMapPtr->robots[2]" << endl;
	cout << slamMapPtr->robots[2]->sensors[3] << " <= slamMapPtr->robots[2]->sensors[3]" << endl;
	cout << slamMapPtr->landmarks[1] << " <= slamMapPtr->landmarks[1]" << endl;
	cout << slamMapPtr->landmarks[2] << " <= slamMapPtr->landmarks[2]" << endl;

	cout << "\nTHAT'S ALL, WHAT'S WRONG?" << endl;
}

BOOST_AUTO_TEST_CASE( test_rtslam )
{
	test_rtslam01();
}

