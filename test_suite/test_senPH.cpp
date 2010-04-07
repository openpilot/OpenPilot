/**
 * test_senPH.cpp
 *
 *  Created on: 12/03/2010
 *      Author: jsola
 *
 *  \file test_senPH.cpp
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

#include "rtslam/pinhole.hpp"
#include "rtslam/robotConstantVelocity.hpp"
#include "rtslam/sensorPinHole.hpp"

using namespace std;
using namespace jblas;
using namespace jafar::rtslam;
using namespace jafar::jmath;

void test_senPH01(void) {

	map_ptr_t mapPtr(new MapAbstract(100));
	constvel_ptr_t robPtr(new RobotConstantVelocity(mapPtr));
	pinhole_ptr_t senPtr(new SensorPinHole(robPtr, true));

	vec3 v;
	vec2 u;
	mat23 U_v;
	vec4 k;
	vec d(2), c(2);

	k(0) = 100;
	k(1) = 100;
	k(2) = 100;
	k(3) = 100;
	randVector(v);
	randVector(d);
	d *= 0.1;
	c = d;
	senPtr->set_parameters(k, d, c);


	//	u = senPH.projectPoint(k,d,v);
	u = pinhole::projectPoint(senPtr->intrinsic, senPtr->distortion, v);
	cout << "k = " << (MATLAB) k << endl;
	cout << "d = " << (MATLAB) d << endl;
	cout << "v = " << (MATLAB) v << endl;
	cout << "u = " << (MATLAB) u << endl;
	pinhole::projectPoint(senPtr->intrinsic, senPtr->distortion, v, u, U_v);
	cout << "u = " << (MATLAB) u << endl;
	cout << "U_v = " << (MATLAB) U_v << endl;
	cout << "[u_mat,smat,Umat_v] = pinHole(v, k, d)" << endl;
	cout << "u_err = norm(u - u_mat)" << endl;
	cout << "U_v_err = norm(U_v - Umat_v)" << endl;

	cout << "End" << endl;

}

BOOST_AUTO_TEST_CASE( test_senPH )
{
	test_senPH01();
}

