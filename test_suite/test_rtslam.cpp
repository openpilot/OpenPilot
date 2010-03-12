/**
 * test_rtslam.cpp
 *
 *  Created on: 12/03/2010
 *      Author: jsola
 *
 *  \file test_rtslam.cpp
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

#include "jmath/jblas.hpp"
#include "jmath/indirectArray.hpp"

#include "rtslam/quatTools.hpp"

#include "rtslam/robotConstantVelocity.hpp"
#include "rtslam/sensorPinHole.hpp"
#include "rtslam/landmarkAnchoredHomogeneousPoint.hpp"
#include "kernel/IdFactory.hpp"

using namespace jafar::rtslam;
using namespace jblas;
using namespace jafar::jmath;
using namespace jafar::jmath::ublasExtra;

void test_rtslam01(void) {

	size_t size_map = 30;
	size_t size_pert = 6;

	MapAbstract map(size_map);
	randVector(map.filter.x);
	randMatrix(map.filter.P);

	cout << "\n% ROBOT CREATION AND PRINT \n%===========" << endl;
	jblas::ind_array iar = ia_pushfront(map.used_states, Robot3DConstantVelocity::size());
	Robot3DConstantVelocity robot(map, iar, size_pert);
	robot.name("Dala");
	robot.state.clear();
	jafar::kernel::IdFactory IdFac;
	robot.id(IdFac.getId());
	cout << "robot: " << robot << endl;

	cout << "\n% CAMERA CREATION AND PRINT \n%===========" << endl;
	Gaussian sensorPose(7);
	sensorPose.setHasNullCov(true);
	jblas::vec4 k;
	jblas::vec dist(3);
	jblas::vec corr(3);
	size_t hsize = 640;
	size_t vsize = 480;

	SensorPinHole camera1(sensorPose, k, dist, corr, hsize, vsize);
	camera1.id(IdFac.getId());
	camera1.name("Flea");
	cout << "camera1: " << camera1 << endl;

	jblas::ind_array ias = ia_pushfront(map.used_states, SensorPinHole::size());
	SensorPinHole camera2(map, ias, k, dist, corr, hsize, vsize);
	camera2.id(IdFac.getId());
	camera1.name("Flea2");
	cout << "camera2: " << camera2 << endl;

	jblas::vec posevec = sensorPose.x();
	SensorPinHole camera3(posevec, k, dist, corr, hsize, vsize);
	camera3.id(IdFac.getId());
	camera1.name("Marlin");
	cout << "camera3: " << camera3 << endl;

	camera1.installToRobot(robot);


	cout << "\n% PARENTAL ACCESS \n%===========" << endl;
	cout << "Robot " << robot.name() << " has sensor " << robot.sensorsList.front()->name() << " of type "
	    << robot.sensorsList.front()->type() << endl;
	cout << "Sensor " << camera1.name() << " is on robot " << camera1.robot->name() << endl;

	cout << "\n% LANDMARK CREATION AND PRINT \n%===========" << endl;
	jblas::ind_array ial = ia_pushfront(map.used_states, Landmark3DAnchoredHomogeneousPoint::size());
	Landmark3DAnchoredHomogeneousPoint ahp1(map, ial);
	ahp1.id(IdFac.getId());
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

	cout << "F.x = " << MATLAB(F) << endl;
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
	test_rtslam01();
}

