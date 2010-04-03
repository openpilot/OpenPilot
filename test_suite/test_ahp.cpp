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
using namespace jafar;
using namespace jafar::rtslam;
using namespace jblas;
using namespace jafar::jmath;
using namespace jafar::jmath::ublasExtra;
using namespace landmarkAHP;

void test_ahp01(void) {

	vec7 ahp;
	vec7 senFrame;
	vec3 v1, v2;
	double r, id;
	mat AHP_s(7, 7);
	mat AHP_v(7, 3);
	mat AHP_r(7, 1);
	mat V_s(3, 7);
	mat V_ahp(3, 7);

	cout << "\n% INIT \n%============" << endl;
	for (size_t i = 0; i < 3; i++)
		v1(i) = i + 2;
	subrange(senFrame, 0, 3) = v1;
	subrange(senFrame, 3, 7) = quaternion::e2q(v1);
	r = 1;
	cout << "F.x = " << (MATLAB) senFrame << endl;
	cout << "v = " << (MATLAB) v1 << endl;
	cout << "r = " << r << endl;
	cout << "F = updateFrame(F);" << endl;

	cout << "\n% FROM B-O FRAME \n%============" << endl;
	cout << "F.x = " << (MATLAB) senFrame << endl;
	cout << "v = " << (MATLAB) v1 << endl;
	cout << "r = " << r << endl;
	ahp = LandmarkAnchoredHomogeneousPoint::fromBearingOnlyFrame(senFrame, v1, r);
	cout << "ahp1 = " << (MATLAB) ahp << endl;
	LandmarkAnchoredHomogeneousPoint::fromBearingOnlyFrame(senFrame, v1, r, ahp, AHP_s, AHP_v, AHP_r);
	cout << "ahp2 = " << (MATLAB) ahp << endl;
	cout << "AHP_s = " << (MATLAB) AHP_s << endl;
	cout << "AHP_v = " << (MATLAB) AHP_v << endl;
	cout << "AHP_r = " << (MATLAB) AHP_r << endl;
	cout << "[ahp_mat, AHP_s_mat, AHP_v_mat, AHP_r_mat] = fromBearingOnlyFrameAHM(F, v, r)" << endl;
	cout << "ahp_err = norm(ahp1 - ahp_mat)" << endl;
	cout << "AHP_s_err = norm(AHP_s - AHP_s_mat)" << endl;
	cout << "AHP_v_err = norm(AHP_v - AHP_v_mat)" << endl;
	cout << "AHP_r_err = norm(AHP_r - AHP_r_mat)" << endl;
	JFR_CHECK_EQUAL(ahp(3), -2.5013342);
	JFR_CHECK_EQUAL(AHP_s(3,5), 3.5702252);
	JFR_CHECK_EQUAL(AHP_s(5,5), 0.676264);
	JFR_CHECK_EQUAL(AHP_v(3,2), -0.649772332351480);
	JFR_CHECK_EQUAL(AHP_v(6,1), 0.557086014531156);
	JFR_CHECK_EQUAL(AHP_r(6,0), 5.385164807134504);

	cout << "\n% TO B-O FRAME \n%============" << endl;
	cout << "F.x = " << (MATLAB) senFrame << endl;
	cout << "ahp = " << (MATLAB) ahp << endl;
	v2 = LandmarkAnchoredHomogeneousPoint::toBearingOnlyFrame(senFrame, ahp);
	JFR_CHECK_VEC_EQUAL(v2, v1);
	LandmarkAnchoredHomogeneousPoint::toBearingOnlyFrame(senFrame, ahp, v2, id);
	JFR_CHECK_VEC_EQUAL(v2, v1);
	JFR_CHECK_EQUAL(id, r);
	LandmarkAnchoredHomogeneousPoint::toBearingOnlyFrame(senFrame, ahp, v2, id, V_s, V_ahp);
	cout << "v2 = " << (MATLAB) v2 << endl;
	cout << "id = " << id << endl;
	cout << "V_s = " << (MATLAB) V_s << endl;
	cout << "V_ahp = " << (MATLAB) V_ahp << endl;
	cout << "[v_mat, V_s_mat, V_ahp_mat] = toBearingOnlyFrameAHM(F, ahp)" << endl;
	cout << "v_err = norm(v2 - v_mat)" << endl;
	cout << "V_s_err = norm(V_s - V_s_mat)" << endl;
	cout << "V_ahp_err = norm(V_ahp - V_ahp_mat)" << endl;
	JFR_CHECK_VEC_EQUAL(v2, v1);
	JFR_CHECK_EQUAL(id, r);
	JFR_CHECK_EQUAL(V_s(0,1), -4.034721000604516);
	JFR_CHECK_EQUAL(V_ahp(2,1), 3.440047966271193);

	cout << "\n% PRODUCT OF FWD AND BCKWD JACOBIANS\n%===================" << endl;
	mat V_v(3, 3), AHP_ahp(7, 7);
	V_v = ublas::prod(V_ahp, AHP_v);
	AHP_ahp = ublas::prod(AHP_v, V_ahp);
	identity_mat I3(3);
	cout << "V_v = " << (MATLAB) V_v << endl;
	//	cout << "AHP_ahp = " << (MATLAB) AHP_ahp << endl;
	//	cout << "V_ahp*AHP_s + V_s = " << (MATLAB) (prod(V_ahp, AHP_s) + V_s) << endl;
	//	cout << "V_ahp*AHP_s = " << (MATLAB) (prod(V_ahp, AHP_s)) << endl;
	//	cout << "V_s = " << (MATLAB) ( V_s) << endl;
	JFR_CHECK_MAT_EQUAL(V_v, I3);

}

BOOST_AUTO_TEST_CASE( test_ahp )
{
	test_ahp01();
}

