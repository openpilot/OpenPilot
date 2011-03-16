
/**
 * \file test_obspa.cpp
 *
 * \date 01/04/2010
 * \author jsola@laas.fr
 *
 *
 *  Test for class ObservationPinHoleAnchoredHomogeneousPoint.
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
#include "rtslam/mapAbstract.hpp"
#include "rtslam/robotOdometry.hpp"
#include "rtslam/sensorPinhole.hpp"
#include "rtslam/landmarkEuclideanPoint.hpp"
#include "rtslam/landmarkAnchoredHomogeneousPoint.hpp"
#include "rtslam/observationPinHoleAnchoredHomogeneous.hpp"

using namespace std;
using namespace jafar;
using namespace jafar::rtslam;
using namespace jblas;
using namespace jafar::jmath;
using namespace jafar::jmath::ublasExtra;

void test_obsap01(void) {

	vec ahp(7);
	vec7 senFrame;
	vec2 imsz;
	vec4 k;
	vec d(2), c(2);
	vec u(2), u2(2);
	vec r(1), r2(1);
	mat AHP_s(7, 7);
	mat AHP_u(7, 3);
	mat AHP_r(7, 1);
	mat U_s(2, 7);
	mat U_ahp(2, 7);

	// ENTER YOUR DATA HERE
	double _pos[3] = {1,2,3};
	double _e[3] = {.1,.2,.3};
	vec3 e;
	double _imsz[2] = {640,480};
	double _k[4] = {320,240,320,320};
	double _d[2] = {0,0};
	double _c[2] = {0,0};
	double _u[2] = {100,100};
	double _r = 1;

	// This is automatic, do NOT enter data here
	r(0) = _r;
	for (size_t i=0; i<2; i++){
		u(i) = _u[i];
		d(i) = _d[i];
		c(i) = _c[i];
		imsz(i) = _imsz[i];
	}
	for (size_t i = 0; i < 3; i++){
		senFrame(i) = _pos[i];
		e(i) = _e[i];
	}
	subrange(senFrame, 3, 7) = quaternion::e2q(e);
	for (size_t i=0;i<4;i++){
		k(i) = _k[i];
	}

	// create all objects
	map_ptr_t mapPtr(new MapAbstract(100));
	robodo_ptr_t robPtr(new RobotOdometry(mapPtr));
	robPtr->id(robPtr->robotIds.getId());
	robPtr->linkToParentMap(mapPtr);
	pinhole_ptr_t pinholePtr(new SensorPinhole(robPtr,MapObject::UNFILTERED));
	pinholePtr->id(pinholePtr->sensorIds.getId());
	pinholePtr->linkToParentRobot(robPtr);
	pinholePtr->params.setImgSize(imsz(0), imsz(1));
	pinholePtr->params.setIntrinsicCalibration(k, d, d.size());
	c = pinholePtr->params.correction;
	
	landmark_factory_ptr_t lmkFactory(new LandmarkFactory<LandmarkAnchoredHomogeneousPoint, LandmarkEuclideanPoint>());
	map_manager_ptr_t mmPoint(new MapManager(lmkFactory));
	mmPoint->linkToParentMap(mapPtr);
	ahp_ptr_t ahpPtr(new LandmarkAnchoredHomogeneousPoint(mapPtr));
	ahpPtr->linkToParentMapManager(mmPoint);
	obs_ph_ahp_ptr_t obspaPtr(new ObservationPinHoleAnchoredHomogeneousPoint(pinholePtr, ahpPtr));
	obspaPtr->linkToParentAHP(ahpPtr);
	obspaPtr->linkToPinHole(pinholePtr);

	// setup objects
	cout << "\n% INIT \n%============" << endl;
	robPtr->pose.x(quaternion::originFrame());

	cout << "R.x = [0 0 0 1 0 0 0]';" << endl;
	cout << "R = updateFrame(R);" << endl;
	cout << "S.x = " << (MATLAB) senFrame << endl;
	cout << "S = updateFrame(S);" << endl;
	cout << "k = " << (MATLAB) k << endl;
	cout << "d = " << (MATLAB) d << endl;
	cout << "c = " << (MATLAB) c << endl;
	cout << "u = " << (MATLAB) u << endl;

	cout << "\n% RETRO-PROJECT \n%============" << endl;
	cout << "S.x = " << (MATLAB) senFrame << endl;
	cout << "u = " << (MATLAB) u << endl;
	cout << "r = " << (MATLAB)r << endl;

	// no jac
	obspaPtr->model->backProject_func(senFrame, u, r, ahp);
	cout << "ahp = " << (MATLAB) ahp << endl;
	// jac
	obspaPtr->model->backProject_func(senFrame, u, r, ahp, AHP_s, AHP_u, AHP_r);
	cout << "[ahp_mat, AHP_rob_mat, AHP_s_mat, AHP_k_mat, AHP_c_mat, AHP_u_mat, AHP_r_mat] = retroProjAhmPntFromPinHoleOnRob(R, S, k, c, u, r);" << endl;
	cout << "ahp = " << (MATLAB) ahp << endl;
	cout << "ahp_mat" << endl;
	cout << "ahp_err = norm(ahp - ahp_mat)" << endl;
	cout << "AHP_s = " << (MATLAB) AHP_s << endl;
	cout << "AHP_s_mat" << endl;
	cout << "AHP_s_err = norm(AHP_s - AHP_s_mat)" << endl;
	cout << "AHP_u = " << (MATLAB) AHP_u << endl;
	cout << "AHP_u_mat" << endl;
	cout << "AHP_u_err = norm(AHP_u - AHP_u_mat)" << endl;
	cout << "AHP_r = " << (MATLAB) AHP_r << endl;
	cout << "AHP_r_mat" << endl;
	cout << "AHP_r_err = norm(AHP_r - AHP_r_mat)" << endl;

	cout << "\n% PROJECT \n%============" << endl;
	cout << "S.x = " << (MATLAB) senFrame << endl;
	cout << "ahp = " << (MATLAB) ahp << endl;

	// no jac
	obspaPtr->model->project_func(senFrame, ahp, u2, r2);
	cout << "u2 = " << (MATLAB) u2 << endl;
	cout << "r2 = " << (MATLAB) r2 << endl;
	// jac
	obspaPtr->model->project_func(senFrame, ahp, u2, r2, U_s, U_ahp);
	cout << "[u2_mat, r2_mat, U2_rob_mat, U2_s_mat, U2_k_mat, U2_d_mat, U2_ahp_mat] = projAhmPntIntoPinHoleOnRob(R, S, k, d, ahp);" << endl;
	cout << "r2 = " << (MATLAB) r2 << endl;
	cout << "u2 = " << (MATLAB) u2 << endl;
	cout << "u2_mat" << endl;
	cout << "u2_err = norm(u2 - u2_mat)" << endl;
	cout << "U2_s = " << (MATLAB) U_s << endl;
	cout << "U2_s_mat" << endl;
	cout << "U2_s_err = norm(U2_s - U2_s_mat)" << endl;
	cout << "U2_ahp = " << (MATLAB) U_ahp << endl;
	cout << "U2_ahp_mat" << endl;
	cout << "U2_ahp_err = norm(U2_ahp - U2_ahp_mat)" << endl;


	cout << "\n% PRODUCT OF FWD AND BCKWD JACOBIANS\n%===================" << endl;
	mat U_u(2, 2), AHP_ahp(7, 7);
	U_u = ublas::prod(U_ahp, AHP_u);
	cout << "U_u = " << (MATLAB) U_u << endl;

	cout << "\n\n\n" << endl;

}

BOOST_AUTO_TEST_CASE( test_obsap )
{
	test_obsap01();
}
