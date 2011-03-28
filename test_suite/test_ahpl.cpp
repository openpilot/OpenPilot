/**
 * \file test_ahpl.cpp
 *
 * \date 17/03/2011
 * \author bhautboi@laas.fr
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
#include "rtslam/landmarkAnchoredHomogeneousPointsLine.hpp"

using namespace std;
using namespace jafar;
using namespace jafar::rtslam;
using namespace jblas;
using namespace jafar::jmath;
using namespace jafar::jmath::ublasExtra;
using namespace lmkAHPL;

void test_ahpl01(void) {

   vec11 ahpl;
   vec7 senFrame;
   vec11 v1, v2;
//   double r, id;
   mat AHPL_s(11, 7);
   mat AHPL_v(11, 11);
   mat V_s(11, 7);
   mat V_ahpl(11, 11);

   cout << "\n% INIT \n%============" << endl;
   for (size_t i = 0; i < 8; i++)
   {
      if(i == 6 || i == 10)
         v1(i) = 1;
      else
         v1(i) = i + 2;
   }
   subrange(senFrame, 0, 3) = subrange(v1,0,3);
   subrange(senFrame, 3, 7) = quaternion::e2q(subrange(v1,0,3));
   cout << "F.x = " << (MATLAB) senFrame << endl;
   cout << "v = " << (MATLAB) v1 << endl;
   cout << "F = updateFrame(F);" << endl;

   cout << "\n% FROM FRAME \n%============" << endl;
   cout << "F.x = " << (MATLAB) senFrame << endl;
   cout << "v = " << (MATLAB) v1 << endl;
   ahpl = lmkAHPL::fromFrame(senFrame, v1);
   cout << "ahpl1 = " << (MATLAB) ahpl << endl;
   lmkAHPL::fromFrame(senFrame, v1, ahpl, AHPL_s, AHPL_v);
   cout << "ahpl2 = " << (MATLAB) ahpl << endl;
   cout << "AHPL_s = " << (MATLAB) AHPL_s << endl;
   cout << "AHPL_v = " << (MATLAB) AHPL_v << endl;
   cout << "[ahpl_mat, AHPL_s_mat, AHPL_v_mat] = fromFrameAhmLin(F, v)" << endl;
   cout << "ahpl_err = norm(ahpl1 - ahpl_mat)" << endl;
   cout << "AHPL_s_err = norm(AHPL_s - AHPL_s_mat)" << endl;
   cout << "AHPL_v_err = norm(AHPL_v - AHPL_v_mat)" << endl;

/*
   JFR_CHECK_EQUAL(ahpl(3), -3.70579);
   JFR_CHECK_EQUAL(AHPL_s(3,5), 5.98611);
   JFR_CHECK_EQUAL(AHPL_s(5,5), -0.482439);
   JFR_CHECK_EQUAL(AHPL_v(3,2), 0);
   JFR_CHECK_EQUAL(AHPL_v(5,5), 0.411982);
*/

   cout << "\n% TO FRAME \n%============" << endl;
   cout << "F.x = " << (MATLAB) senFrame << endl;
   cout << "ahpl = " << (MATLAB) ahpl << endl;
   v2 = lmkAHPL::toFrame(senFrame, ahpl);
   JFR_CHECK_VEC_EQUAL(v2, v1);
//   lmkAHPL::toFrame(senFrame, ahpl, v2);
//   JFR_CHECK_VEC_EQUAL(v2, v1);
//   JFR_CHECK_EQUAL(id, r);
   lmkAHPL::toFrame(senFrame, ahpl, v2, V_s, V_ahpl);
   cout << "v2 = " << (MATLAB) v2 << endl;
   cout << "V_s = " << (MATLAB) V_s << endl;
   cout << "V_ahpl = " << (MATLAB) V_ahpl << endl;
   cout << "[v_mat, V_s_mat, V_ahpl_mat] = toFrameAhmLin(F, ahpl)" << endl;
   cout << "v_err = norm(v2 - v_mat)" << endl;
   cout << "V_s_err = norm(V_s - V_s_mat)" << endl;
   cout << "V_ahpl_err = norm(V_ahpl - V_ahpl_mat)" << endl;
   JFR_CHECK_VEC_EQUAL(v2, v1);
//   JFR_CHECK_EQUAL(V_s(0,1), -4.034721000604516);
//   JFR_CHECK_EQUAL(V_ahpl(2,1), 3.440047966271193);

   cout << "\n% PRODUCT OF FWD AND BCKWD JACOBIANS\n%===================" << endl;
   mat V_v(11, 11), AHPL_ahpl(11, 11);
   V_v = ublas::prod(V_ahpl, AHPL_v);
   AHPL_ahpl = ublas::prod(AHPL_v, V_ahpl);
   identity_mat I11(11);
   cout << "V_v = " << (MATLAB) V_v << endl;
   cout << "AHPL_ahpl = " << (MATLAB) AHPL_ahpl << endl;
   cout << "V_ahpl*AHPL_s + V_s = " << (MATLAB) (prod(V_ahpl, AHPL_s) + V_s) << endl;
   cout << "V_ahpl*AHPL_s = " << (MATLAB) (prod(V_ahpl, AHPL_s)) << endl;
   cout << "V_s = " << (MATLAB) ( V_s) << endl;
   JFR_CHECK_MAT_EQUAL(V_v, I11);

}

BOOST_AUTO_TEST_CASE( test_ahpl )
{
   test_ahpl01();
}

