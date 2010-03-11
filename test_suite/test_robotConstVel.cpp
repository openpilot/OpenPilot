/**
 * test_robotConstVel.cpp
 *
 *  Created on: 07/03/2010
 *      Author: jsola
 *
 *  \file test_robotConstVel.cpp
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

#include "jmath/indirectArray.hpp"
#include "rtslam/quatTools.hpp"

#include "rtslam/robotConstantVelocity.hpp"
#include "rtslam/sensorPinHole.hpp"
#include "rtslam/landmarkAnchoredHomogeneousPoint.hpp"


void test_robotConstVel01(void) {
	using namespace jafar::rtslam;
	using namespace jblas;
	using namespace jafar::jmath;
	using namespace jafar::jmath::ublasExtra;

	size_t size_storage = 30;
	size_t size_state = 13;
	size_t size_pert = 6;

	MapAbstract map(size_storage);
	randVector(map.filter.x);
	randMatrix(map.filter.P);

	cout << "\n% ROBOT CREATION AND PRINT \n%===========" << endl;
	jblas::ind_array iar(size_state);
	for (size_t i = 0; i < size_state; i++)
		//		iar(i) = (size_t) (1.5 * (double) i); // Make it weird on purpose to see indirect indexing
		iar(i) = i;
	Robot3DConstantVelocity robot(map, iar, size_pert);
	robot.set_name("Dala");
	robot.state.clear();
	cout << "robot: " << robot << endl;
	size_t id = robot.getNextId();
	cout << "Robot ids: " << id << " " << robot.getNextId() << " " << robot.getNextId() << " " << robot.getNextId() << " " << robot.getNextId() << " " << robot.getNextId() << endl;


	cout << "\n% CAMERA CREATION AND PRINT \n%===========" << endl;
	Gaussian sensorPose(7);
	sensorPose.setHasNullCov(true);
	jblas::vec4 k;
	jblas::vec dist(3);
	jblas::vec corr(3);
	size_t hsize = 640;
	size_t vsize = 480;

	SensorPinHole camera1(sensorPose, k, dist, corr, hsize, vsize);
	cout << "camera1: " << camera1 << endl;
	camera1.name = "Flea2";

	jblas::ind_array ias(7);
	for (size_t i = 0; i < 7; i++)
		ias(i) = i + 13;
	SensorPinHole camera2(map, ias, k, dist, corr, hsize, vsize);
	cout << "camera2: " << camera2 << endl;

	jblas::vec posevec = sensorPose.x();
	SensorPinHole camera3(posevec, k, dist, corr, hsize, vsize);
	cout << "camera3: " << camera3 << endl;

	camera1.installToRobot(robot);


	cout << "\n% PARENTAL ACCESS \n%===========" << endl;
	cout << "Robot " << robot.name << " has sensor " << robot.sensorsList.front()->name << " of type "
	    << robot.sensorsList.front()->type << endl;
	cout << "Sensor " << camera1.name << " is on robot " << camera1.robot->name << endl;


	cout << "\n% LANDMARK CREATION AND PRINT \n%===========" << endl;
	jblas::ind_array ial(7);
	for (size_t i = 0; i < 7; i++)
		ial(i) = i + 20;
	Landmark3DAnchoredHomogeneousPoint ahp1(map, ial);
	cout << "lmk1: " << ahp1 << endl;

	jblas::vec7 F;
	jblas::vec3 T;
	jblas::vec3 E;
	for (size_t i = 0; i < 3; i++) {
		E(i) = 0.1 * (i + 1);
		T(i) = i + 1;
	}
	subrange(F, 0, 3) = T;
	subrange(F, 3, 7) = quaternion::e2q(E);
	cout << "F = " << F << endl;
	cout << "ahpf= " << ahp1.state.x() << endl;



	cout << "\n% LANDMARK FROM FRAME \n%==================" << endl;
	jblas::vec ahp(7), ahpf(7);
	jblas::mat AHP_f(7, 7), AHP_ahpf(7, 7);
	ahpf = ahp1.state.x();
	//	ahp = landmarkAHP::fromFrame(F, ahpf);
	landmarkAHP::fromFrame(F, ahpf, ahp, AHP_f, AHP_ahpf);

	cout << "F.x = " << MATLAB (F) << endl;
	cout << "F = updateFrame(F);" << endl;
	cout << "ahpf = " << (MATLAB) (vec) ahpf << endl;
	cout << "ahp = " << (MATLAB) ahp << endl;
	cout << "AHP_f = " << (MATLAB) AHP_f << endl;
	cout << "AHP_ahpf = " << (MATLAB) AHP_ahpf << endl;
	cout << "[ahp_m,AHP_f_m,AHP_ahpf_m] = fromFrameAhm(F,ahpf);" << endl;
	cout << "ahp_err = norm(ahp-ahp_m)" << endl;
	cout << "AHP_f_err = norm(AHP_f-AHP_f_m)" << endl;
	cout << "AHP_ahpf_err = norm(AHP_ahpf-AHP_ahpf_m)" << endl;

	cout << "\n% LANDMARK TO FRAME \n%==================" << endl;
	jblas::mat AHPF_f(7, 7), AHPF_ahp(7, 7);
	landmarkAHP::toFrame(F, ahp, ahpf, AHPF_f, AHPF_ahp);
	//	ahpf = landmarkAHP::toFrame(F, ahp);
	cout << "ahp = " << (MATLAB) ahp << endl;
	cout << "ahpf = " << (MATLAB) (vec) ahpf << endl;
	cout << "AHPF_f = " << (MATLAB) AHPF_f << endl;
	cout << "AHPF_ahp = " << (MATLAB) AHPF_ahp << endl;
	cout << "[ahpf_m,AHPF_f_m,AHPF_ahp_m] = toFrameAhm(F,ahp);" << endl;
	cout << "ahpf_err = norm(ahpf-ahpf_m)" << endl;
	cout << "AHPF_f_err = norm(AHPF_f-AHPF_f_m)" << endl;
	cout << "AHPF_ahp_err = norm(AHPF_ahp-AHPF_ahp_m)" << endl;

	cout << "\n% REPARAMETRIZATION \n%==================" << endl;
	cout << "ahp = " << (MATLAB) ahp << endl;
	jblas::vec3 euc;
	jblas::mat EUC_ahp(3, 7);
	//	euc = landmarkAHP::ahp2euc(ahp);
	landmarkAHP::ahp2euc(ahp, euc, EUC_ahp);
	cout << "euc = " << (MATLAB) euc << endl;
	cout << "EUC_ahp = " << (MATLAB) EUC_ahp << endl;
	cout << "[euc_m, EUC_ahp_m] = ahm2euc(ahp);" << endl;
	cout << "euc_err = norm(euc - euc_m)" << endl;
	cout << "EUC_ahp_err = norm(EUC_ahp-EUC_ahp_m)" << endl;



}




BOOST_AUTO_TEST_CASE( test_robotConstVel )
{
	test_robotConstVel01();
}


