/**
 * test_senPH.cpp
 *
 * \date 12/03/2010
 * \author jsola@laas.fr
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
#include "jmath/jblas.hpp"
#include "jmath/ublasExtra.hpp"

#include <iostream>

#include "rtslam/pinholeTools.hpp"
#include "rtslam/robotConstantVelocity.hpp"
#include "rtslam/sensorPinHole.hpp"

using namespace std;
using namespace jblas;
using namespace jafar;
using namespace jafar::rtslam;
using namespace jafar::jmath;

void test_senPH01(void) {

  std::cout<<"#"<<__LINE__<<std::endl;
	map_ptr_t mapPtr(new MapAbstract(100));
  std::cout<<"#"<<__LINE__<<std::endl;
	robconstvel_ptr_t robPtr(new RobotConstantVelocity(mapPtr));
	robPtr->linkToParentMap(mapPtr);
	std::cout<<"#"<<__LINE__<<std::endl;
	pinhole_ptr_t senPtr(new SensorPinHole(robPtr, MapObject::FILTERED));
	senPtr->linkToParentRobot(robPtr);
  std::cout<<"#"<<__LINE__<<std::endl;

	vec3 v;
  std::cout<<"#"<<__LINE__<<std::endl;
	vec2 u;
  std::cout<<"#"<<__LINE__<<std::endl;
	mat23 U_v;
	mat32 V_u;
	mat V_s(3,1);
	vec4 k;
	vec d(2), c(2);

  std::cout<<"#"<<__LINE__<<std::endl;
	k(0) = 320;
	k(1) = 240;
	k(2) = 320;
	k(3) = 240;
	randVector(d);
	randVector(d);
	d *= 0.0;
	c = d;
	senPtr->set_parameters(k, d, c);

  std::cout<<"#"<<__LINE__<<std::endl;
	randVector(u);
	u *= k(0);
	double s = 2;


  std::cout<<"#"<<__LINE__<<std::endl;
	cout << "\n% BACK-PROJECTION \n%=====================\n" << endl;
	pinhole::backProjectPoint(k, c, u, s, v, V_u, V_s);
	cout << "k = " << (MATLAB) k << endl;
	cout << "c = " << (MATLAB) c << endl;
	cout << "u = " << (MATLAB) u << endl;
	cout << "s = " << s << endl;
	cout << "v = " << (MATLAB) v << endl;
	cout << "[v_mat,Vmat_u,Vmat_s] = invPinHole(u, s, k, c)" << endl;
	cout << "V_u = " << (MATLAB) V_u << endl;
	cout << "V_s = " << (MATLAB) V_s << endl;
	cout << "v_err = norm(v - v_mat)" << endl;
	cout << "V_u_err = norm(V_u - Vmat_u)" << endl;
	cout << "V_s_err = norm(V_s - Vmat_s)" << endl;


	cout << "\n% PROJECTION \n%=====================\n" << endl;
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

	cout << "\n% BACK plus FORWARD PROJECTIONS \n%=====================\n" << endl;
	cout << "U_v_times_V_u = " << (MATLAB) ublas::prod(U_v, V_u) << endl;
	JFR_CHECK_MAT_EQUAL(ublas::prod(U_v, V_u), identity_mat(2))

}

BOOST_AUTO_TEST_CASE( test_senPH )
{
	test_senPH01();
}

