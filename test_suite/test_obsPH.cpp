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
#include "jmath/matlab.hpp"

#include <iostream>

#include "rtslam/observationPinHolePoint.hpp"

//#include <map>

using namespace jblas;
using namespace jafar::jmath;
using namespace jafar::jmath::ublasExtra;
using namespace jafar::rtslam;

void test_obsPH01(void) {

	ObservationPinHolePoint obsph;

	vec3 v;
	vec2 u;
	mat U_v(2,3);
	vec4 k;
	vec d(2);

	randVector(v);
	k(0) = 100;
	k(1) = 100;
	k(2) = 100;
	k(3) = 100;
	randVector(d);

	u = obsph.project(k,d,v);
	obsph.project(k,d,v,u,U_v);
	cout<<"u = "<< (MATLAB)u << endl;
	cout<<"v = "<< (MATLAB)v << endl;
	cout<<"k = "<< (MATLAB)k << endl;
	cout<<"d = "<< (MATLAB)d << endl;
	cout<<"U_v = "<< (MATLAB)U_v << endl;
	cout<<"[u_mat,smat,Umat_v] = pinHole(v, k, d)"<<endl;
	cout<<"u_err = norm(u - u_mat)"<<endl;
	cout<<"U_v_err = norm(U_v - Umat_v)"<<endl;


//	vec2 v2;
//	randVector(v2);
//
//		obsph.distort(d,v2,u,U_v);
//		cout<<"d = "<< (MATLAB)d << endl;
//		cout<<"v2 = "<< (MATLAB)v2 << endl;
//		cout<<"u = "<< (MATLAB)u << endl;
//		cout<<"U_v = "<< (MATLAB)U_v << endl;
//		cout<<"[u_mat,Umat_v] = distort(v2,d)"<<endl;
//		cout<<"u_err = norm(u - u_mat)"<<endl;
//		cout<<"U_verr = norm(U_v - Umat_v)"<<endl;

//		obsph.project0(v,u,U_v);
//		cout<<"v = "<< (MATLAB)v << endl;
//		cout<<"u = "<< (MATLAB)u << endl;
//		cout<<"U_v = "<< (MATLAB)U_v << endl;
//		cout<<"[u_mat,Umat_v] = project(v)"<<endl;
//		cout<<"u_err = norm(u - u_mat)"<<endl;
//		cout<<"U_v_err = norm(U_v - Umat_v)"<<endl;

cout << "End" <<endl;

}

BOOST_AUTO_TEST_CASE( test_obsPH )
{
	test_obsPH01();
}

