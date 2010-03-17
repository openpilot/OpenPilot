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

using namespace jafar::rtslam;
using namespace jblas;
using namespace jafar::jmath;
using namespace jafar::jmath::ublasExtra;

void fillMapSeq(MapAbstract & map) {
	size_t size_map = map.max_size;
	for (size_t i = 0; i < size_map; i++) {
		map.filter.x(i) = i;
		for (size_t j = 0; j < size_map; j++)
			map.filter.P(i, j) = i + 100 * j;
	}
}

void fillMapIdent(MapAbstract & map) {
	size_t size_map = map.max_size;
	for (size_t i = 0; i < size_map; i++) {
		map.filter.x(i) = i;
	}
	map.filter.P = jblas::identity_mat(size_map);
}

void fillMapRndm(MapAbstract & map) {
	randVector(map.filter.x);
	randMatrix(map.filter.P);
}

void test_rtslam01(void) {

	size_t size_map = 100;

	MapAbstract slamMap(size_map);

	fillMapIdent(slamMap);

	cout << endl << "\n ROBOTS AND SENSORS \n%====================" << endl;


	// Add 2 robots
	std::size_t sizerob = Robot3DConstantVelocity::size();
	std::size_t sizesen = SensorPinHole::size();
	if (slamMap.unusedStates(sizerob)) {
		Robot3DConstantVelocity * robPtr1 = new Robot3DConstantVelocity(slamMap);
		robPtr1->name("SUBMARINE");

		if (slamMap.unusedStates(sizesen)) {
			SensorPinHole * senPtr1 = new SensorPinHole(*robPtr1);
			senPtr1->name("FLEA");
		}

		if (slamMap.unusedStates(sizesen)) {
			SensorPinHole * senPtr2 = new SensorPinHole(*robPtr1, true);
			senPtr2->name("MARLIN");
		}
	}
	if (slamMap.unusedStates(sizerob)) {
		Robot3DConstantVelocity * robPtr2 = new Robot3DConstantVelocity(slamMap);
		robPtr2->name("AEROPLANE");

		if (slamMap.unusedStates(sizesen)) {
			SensorPinHole * senPtr3 = new SensorPinHole(*robPtr2);
			senPtr3->name("VIDERE");
		}
	}

	// Print all robot and sensor ids
	MapAbstract::robots_t::iterator robIter;
	RobotAbstract::sensors_t::iterator senIter;

	slamMap.robots.begin();

	for (robIter = slamMap.robots.begin(); robIter != slamMap.robots.end(); robIter++) {

		size_t rid = robIter->first;
		RobotAbstract * robPtr = robIter->second;

		cout << *robPtr << endl;

		for (senIter = robPtr->sensors.begin(); senIter != robPtr->sensors.end(); senIter++) {
			size_t sid = senIter->first;
			SensorAbstract * senPtr = senIter->second;
			cout << *senPtr << endl;

		}
	}

	cout << "\n LANDMARKS \n%==========" << endl;


	// Add 2 lmks
	for (size_t i = 0; i < 2; i++) {
		std::size_t sizelmk = Landmark3DAnchoredHomogeneousPoint::size();
		if (slamMap.unusedStates(sizelmk)) {
			Landmark3DAnchoredHomogeneousPoint * lmkPtr1 = new Landmark3DAnchoredHomogeneousPoint(slamMap);
			lmkPtr1->name("");
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

