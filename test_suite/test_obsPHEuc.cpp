/*
 * test_obsPHEuc.cpp
 *
 *  Created on: Apr 15, 2010
 *      Author: agonzale
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
#include "rtslam/landmarkEuclideanPoint.hpp"
#include "rtslam/observationPinHoleEuclideanPoint.hpp"
#include "rtslam/kalmanFilter.hpp"
#include "rtslam/robotInertial.hpp"

#include "rtslam/observationPinHoleAnchoredHomogeneous.hpp"

//#include <map>

using namespace jblas;
using namespace jafar::jmath;
using namespace jafar::jmath::ublasExtra;
using namespace jafar::rtslam;
using namespace boost;

void test_obsPHEuc01() {

	cout << "\n\n\n% ######    TEST OBS PH EUC PT   ######\n" << endl;

	using namespace boost;

	vec7 robFrame;
	for (int i = 0; i < 7; i++)
		robFrame(i) = i + 4;

	map_ptr_t mapPtr(new MapAbstract(100));
	mapPtr->fillSeq();

	robconstvel_ptr_t robPtr(new RobotConstantVelocity(mapPtr));
	robPtr->linkToParentMap(mapPtr);
	robPtr->state.clear();
	robPtr->pose.x(robFrame);

	pinhole_ptr_t senPtr(new SensorPinHole(robPtr, MapObject::FILTERED));
	senPtr->linkToParentRobot(robPtr);
	senPtr->state.clear();
	senPtr->pose.x(robFrame);

	//rtslam stuff
	eucp_ptr_t lmkPtr(new LandmarkEuclideanPoint(mapPtr));
	lmkPtr->linkToParentMap(mapPtr);

	//		ahp_ptr_t lmkPtr(new LandmarkAnchoredHomogeneousPoint(mapPtr));
	//		lmkPtr->linkToParentMap(mapPtr);

	obs_ph_euc_ptr_t obsPtr(new ObservationPinHoleEuclideanPoint(senPtr, lmkPtr));
	obsPtr->linkToPinHole(senPtr);
	obsPtr->linkToParentEUC(lmkPtr);

	vec3 lmkPoint;
	randVector(lmkPoint);
	lmkPtr-> state.x() = lmkPoint;

	vec4 k;
	vec d;
	vec2 imSize;
	
	randVector(k);
	senPtr->params.setImgSize(imSize(0), imSize(1));
	senPtr->params.setIntrinsicCalibration(k, d, d.size());

	obsPtr->project();

	//Matlab stuff
	cout << "Rf = " << (MATLAB) robPtr->pose.x() << endl;
	cout << "Sf = " << (MATLAB) senPtr->pose.x() << endl;
	cout << "Spk = " << (MATLAB) senPtr->params.intrinsic << endl;
	cout << "Spd = " << (MATLAB) senPtr->params.distortion << endl;
	cout << "l = " << (MATLAB) lmkPoint << endl;
	cout
	    << "[um, sm, U_rm, U_sm, U_km, U_dm, U_lm] = projEucPntIntoPinHoleOnRob(Rf, Sf, Spk, Spd, l);"
	    << endl;

	//Checking differences
	cout << "EXP_r_rt = " << (MATLAB) obsPtr->EXP_sg << endl;
	cout << "urt = " << (MATLAB) obsPtr->expectation.x() << endl;
	cout << "EXP_rsl_m = [U_rm U_lm]" << endl;
	cout << "u_err = norm(urt - um)" << endl;
	cout << "EXP_rsl_err = norm(EXP_rsl_rt - EXP_rsl_m)" << endl;
}

BOOST_AUTO_TEST_CASE( test_obsPHEuc )
{
	test_obsPHEuc01();
}

