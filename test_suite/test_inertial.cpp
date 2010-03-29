/**
 * \file test_inertial.cpp
 *
 *  Created on: 28/03/2010
 *     \author: jsola@laas.fr
 *
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
#include "jmath/matlab.hpp"

#include <iostream>
#include <boost/shared_ptr.hpp>

#include "rtslam/rtSlam.hpp"
#include "rtslam/objectAbstract.hpp"
#include "rtslam/robotAbstract.hpp"
#include "rtslam/robotConstantVelocity.hpp"
#include "rtslam/sensorPinHole.hpp"
#include "rtslam/landmarkAnchoredHomogeneousPoint.hpp"
#include "rtslam/observationPinHoleAnchoredHomogeneous.hpp"
#include "rtslam/kalmanFilter.hpp"
#include "rtslam/robotInertial.hpp"

using namespace jblas;
using namespace jafar::jmath;
using namespace jafar::jmath::ublasExtra;
using namespace jafar::rtslam;
using namespace boost;

void test_inertial01() {

	map_ptr_t mapPtr(new MapAbstract(100));
	shared_ptr<RobotInertial> robPtr(new RobotInertial(*mapPtr));
	mapPtr->linkToRobot(robPtr);
	robPtr->linkToMap(mapPtr);

	mapPtr->fillRndm();

	robPtr->pose.x(quaternion::originFrame());

	vec u(6);
	randVector(u);
	u *= 0.1;
	Gaussian n(scalar_vector<double>(12, 0.1), sym_mat(scalar_diag_mat(12, 1.0)));
	Perturbation pert(n);
	cout << n << endl;

	robPtr->dt = 0.5;
	robPtr->control = u;
	robPtr->set_perturbation(pert);
	cout << robPtr->state.x() << endl;

	cout << "P(:,1) = " << (MATLAB) robPtr->state.x() << endl;
	for (size_t t = 2; t <= 15; t++){
	robPtr->move();
	cout << "P(:," << t << ") = " << (MATLAB) robPtr->state.x() << endl;
	}
}

BOOST_AUTO_TEST_CASE( test_inertial )
{
	test_inertial01();
}
