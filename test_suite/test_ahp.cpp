/**
 * \file test_ahp.cpp
 *
 *  Created on: 01/04/2010
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

#include <iostream>
#include "jmath/matlab.hpp"
#include "jmath/random.hpp"
#include "rtslam/landmarkAnchoredHomogeneousPoint.hpp"

using namespace std;
using namespace jafar::rtslam;
using namespace jblas;
using namespace jafar::jmath;
using namespace jafar::jmath::ublasExtra;
using namespace landmarkAHP;

void test_ahp01(void) {

	vec7 ahp;
	vec7 s;
	vec3 v;
	double r;
	mat AHP_s(7, 7);
	mat AHP_v(7, 3);
	mat AHP_r(7, 1);
	mat V_s(3, 7);
	mat V_ahp(3, 7);

	cout << "\n% INIT \n%============" << endl;
	for (size_t i = 0; i < 7; i++)
		ahp(i) = i + 5;
	for (size_t i = 0; i < 3; i++)
		v(i) = i + 2;
	subrange(s, 0, 3) = v;
	subrange(s, 3, 7) = quaternion::e2q(v);
	r = 1;
	cout << "s = " << (MATLAB) s << endl;
	cout << "v = " << (MATLAB) v << endl;
	cout << "r = " << r << endl;
	cout << "F.x = s; F = updateFrame(F);" << endl;

	cout << "\n% FROM B-O FRAME \n%============" << endl;
	cout << "F.x = " << (MATLAB) s << endl;
	cout << "v = " << (MATLAB) v << endl;
	cout << "r = " << r << endl;
	ahp = fromBearingOnlyFrame(s, v, r);
	cout << "ahp1 = " << (MATLAB) ahp << endl;
	fromBearingOnlyFrame(s, v, r, ahp, AHP_s, AHP_v, AHP_r);
	JFR_CHECK_EQUAL(ahp(3), -2.5013342);
	JFR_CHECK_EQUAL(AHP_s(3,5), 3.5702252);
	JFR_CHECK_EQUAL(AHP_s(5,5), 0.676264);
	JFR_CHECK_EQUAL(AHP_v(3,2), -0.649772332351480);
	JFR_CHECK_EQUAL(AHP_v(6,1), 0.557086014531156);
	JFR_CHECK_EQUAL(AHP_r(6,0), 5.385164807134504);
	cout << "ahp2 = " << (MATLAB) ahp << endl;
	cout << "AHP_s = " << (MATLAB) AHP_s << endl;
	cout << "AHP_v = " << (MATLAB) AHP_v << endl;
	cout << "AHP_r = " << (MATLAB) AHP_r << endl;
	cout << "[ahp_mat, AHP_s_mat, AHP_v_mat, AHP_r_mat] = fromBearingOnlyFrameAHM(F, v, r)" << endl;
	cout << "ahp_err = norm(ahp1 - ahp_mat)" << endl;
	cout << "AHP_s_err = norm(AHP_s - AHP_s_mat)" << endl;
	cout << "AHP_v_err = norm(AHP_v - AHP_v_mat)" << endl;
	cout << "AHP_r_err = norm(AHP_r - AHP_r_mat)" << endl;

	cout << "\n% TO B-O FRAME \n%============" << endl;
	cout << "F.x = " << (MATLAB) s << endl;
	cout << "ahp = " << (MATLAB) ahp << endl;
	v = toBearingOnlyFrame(s, ahp);
	cout << "v1 = " << (MATLAB) v << endl;
	toBearingOnlyFrame(s, ahp, v, V_s, V_ahp);
	JFR_CHECK_EQUAL(v(2), 4.0);
	JFR_CHECK_EQUAL(V_s(0,1), -4.034721000604516);
	JFR_CHECK_EQUAL(V_ahp(2,1), 3.440047966271193);
	cout << "v2 = " << (MATLAB) v << endl;
	cout << "V_s = " << (MATLAB) V_s << endl;
	cout << "V_ahp = " << (MATLAB) V_ahp << endl;
	cout << "[v_mat, V_s_mat, V_ahp_mat] = toBearingOnlyFrameAHM(F, ahp)" << endl;
	cout << "v_err = norm(v1 - v_mat)" << endl;
	cout << "V_s_err = norm(V_s - V_s_mat)" << endl;
	cout << "V_ahp_err = norm(V_ahp - V_ahp_mat)" << endl;

	cout << "\n% PRODUCT OF FWD AND BCKWD JACOBIANS\n%===================" << endl;
	mat M3(3, 3), M7(7, 7);
	M3 = ublas::prod(V_ahp, AHP_v);
	JFR_CHECK_EQUAL(M3(0,1), 0.0);
	JFR_CHECK_EQUAL(M3(1,1), 1.0); // M3 = V_ahp * AHP_v must be a identity matrix
	M7 = ublas::prod(AHP_v, V_ahp);
	cout << "V_v = " << (MATLAB) M3 << endl;
	cout << "AHP_ahp = " << (MATLAB) M7 << endl;

}

BOOST_AUTO_TEST_CASE( test_ahp )
{
	test_ahp01();
}

