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

#include <iostream>
#include "jmath/matlab.hpp"
#include "jmath/random.hpp"

#include "jmath/jblas.hpp"
#include "jmath/indirectArray.hpp"
#include "jmath/misc.hpp"

#include "rtslam/quatTools.hpp"

#include "rtslam/robotConstantVelocity.hpp"
#include "rtslam/sensorPinHole.hpp"
#include "rtslam/landmarkAnchoredHomogeneousPoint.hpp"
#include "kernel/IdFactory.hpp"

using namespace jafar::rtslam;
using namespace jblas;
using namespace jafar::jmath;
using namespace jafar::jmath::ublasExtra;

void test_rtslam01(void) {

	size_t size_map = 300;

	MapAbstract map(size_map);
	//	randVector(map.filter.x);
	//	randMatrix(map.filter.P);

	cout << "\n% ROBOT CREATION AND PRINT \n%===========" << endl;
	jblas::ind_array iar = ia_pushfront(map.used_states, Robot3DConstantVelocity::size());
	Robot3DConstantVelocity robot(map, iar);
	robot.name("Dala");
	//	robot.state.clear();
	jafar::kernel::IdFactory IdFac;
	robot.id(IdFac.getId());
	cout << "robot: " << robot << endl;

	cout << "\n% CAMERA CREATION AND PRINT \n%===========" << endl;
	Gaussian sensorPose(7);
	sensorPose.hasNullCov(false);
	jblas::vec4 k;
	jblas::vec dist(3);
	jblas::vec corr(3);
	size_t hsize = 640;
	size_t vsize = 480;

	SensorPinHole camera1(sensorPose);
	camera1.id(IdFac.getId());
	//	camera1.name("");
	camera1.set_parameters(k, dist, corr, hsize, vsize);
	cout << "camera1: " << camera1 << endl;

	jblas::ind_array ias = ia_pushfront(map.used_states, SensorPinHole::size());
	SensorPinHole camera2(map, ias);
	camera2.id(IdFac.getId());
	camera2.name("Flea2");
	camera2.set_parameters(k, dist, corr, hsize, vsize);
	cout << "camera2: " << camera2 << endl;

	SensorPinHole camera3(sensorPose.x());
	camera3.id(IdFac.getId());
	camera3.name("Marlin");
	camera3.set_parameters(k, dist, corr, hsize, vsize);
	cout << "camera3: " << camera3 << endl;

	cout << "\n% PARENTAL ACCESS \n%===========" << endl;

	camera2.installToRobot(robot);


	//	cout << "Robot " << robot.name() << " has sensor " << robot.sensorsList.front()->name() << " of type "
	//	    << robot.sensorsList.front()->type() << endl;
	cout << "Sensor " << camera2.name() << " is on robot " << camera2.robot->name() << endl;

	cout << "\n% LANDMARK CREATION AND PRINT \n%===========" << endl;
	jblas::ind_array ial = ia_pushfront(map.used_states, Landmark3DAnchoredHomogeneousPoint::size());
	Landmark3DAnchoredHomogeneousPoint ahp1(map, ial);
	ahp1.id(IdFac.getId());
	cout << "lmk1: " << ahp1 << endl;
}

void test_rtslam02(void) {

	MapAbstract map(300);


	// Size of robot state
	std::size_t N = Robot3DConstantVelocity::size();


	// Add one robot
	jblas::ind_array iar2 = map.reserveStates(N);
	if (iar2.size() == N) {
		Robot3DConstantVelocity * robotPtr = new Robot3DConstantVelocity(map, iar2);
		robotPtr->name("HRP2");
		cout << *map.robots[robotPtr->id()] << endl << endl;
	}

	// Add 2 more robots
	for (size_t i = 0; i < 2; i++) {
		jblas::ind_array ia = map.reserveStates(N);
		if (ia.size() == N) {
			Robot3DConstantVelocity * robPtr = new Robot3DConstantVelocity(map, ia);
			//			ostream name;
			std::stringstream name;
			name << "PLANE " << i + 1;
			robPtr->name(name.str());
		}
	}

	// Print all robots
	// TODO: find the way of using iterators !
	for (size_t i = 1; i <= map.robots.size(); i++) {
		cout << "\nAbout to print robot # " << i << endl;
		cout << *map.robots[i] << endl << endl;
	}

	// Sensors
	Gaussian sensorPose(7);
	sensorPose.hasNullCov(false);
	jblas::vec4 k;
	jblas::vec dist(3);
	jblas::vec corr(3);
	size_t hsize = 640;
	size_t vsize = 480;


	// this is local
	SensorPinHole camera1(sensorPose);
	camera1.id(map.sensorIds.getId());
	//	camera1.name("");
	camera1.set_parameters(k, dist, corr, hsize, vsize);
	camera1.name("Marlin");
	camera1.installToRobot(*map.robots[1]);
	cout << "camera1: " << camera1 << endl;

	jblas::ind_array ias = map.reserveStates(SensorPinHole::size());
	if (ias.size() == SensorPinHole::size()) {
		SensorPinHole camera2(map, ias);
		camera2.id(map.sensorIds.getId());
		camera2.name("Flea2");
		camera2.set_parameters(k, dist, corr, hsize, vsize);
		camera2.installToRobot(*map.robots[2]);
		cout << "camera2: " << camera2 << endl;
	}

}

void test_rtslam03(void) {

	size_t size_map = 300;

	MapAbstract map(size_map);
	cout << "Current map size: " << map.current_size << endl;

	cout << "\n MAP OBJECT \n%==========" << endl;
	MapObject O(map, 3);
	O.id(1);
	cout << O << endl;
	cout << "Current map size: " << map.current_size << endl;

	cout << "\n ROBOTS \n%==========" << endl;
	// Add 2 robots
	std::size_t N = Robot3DConstantVelocity::size();
	for (size_t i = 0; i < 2; i++) {
		//		jblas::ind_array ia = map.reserveStates(N);
		if (map.unusedStates(N)) {
			Robot3DConstantVelocity * robPtr = new Robot3DConstantVelocity(map);
			std::stringstream name;
			robPtr->name("PLANE " + jafar::jmath::intToStr(i + 1));
		}
		cout << "Current map size: " << map.current_size << endl;
	}


	// Print all robots
	// TODO: find the way of using iterators !
	for (size_t i = 1; i <= map.robots.size(); i++) {
		cout << "\nAbout to print robot # " << i << endl;
		cout << *map.robots[i] << endl << endl;
	}

}

BOOST_AUTO_TEST_CASE( test_rtslam )
{
	//	test_rtslam01();
	//	test_rtslam02();
	test_rtslam03();
}

