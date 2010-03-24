/**
 * \file test_main.cpp
 *
 *  Created on: 24/03/2010
 *     \author: jsola@laas.fr
 *
 *
 *  This test acts as a main() function for rtslam.
 *
 *  It is supposed to implement a simple but full SLAM program.
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
using namespace boost;

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

/**
 * Print all MAP data.
 *
 * It traverses the map tree in the following way:
 * - robots
 *   - sensors in robot
 * - landmarks
 *   - observations of landmark from each sensor
 *
 * \param slamMapPtr pointer to the SLAM map.
 */
void printSlam(map_ptr_t slamMapPtr) {
	// Add observations
	robots_ptr_set_t::iterator robIter;
	sensors_ptr_set_t::iterator senIter;
	landmarks_ptr_set_t::iterator lmkIter;
	observations_ptr_set_t::iterator obsIter;

	cout << "\n% ROBOTS AND SENSORS \n%=========================" << endl;
	for (robIter = slamMapPtr->robots.begin(); robIter != slamMapPtr->robots.end(); robIter++) {
		robot_ptr_t robPtr = robIter->second;
		cout << *robPtr << endl;
		for (senIter = robPtr->sensors.begin(); senIter != robPtr->sensors.end(); senIter++) {
			sensor_ptr_t senPtr = senIter->second;
			cout << *senPtr << endl;
		}
	}
	cout << "\n% LANDMARKS AND OBSERVATIONS \n%==========================" << endl;
	for (lmkIter = slamMapPtr->landmarks.begin(); lmkIter != slamMapPtr->landmarks.end(); lmkIter++) {
		landmark_ptr_t lmkPtr = lmkIter->second;
		cout << *lmkPtr << endl;
		for (obsIter = lmkPtr->observations.begin(); obsIter != lmkPtr->observations.end(); obsIter++) {
			observation_ptr_t obsPtr = obsIter->second;
			cout << *obsPtr << endl;
		}
	}
}

/**
 * Add a new robot to map
 * \param slamMapPtr a pointer to the SLAM map.
 * \param name the name of the robot.
 */
robot_ptr_t newRobot(map_ptr_t slamMapPtr, string name) {

	size_t rid = slamMapPtr->robotIds.getId();
	shared_ptr<Robot3DConstantVelocity> robPtr(new Robot3DConstantVelocity(*slamMapPtr));

	robPtr->id(rid);
	robPtr->name(name);
	slamMapPtr->linkToRobot(robPtr);
	robPtr->linkToMap(slamMapPtr);
	return robPtr;
}

/**
 * Add a new sensor.
 * \param robPtr a pointer to the robot owning the sensor.
 * \param inInMap flag to estimate robot pose within the SLAM EKF.
 */
sensor_ptr_t newSensor(robot_ptr_t robPtr, string name, bool isInMap = false) {
	map_ptr_t slamMapPtr = robPtr->slamMap;

	size_t sid = slamMapPtr->sensorIds.getId();
	shared_ptr<SensorPinHole> senPtr(new SensorPinHole(*robPtr, isInMap));

	senPtr->id(sid);
	senPtr->name(name);
	robPtr->linkToSensor(senPtr);
	senPtr->linkToRobot(robPtr);
	return senPtr;
}

/**
 * Add new observation.
 * \param senPtr pointer to the sensor observing the landmark.
 * \param lmkPtr poitner to the observed landmark
 */
observation_ptr_t newObservation(sensor_ptr_t senPtr, landmark_ptr_t lmkPtr) {
	shared_ptr<ObservationPinHoleAnchoredHomogeneousPoint> obsPtr(new ObservationPinHoleAnchoredHomogeneousPoint());
	size_t id = 10000 * senPtr->id() + lmkPtr->id();
	obsPtr->id() = id;
	obsPtr->linkToSensor(senPtr);
	obsPtr->linkToLandmark(lmkPtr);
	senPtr->linkToObservation(obsPtr);
	lmkPtr->linkToObservation(obsPtr);

	return obsPtr;
}

/**
 * Add a new landmark
 * \param senPtr the sensor that discovered it.
 */
landmark_ptr_t newLandmark(sensor_ptr_t senPtr) {

	map_ptr_t slamMapPtr = senPtr->robot->slamMap;

	size_t lid = slamMapPtr->landmarkIds.getId();
	shared_ptr<LandmarkAnchoredHomogeneousPoint> lmkPtr(new LandmarkAnchoredHomogeneousPoint(*slamMapPtr));

	lmkPtr->id(lid);
	lmkPtr->name("");

	slamMapPtr->linkToLandmark(lmkPtr);
	lmkPtr->linkToMap(slamMapPtr);

	// Add observations for each sensor
	for (robots_ptr_set_t::iterator robIter = slamMapPtr->robots.begin(); robIter != slamMapPtr->robots.end(); robIter++) {
		robot_ptr_t robPtr = robIter->second;
		for (sensors_ptr_set_t::iterator senIter = robPtr->sensors.begin(); senIter != robPtr->sensors.end(); senIter++) {
			sensor_ptr_t senPtr = senIter->second;
			observation_ptr_t obsPtr = newObservation(senPtr, lmkPtr);
		}
	}

	return lmkPtr;
}

map_ptr_t initSlam(size_t size_map) {

	using namespace boost;

	shared_ptr<MapAbstract> slamMapPtr(new MapAbstract(size_map));



	// Map object sizes;
	std::size_t size_robCV = Robot3DConstantVelocity::size();
	std::size_t size_senPH = SensorPinHole::size();
	std::size_t size_lmkAHP = LandmarkAnchoredHomogeneousPoint::size();


	// Add 2 robots, 3 sensors
	if (slamMapPtr->unusedStates(size_robCV)) {
		robot_ptr_t robPtr = newRobot(slamMapPtr, "SUBMARINE");

		if (slamMapPtr->unusedStates(size_senPH)) {
			sensor_ptr_t senPtr = newSensor(robPtr, "FLEA", false);
		}
		if (slamMapPtr->unusedStates(size_senPH)) {
			sensor_ptr_t senPtr = newSensor(robPtr, "MARLIN", true);
		}
	}

//	if (slamMapPtr->unusedStates(size_robCV)) {
//		robot_ptr_t robPtr = newRobot(slamMapPtr, "AEROPLANE");
//		if (slamMapPtr->unusedStates(size_senPH)) {
//			sensor_ptr_t senPtr = newSensor(robPtr, "FLEA2", false);
//		}
//	}

	// Add 3 lmks
	if (slamMapPtr->unusedStates(size_lmkAHP)) {
		sensor_ptr_t senPtr = slamMapPtr->robots[1]->sensors[1]; // discovered by sensor 1
		landmark_ptr_t lmkPtr = newLandmark(senPtr);
	}
//	if (slamMapPtr->unusedStates(size_lmkAHP)) {
//		sensor_ptr_t senPtr = slamMapPtr->robots[1]->sensors[2]; // discovered by sensor 2
//		landmark_ptr_t lmkPtr = newLandmark(senPtr);
//	}
//	if (slamMapPtr->unusedStates(size_lmkAHP)) {
//		sensor_ptr_t senPtr = slamMapPtr->robots[2]->sensors[3]; // discovered by sensor 3
//		landmark_ptr_t lmkPtr = newLandmark(senPtr);
//	}

	return slamMapPtr;

}

void test_main01(){

	cout << "\n\n\n% ######    WELCOME TO RTSLAM    ######" << endl;

	using namespace boost;

	map_ptr_t slamMapPtr = initSlam(300);

	printSlam(slamMapPtr);

	fillMapDiag(*slamMapPtr);

	printSlam(slamMapPtr);

	cout << "\nTHAT'S ALL, WHAT'S WRONG?" << endl;
}

BOOST_AUTO_TEST_CASE( test_rtslam )
{
	test_main01();
}

