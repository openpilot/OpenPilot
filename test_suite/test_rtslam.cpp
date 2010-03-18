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

void fillMapIdent(MapAbstract & map) {
	size_t size_map = map.max_size;
	for (size_t i = 0; i < size_map; i++) {
		map.x(i) = i;
	}
	map.P() = jblas::identity_mat(size_map);
}

void fillMapRndm(MapAbstract & map) {
	randVector(map.x());
	randMatrix(map.P());
}

void test_rtslam01(void) {

	size_t size_map = 100;

	MapAbstract slamMap(size_map);

	fillMapIdent(slamMap);

	cout << endl;

	cout << "\n% OBJECT ABSTRACT \n%====================" << endl;
	ObjectAbstract O;
	O.id(0);
	cout << O << endl;

	cout << "\n% ROBOTS AND SENSORS \n%====================" << endl;


	// Add 2 robots
	std::size_t sizerobCV = Robot3DConstantVelocity::size();
	std::size_t sizesenPH = SensorPinHole::size();
	if (slamMap.unusedStates(sizerobCV)) {
		Robot3DConstantVelocity * robPtr = new Robot3DConstantVelocity(slamMap);
		robPtr->name("SUBMARINE");

		if (slamMap.unusedStates(sizesenPH)) {
			SensorPinHole * senPtr = new SensorPinHole(*robPtr);
			senPtr->name("FLEA");
		}

		if (slamMap.unusedStates(sizesenPH)) {
			SensorPinHole * senPtr = new SensorPinHole(*robPtr, true);
			senPtr->name("MARLIN");
		}
	}
	if (slamMap.unusedStates(sizerobCV)) {
		Robot3DConstantVelocity * robPtr = new Robot3DConstantVelocity(slamMap);
		robPtr->name("AEROPLANE");

		if (slamMap.unusedStates(sizesenPH)) {
			SensorPinHole * senPtr = new SensorPinHole(*robPtr);
			senPtr->name("VIDERE");
		}
	}

	// Print all robot and sensor ids
	MapAbstract::robots_t::iterator robIter;
	RobotAbstract::sensors_t::iterator senIter;

	slamMap.robots.begin();

	for (robIter = slamMap.robots.begin(); robIter != slamMap.robots.end(); robIter++) {

		RobotAbstract * robPtr = robIter->second;
		cout << *robPtr << endl;

		for (senIter = robPtr->sensors.begin(); senIter != robPtr->sensors.end(); senIter++) {

			SensorAbstract * senPtr = senIter->second;
			cout << *senPtr << endl;

		}
	}

	cout << "\n% LANDMARKS \n%==========" << endl;


	// Add 2 lmks
	for (size_t i = 0; i < 2; i++) {
		std::size_t sizelmkAHP = LandmarkAnchoredHomogeneousPoint::size();
		if (slamMap.unusedStates(sizelmkAHP)) {
			LandmarkAnchoredHomogeneousPoint * lmkPtr = new LandmarkAnchoredHomogeneousPoint(slamMap);
			lmkPtr->name("");
		}
	}

	// Print all landmarks
	MapAbstract::landmarks_t::iterator lmkIter;

	for (lmkIter = slamMap.landmarks.begin(); lmkIter != slamMap.landmarks.end(); lmkIter++) {

		size_t lid = lmkIter->first;
		LandmarkAbstract * lmkPtr = lmkIter->second;

		cout << "Landmark " << lid << ": " << *lmkPtr << endl;

	}
}

BOOST_AUTO_TEST_CASE( test_rtslam )
{
	test_rtslam01();
}

