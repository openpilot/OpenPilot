/**
 * test_gaussian.cpp
 *
 *  Created on: 28/02/2010
 *      Author: jsola
 *
 *  \file test_gaussian.cpp
 *
 *  Tests for the class Gaussian
 *
 * \ingroup rtslam
 */
// boost unit test includes
#include <boost/test/auto_unit_test.hpp>

// jafar debug include
#include "kernel/jafarDebug.hpp"

#include <iostream>
#include "jmath/jblas.hpp"
#include "jmath/ublasExtra.hpp"
#include "rtslam/gaussian.hpp"
#include "jmath/matlab.hpp"
#include "jmath/random.hpp"

using namespace std;
using namespace jblas;
using namespace jafar::jmath;
using namespace jafar::rtslam;

void test_gaussian01(void) { // TESTS FOR THE GAUSSIAN CLASS


	size_t N = 4;
	size_t n = 2;
	vec x;
	sym_mat P;
	randVector(x, N);
	randMatrix(P, N, N);

//	cout << "\n% EMPTY CONSTRUCTOR\n% ===========" << endl;
//	cout << "% Gaussian G0;" << endl;
//	Gaussian G0;
//	cout << G0 << endl;

	cout << "\n% SIZE CONSTRUCTOR\n% ===========" << endl;
	cout << "% Gaussian GS(n);" << endl;
	Gaussian GS(n);
	cout << "GS: " << GS << endl;
	GS.clear();
	cout << "GS: " << GS << endl;

	cout << "\n% LOCAL CONSTRUCTOR \n% ===========" << endl;
	cout << "x = " << (MATLAB) x << endl;
	cout << "P = " << (MATLAB) P << endl;
	cout << "% Gaussian Gl(x, P);" << endl;
	Gaussian Gl(x, P);
	cout << "Gl: " << Gl << endl;
	cout << "Gl.x = " << (MATLAB) (vec) Gl.x() << endl;
	cout << "Gl.P = " << (MATLAB) Gl.P() << endl;

	cout << "\n% COPY-CONSTRUCT FROM LOCAL GAUSSIAN \n% ===========" << endl;
	cout << "% Gaussian Gl2(Gl);" << endl;
	Gaussian Gl2(Gl);
	cout << "Gl: " << Gl << endl;
	cout << "Gl2: " << Gl2 << endl;
	Gl.x()(0) = 99;
	cout << "Gl.x()(0) = 99; " << endl;
	cout << "Gl: " << Gl << endl;
	cout << "Gl2: " << Gl2 << endl;

	//	cout << "Gl2.x = " << (MATLAB) (vec) Gl2.x() << endl;
	//	cout << "Gl2.P = " << (MATLAB) Gl2.P() << endl;

	// Remote Gaussian, referred to G
	cout << "\n% REMOTE CONSTRUCTOR, G \n% ===========" << endl;
	ind_array ia(n);
	ia(0) = 2;
	ia(1) = 0;
	cout << "% Gaussian Gr(Gl, ia);" << endl;
	Gaussian Gr(Gl, ia);
	cout << "Gr: " << Gr << endl;
	cout << "ia = " << (MATLAB) ia << endl;
	cout << "x = " << (MATLAB) (vec) Gl.x() << endl;
	cout << "P = " << (MATLAB) Gl.P() << endl;
	cout << "Gr.ia = " << (MATLAB) Gr.ia() << endl;
	cout << "Gr.x = " << (MATLAB) (vec) Gr.x() << endl;
	cout << "Gr.P = " << (MATLAB) Gr.P() << endl;

	// Remote Gaussian, referred to {x, P}
	cout << "\n% REMOTE CONSTRUCTOR, {x, P} \n% ===========" << endl;
	cout << "% Gaussian Gxp(x, P, ia);" << endl;
	Gaussian Gxp(x, P, ia);
	cout << "ia = " << (MATLAB) ia << endl;
	cout << "x = " << (MATLAB) (vec) x << endl;
	cout << "P = " << (MATLAB) P << endl;
	cout << "Gxp: " << Gxp << endl;
	cout << "Gxp.ia = " << (MATLAB) Gxp.ia() << endl;
	cout << "Gxp.x = " << (MATLAB) (vec) Gxp.x() << endl;
	cout << "Gxp.P = " << (MATLAB) Gxp.P() << endl;

	cout << "\n% COPY-CONSTRUCT FROM REMOTE GAUSSIAN \n% ===========" << endl;
	Gaussian Gl3(Gr);
	cout << "Gr: " << Gr << endl;
	cout << "Gl3: " << Gl3 << endl;
	Gr.P()(0, 1) = 55;
	cout << "Gr.P()(0,1) = 55;" << endl;
	cout << "Gr: " << Gr << endl;
	cout << "Gl3: " << Gl3 << endl;

	cout << "\n% CHECK OPERATOR << \n% ===========" << endl;
	cout << "Gr: " << Gr << endl;
	cout << "Gl: " << Gl << endl;
	cout << "Gxp:" << Gxp << endl;

	cout << "\n% CHANGE LOCAL STORAGE, CHECK REMOTE GAUSSIAN \n% ===========" << endl;
	Gl.x()(2) = 103.0;
	cout << "ia = " << (MATLAB) ia << endl;
	cout << "Gl.x(3) = 103.0;" << endl;
	cout << "Gr.x = " << (MATLAB) (vec) Gr.x() << endl;

	cout << "\n% CHANGE REMOTE STORAGE, CHECK REMOTE AND LOCAL GAUSSIANS \n% ===========" << endl;
	Gr.x()(1) = 2.0;
	Gr.P()(0, 1) = 12.0;
	Gr.P()(0, 0) = 11.0;
	cout << "ia = " << (MATLAB) ia << endl;
	cout << "Gr.x(2) = 2.0;" << endl;
	cout << "Gr.P(1,1) = 11.0;" << endl;
	cout << "Gr.P(1,2) = 12.0;" << endl;
	cout << "% Now check remotely stored Gaussian -----------" << endl;
	cout << "Gr.x = " << (MATLAB) (vec) Gr.x() << endl;
	cout << "Gr.P = " << (MATLAB) Gr.P() << endl;
	cout << "% Now check locally stored Gaussian -----------" << endl;
	cout << "ia = " << (MATLAB) ia << endl;
	cout << "Gl.x = " << (MATLAB) (vec) Gl.x() << endl;
	cout << "Gl.P = " << (MATLAB) Gl.P() << endl;

	cout << "\n% SET X \n% ===========" << endl;
	vec xN(N);
	vec xn(n);
	randVector(xN);
	randVector(xn);
	cout << "xN = " << (MATLAB) xN << endl;
	Gl.x(xN);
	cout << "Gl.x = " << (MATLAB) (vec) Gl.x() << endl;
	cout << "xn = " << (MATLAB) (vec) xn << endl;
	Gr.x(xn);
	cout << "Gr.x = " << (MATLAB) (vec) Gr.x() << endl;
	cout << "Gl.x = " << (MATLAB) (vec) Gl.x() << endl;

	cout << "\n% SET P \n% ===========" << endl;
	sym_mat PN(N);
	sym_mat Pn(n);
	randMatrix(PN);
	randMatrix(Pn);
	cout << "PN = " << (MATLAB) PN << endl;
	Gl.P(PN);
	cout << "Gl.P = " << (MATLAB) Gl.P() << endl;
	cout << "Pn = " << (MATLAB) Pn << endl;
	Gr.P(Pn);
	cout << "Gr.P = " << (MATLAB) Gr.P() << endl;
	cout << "Gl.P = " << (MATLAB) Gl.P() << endl;

	cout << "\n% SET STD \n% ===========" << endl;
	vec stdN(N);
	vec stdn(n);
	randVector(stdN);
	randVector(stdn);
	cout << "stdN = " << (MATLAB) stdN << endl;
	Gl.std(stdN);
	cout << "Gl.P = " << (MATLAB) Gl.P() << endl;
	cout << "stdn = " << (MATLAB) stdn << endl;
	Gr.std(stdn);
	cout << "Gr.P = " << (MATLAB) Gr.P() << endl;
	cout << "Gl.P = " << (MATLAB) Gl.P() << endl;

	cout << "\n% CLEAR \n% ===========" << endl;
	Gl.x(x);
	Gl.P(P);
	cout << "Gr.clear();" << endl;
	Gr.clear();
	cout << "Gr.x = " << (MATLAB) (vec) Gr.x() << endl;
	cout << "Gr.P = " << (MATLAB) Gr.P() << endl;
	cout << "Gl.x = " << (MATLAB) (vec) Gl.x() << endl;
	cout << "Gl.P = " << (MATLAB) Gl.P() << endl;
	cout << "Gl.clear();" << endl;
	Gl.clear();
	cout << "Gr.x = " << (MATLAB) (vec) Gr.x() << endl;
	cout << "Gr.P = " << (MATLAB) Gr.P() << endl;
	cout << "Gl.x = " << (MATLAB) (vec) Gl.x() << endl;
	cout << "Gl.P = " << (MATLAB) Gl.P() << endl;

	cout << "\n% SMALL ACCESSORS AND FUNCTIONS \n% ===========" << endl;
	cout << "Gl.storage = " << Gl.storage() << endl;
	cout << "Gl.size = " << Gl.size() << endl;
	cout << "Gl.hasNullCov = " << Gl.hasNullCov() << endl;
	cout << "Gr.storage = " << Gr.storage() << endl;
	cout << "Gr.size = " << Gr.size() << endl;
	cout << "Gr.hasNullCov = " << Gr.hasNullCov() << endl;

	cout << "\n% SET OFF-DIAGONAL P \n%=============" << endl;
	mat M;
	randMatrix(M, 2, 2);
	cout << "M = " << (MATLAB) (mat) M << endl;
	ind_array ia1(2), ia2(2);
	ia1(0) = 0;
	ia1(1) = 1;
	ia2(0) = 1;
	ia2(1) = 2;
	Gl.P(M, ia1, ia2);
	cout << "Gl.P = " << (MATLAB) (mat) Gl.P() << endl;

	//	cout << "\n% SET OFF-DIAGONAL P TO BE NOT-OFF-DIAG (THIS MUST NOT WORK!)\n%=============" << endl;
	//	cout << "M = " << (MATLAB) (mat) M << endl;
	//	ia1(0) = 0;
	//	ia1(1) = 1;
	//	ia2(0) = 1;
	//	ia2(1) = 0;
	//	Gl.set_P_off_diag(M, ia1, ia2);
	//	cout << "Gl.P = " << (MATLAB) (mat) Gl.P() << endl;

}

BOOST_AUTO_TEST_CASE( test_gaussian )
{
	test_gaussian01();
}

