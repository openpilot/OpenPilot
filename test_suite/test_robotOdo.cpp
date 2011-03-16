/**
 * test_rtslam.cpp
 *
 *  Created on: 12/03/2010
 *      Author: agonzale
 *
 *  \file test_rtslam.cpp
 *
 *  This test file acts as a main() program for the rtslam project.
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
//#include "rtslam/robotConstantVelocity.hpp"
#include "rtslam/sensorPinhole.hpp"
#include "rtslam/landmarkAnchoredHomogeneousPoint.hpp"
//#include "rtslam/observationPinHoleAnchoredHomogeneous.hpp"
#include "rtslam/kalmanFilter.hpp"
#include "rtslam/robotOdometry.hpp"

using namespace jblas;
using namespace jafar::jmath;
using namespace jafar::jmath::ublasExtra;
using namespace jafar::rtslam;
using namespace boost;

void test_robotOdo01() {

	cout << "beginning test"<<endl;

	map_ptr_t mapPtr(new MapAbstract(100));
	robodo_ptr_t robPtr(new RobotOdometry(mapPtr));
	robPtr->linkToParentMap(mapPtr);

	mapPtr->fillRndm();

	robPtr->pose.x(quaternion::originFrame());

	vec u(6);
	randVector(u);
	u *= 0.1;
	Perturbation pert(scalar_vector<double>(6, 0.1), sym_mat(scalar_diag_mat(6, 1.0)));

	cout << u <<endl;

	robPtr->dt_or_dx = 0.5;
	robPtr->control = u;
	robPtr->set_perturbation(pert);

	cout << robPtr->state.x() << endl;

	cout << "P(:,1) = " << (MATLAB) robPtr->state.x() << endl;
	for (size_t t = 2; t <= 12; t++){
		robPtr->move();
		cout << "P(:," << t << ") = " << (MATLAB) robPtr->state.x() << endl;
	}
}

BOOST_AUTO_TEST_CASE( test_robotOdo )
{
	test_robotOdo01();
}

