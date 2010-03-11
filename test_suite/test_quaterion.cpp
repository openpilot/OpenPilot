/**
 * test_quaterion.cpp
 *
 *  Created on: 28/02/2010
 *      Author: jsola
 *
 *  \file test_quaterion.cpp
 *
 *  ## Add a description here ##
 *
 */

// boost unit test includes
#include <boost/test/auto_unit_test.hpp>

// jafar debug include
#include "kernel/jafarDebug.hpp"


#include <iostream>
#include "jmath/jblas.hpp"
//#include "jmath/ublasExtra.hpp"
#include "rtslam/quatTools.hpp"
//#include <boost/numeric/ublas/blas.hpp>
#include "jmath/matlab.hpp"
#include "jmath/random.hpp"

namespace ublas = boost::numeric::ublas;

using namespace jafar::jmath;
using namespace std;

void test_quaternion01(void) { // TESTS FOR QUATTOOLS.HPP IN RTSLAM

	using namespace ublas;
	using namespace jblas;
	using namespace jafar::rtslam::quaternion;

	// Initialization
	vec q_(4);
	randVector(q_);
	jafar::jmath::ublasExtra::normalize(q_);
	vec4 q(q_);
	vec v_(3);
	randVector(v_);
	vec3 v(v_);

	// quaternion things
	cout << "\n% QUATERNION THINGS\n% ==============" << endl;
	cout << "q = " << (MATLAB) q << endl;
	vec4 qc(q2qc(q));
	cout << "qc = " << (MATLAB) qc << endl;
	mat33 R, Rt;
	R = q2R(q);
	cout << "R = " << (MATLAB) (mat) R << endl;
	Rt = q2Rt(q);
	cout << "Rt = " << (MATLAB) (mat) Rt << endl;

	// quaternion jacobians
	mat44 QC_q;
	q2qc_by_dq(q, QC_q);
	cout << "QC_q = " << (MATLAB) (mat) QC_q << endl;

	// vector and quaternion things
	cout << "\n% ROTATIONS\n% ================" << endl;
	cout << "q = " << (MATLAB) q << endl;
	cout << "v = " << (MATLAB) v << endl;
	vec3 vo;
	vo = prod(R, v);
	cout << "vo1 = " << (MATLAB) vo << endl;
	vo = RofQtimesV(q, v);
	cout << "vo2 = " << (MATLAB) vo << endl;
	mat VO_q(3, 4);
	mat VO_v(3, 3);
	RofQtimesV_by_dq(q, v, VO_q);
	RofQtimesV_by_dv(q, VO_v);
	cout << "VO_q = " << (MATLAB) (mat) VO_q << endl;
	cout << "VO_v = " << (MATLAB) (mat) VO_v << endl;
	RofQtimesV(q, v, vo, VO_q, VO_v);
	cout << "vo = " << (MATLAB) vo << endl;
	cout << "VO_q = " << (MATLAB) (mat) VO_q << endl;
	cout << "VO_v = " << (MATLAB) (mat) VO_v << endl;
	cout << "[vo_m, VO_q_m, VO_v_m] = Rp(q,v)" << endl;
	cout << "vo - vo_m" << endl;
	cout << "VO_q - VO_q_m" << endl;
	cout << "VO_v - VO_v_m" << endl;

	// inverse rotations
	cout << "\n% INVERSE ROTATIONS\n% ================" << endl;
	cout << "q = " << (MATLAB) q << endl;
	cout << "v = " << (MATLAB) v << endl;
	vo = RTofQtimesV(q, v);
	cout << "vo = " << (MATLAB) vo << endl;
	RTofQtimesV_by_dq(q, v, VO_q);
	RTofQtimesV_by_dv(q, VO_v);
	cout << "VO_q = " << (MATLAB) (mat) VO_q << endl;
	cout << "VO_v = " << (MATLAB) (mat) VO_v << endl;
	RTofQtimesV(q, v, vo, VO_q, VO_v);
	cout << "vo = " << (MATLAB) vo << endl;
	cout << "VO_q = " << (MATLAB) (mat) VO_q << endl;
	cout << "VO_v = " << (MATLAB) (mat) VO_v << endl;
	cout << "[vo_m, VO_q_m, VO_v_m] = Rtp(q,v)" << endl;
	cout << "vo - vo_m" << endl;
	cout << "VO_q - VO_q_m" << endl;
	cout << "VO_v - VO_v_m" << endl;

	// quaternion product
	cout << "\n% QUATERNION PRODUCT\n% ==================" << endl;
	randVector(q_);
	jafar::jmath::ublasExtra::normalize(q_);
	vec4 q1(q_);
	randVector(q_);
	jafar::jmath::ublasExtra::normalize(q_);
	vec4 q2(q_);
	cout << "q1 = " << (MATLAB) q1 << endl;
	cout << "q2 = " << (MATLAB) q2 << endl;
	mat Q_q1(4, 4);
	mat Q_q2(4, 4);
	qProd(q1, q2, q, Q_q1, Q_q2);
	cout << "q = " << (MATLAB) q << endl;
	cout << "Q_q1 = " << (MATLAB) (mat) Q_q1 << endl;
	cout << "Q_q2 = " << (MATLAB) (mat) Q_q2 << endl;
	cout << "[q_m, Q_q1_m, Q_q2_m] = qProd(q1,q2)" << endl;
	cout << "q - q_m" << endl;
	cout << "Q_q1 - Q_q1_m" << endl;
	cout << "Q_q2 - Q_q2_m" << endl;

	cout << "\n% CONVERSIONS V->Q\n% ===========" << endl;
	v(0) = 1.0;
	v(1) = 2.0;
	v(2) = 3.0;
	cout << "v = " << (MATLAB) v << endl;
	q = v2q(v);
	cout << "q = " << (MATLAB) q << endl;
	mat Q_v(4, 3);
	v2q_by_dv(v, Q_v);
	cout << "Q_v = " << (MATLAB) Q_v << endl;
	cout << "[q_m, Q_v_m] = v2q(v)" << endl;
	cout << "q - q_m" << endl;
	cout << "Q_v - Q_v_m" << endl;

	cout << "\n% CONVERSIONS E->Q\n% ===========" << endl;
	vec3 e(v);
	e(0) = 0.1;
	e(1) = 0.2;
	e(2) = 0.3;
	cout << "e = " << (MATLAB) e << endl;
	q = e2q(e);
	cout << "q = " << (MATLAB) q << endl;
	mat Q_e(4, 3);
	e2q_by_de(e, Q_e);
	e2q(e, q, Q_e);
	cout << "Q_e = " << (MATLAB) Q_e << endl;
	cout << "[q_m, Q_e_m] = e2q(e)" << endl;
	cout << "q - q_m" << endl;
	cout << "Q_e - Q_e_m" << endl;

	cout << "\n% CONVERSIONS Q->E\n% ===========" << endl;
	cout << "q = " << (MATLAB) q << endl;
	e = q2e(q);
	cout << "e = " << (MATLAB) e << endl;
	mat E_q(3, 4);
	q2e(q, e, E_q);
	cout << "e = " << (MATLAB) e << endl;
	cout << "E_q = " << (MATLAB) E_q << endl;
	cout << "[e_m, E_q_m] = q2e(q)" << endl;
	cout << "e - e_m" << endl;
	cout << "E_q - E_q_m" << endl;

	cout << "\n% POINT TO-FRAME TRANSFORMS\n% ===========" << endl;
	vec7 F;
	vec3 t(vo);
	vec3 p(v);
	ublas::subrange(F, 0, 3) = t;
	ublas::subrange(F, 3, 7) = q;
	cout << "F = " << (MATLAB) F << endl;
	cout << "p = " << (MATLAB) p << endl;
	vec3 pf;
	mat PF_f(3, 7);
	mat PF_p(3, 7);
	pf = eucToFrame(F, p);
	cout << "pf = " << (MATLAB) pf << endl;
	eucToFrame(F, p, pf, PF_f, PF_p);
	cout << "pf = " << (MATLAB) pf << endl;
	cout << "PF_f = " << (MATLAB) (mat) PF_f << endl;
	cout << "PF_p = " << (MATLAB) (mat) PF_p << endl;
	cout << "[pf_m, PF_f_m, PF_p_m] = toFrame(F, p)" << endl;
	cout << "pf - pf_m" << endl;
	cout << "PF_f - PF_f_m" << endl;
	cout << "PF_p - PF_p_m" << endl;

	cout << "\n% VECTOR TO-FRAME TRANSFORMS\n% ===========" << endl;
	cout << "F = " << (MATLAB) F << endl;
	cout << "v = " << (MATLAB) v << endl;
	vec3 vf;
	mat VF_f(3, 7), VF_v(3, 3);
	vf = vecToFrame(F, v);
	cout << "vf = " << (MATLAB) vf << endl;
	vecToFrame(F, v, vf, VF_f, VF_v);
	cout << "vf = " << (MATLAB) vf << endl;
	cout << "VF_f = " << (MATLAB) (mat) VF_f << endl;
	cout << "VF_v = " << (MATLAB) (mat) VF_v << endl;
	cout << "[vf_m, VF_f_m, VF_v_m] = toFrame(F-[F(1:3); 0; 0; 0; 0], v);" << endl;
	cout << "VF_f_m(:,1:3) = 0; vf_m, VF_f_m, VF_v_m" << endl;
	cout << "vf - vf_m" << endl;
	cout << "VF_f - VF_f_m" << endl;
	cout << "VF_v - VF_v_m" << endl;

	cout << "\n% POINT FROM-FRAME TRANSFORMS\n% ===========" << endl;
	cout << "F = " << (MATLAB) F << endl;
	cout << "pf = " << (MATLAB) pf << endl;
	mat P_f(3, 7);
	mat P_pf(3, 7);
	p = eucFromFrame(F, pf);
	cout << "p = " << (MATLAB) p << endl;
	eucFromFrame(F, pf, p, P_f, P_pf);
	cout << "p = " << (MATLAB) p << endl;
	cout << "P_f = " << (MATLAB) (mat) P_f << endl;
	cout << "P_pf = " << (MATLAB) (mat) P_pf << endl;
	cout << "[p_m, P_f_m, P_pf_m] = fromFrame(F, pf)" << endl;
	cout << "p - p_m" << endl;
	cout << "P_f - P_f_m" << endl;
	cout << "P_pf - P_pf_m" << endl;

	cout << "\n% VECTOR FROM-FRAME TRANSFORMS\n% ===========" << endl;
	cout << "F = " << (MATLAB) F << endl;
	mat V_f(3, 7), V_vf(3, 3);
	v = vecFromFrame(F, vf);
	cout << "v = " << (MATLAB) v << endl;
	vecFromFrame(F, vf, v, V_f, V_vf);
	cout << "v = " << (MATLAB) v << endl;
	cout << "vf = " << (MATLAB) vf << endl;
	cout << "V_f = " << (MATLAB) (mat) V_f << endl;
	cout << "V_vf = " << (MATLAB) (mat) V_vf << endl;
	cout << "[v_m, V_f_m, V_vf_m] = fromFrame(F-[F(1:3); 0; 0; 0; 0], vf);" << endl;
	cout << "V_f_m(:,1:3) = 0; v_m, V_f_m, V_vf_m" << endl;
	cout << "v - v_m" << endl;
	cout << "V_f - V_f_m" << endl;
	cout << "V_vf - V_vf_m" << endl;

	cout << "\n% FRAME COMPOSITION\n% ===========" << endl;
	cout << "F.x = " << (MATLAB) F << endl;
	jblas::vec7 G(F); // another frame
	cout << "G.x = " << (MATLAB) G << endl;
	jblas::vec7 H; // the composed frame
	H = composeFrames(G, F);
	cout << "H.x = " << (MATLAB) H << endl;
	cout << "F=updateFrame(F); G=updateFrame(G); H=updateFrame(H);" << endl;
	cout << "H_mat = composeFrames(G,F);" << endl;
	cout << "H_err = norm(H.x-H_mat.x)" << endl;

}




BOOST_AUTO_TEST_CASE( test_quaternion )
{
	test_quaternion01();
}

