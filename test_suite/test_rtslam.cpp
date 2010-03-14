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

#include <iostream>
#include "jmath/matlab.hpp"
#include "jmath/random.hpp"

#include "jmath/jblas.hpp"
#include "jmath/indirectArray.hpp"

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
	randVector(map.filter.x);
	randMatrix(map.filter.P);

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

	cout << "Robot " << robot.name() << " has sensor " << robot.sensorsList.front()->name() << " of type "
	    << robot.sensorsList.front()->type() << endl;
	cout << "Sensor " << camera2.name() << " is on robot " << camera2.robot->name() << endl;

	cout << "\n% LANDMARK CREATION AND PRINT \n%===========" << endl;
	jblas::ind_array ial = ia_pushfront(map.used_states, Landmark3DAnchoredHomogeneousPoint::size());
	Landmark3DAnchoredHomogeneousPoint ahp1(map, ial);
	ahp1.id(IdFac.getId());
	cout << "lmk1: " << ahp1 << endl;


}

BOOST_AUTO_TEST_CASE( test_rtslam )
{
	test_rtslam01();
}

