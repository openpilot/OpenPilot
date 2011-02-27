/*
 * test_euclidean.cpp
 *
 *  Created on: Apr 14, 2010
 *      Author: agonzale
 */

// boost unit test includes
#include <boost/test/auto_unit_test.hpp>

// jafar debug include
#include "kernel/jafarDebug.hpp"
#include "kernel/jafarTestMacro.hpp"

#include <iostream>
#include "jmath/matlab.hpp"
#include "jmath/random.hpp"

#include "rtslam/landmarkEuclideanPoint.hpp"
#include "rtslam/landmarkAnchoredHomogeneousPoint.hpp"
#include "rtslam/mapManager.hpp"

using namespace std;
using namespace jafar;
using namespace jafar::rtslam;
using namespace jblas;
using namespace jafar::jmath;
using namespace jafar::jmath::ublasExtra;

void test_euc01(void) {

	vec3 euc;
	vec7 senFrame;
	vec3 v1;
	double r;
	mat EUC_f(3,7);
	mat33 EUC_euc;

	map_ptr_t mapPtr(new MapAbstract(100));
	landmark_factory_ptr_t lmkFactory(new LandmarkFactory<LandmarkAnchoredHomogeneousPoint, LandmarkEuclideanPoint>());
	map_manager_ptr_t mmPoint(new MapManager(lmkFactory));
	mmPoint->linkToParentMap(mapPtr);
	eucp_ptr_t eucPtr(new LandmarkEuclideanPoint(mapPtr));
	eucPtr->linkToParentMapManager(mmPoint);

	cout << "\n% BIGGINING TEST_ENCLIDEAN \n%============" << endl;
	for (size_t i = 0; i < 3; i++)
		v1(i) = i + 2;
		subrange(senFrame, 0, 3) = v1;
		subrange(senFrame, 3, 7) = quaternion::e2q(v1);
		r = 1;
		cout << "F.x = " << (MATLAB) senFrame << endl;
		cout << "v = " << (MATLAB) v1 << endl;
		cout << "r = " << r << endl;
		cout << "F = updateFrame(F);" << endl;

		cout << "\n% FROM FRAME \n%============" << endl;
		cout << "F.x = " << (MATLAB) senFrame << endl;
		cout << "euc_m = "<< (MATLAB) eucPtr->state.x()<<endl;
		euc = quaternion::vecFromFrame(senFrame,v1);
		cout << "euc1 = " << (MATLAB) euc << endl;
		quaternion::vecFromFrame(senFrame, v1, euc, EUC_f, EUC_euc);
		cout << "euc2 = " << (MATLAB) euc << endl;
		cout << "EUC_f = " << (MATLAB) EUC_f << endl;
		cout << "EUC_euc = " << (MATLAB) EUC_euc << endl;
		cout << "[euc_mat, EUC_f_mat, EUC_euc_mat] = fromFrame(F, euc_m)" << endl;
		cout << "euc_err = norm(euc1 - euc_mat)" << endl;
		cout << "EUC_f_err = norm(EUC_f - EUC_f_mat)" << endl;
		cout << "EUC_euc_err = norm(EUC_euc - EUC_euc_mat)" << endl;
		cout << "\n% END OF TEST_ENCLIDEAN \n%============" << endl;


}

BOOST_AUTO_TEST_CASE( test_euclidean )
{
	test_euc01();
}
