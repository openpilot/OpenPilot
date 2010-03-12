/**
 * test_ixaxpy_prod.cpp
 *
 *  Created on: 28/02/2010
 *      Author: jsola
 *
 *  \file test_ixaxpy_prod.cpp
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
#include "jmath/matlab.hpp"
#include "jmath/random.hpp"
#include "jmath/ixaxpy.hpp"

using namespace std;
using namespace jafar::jmath;
using namespace ublas;

void test_ixaxpy01(void) { // TEST FOR XAXPY_PROD()

	size_t n;
	matrix<double> M;

	cout << "\n% TEST FOR SYMMETRIC MATRICES\n% ===========" << endl;
	// define an indirect_array
	n = 3;
	jblas::ind_array ia(n);
	ia(0) = 1;
	ia(1) = 2;
	ia(2) = 3;

	matrix<double> m(3, 3);
	randMatrix(m, 3, 3);
	symmetric_adaptor<matrix<double> > s(m);

	M(10, 10);
	randMatrix(M, 10, 10);
	symmetric_adaptor<matrix<double> > P(M);

}

void test_ixaxpy02(void) {
	size_t n;
	matrix<double> M;

	cout << "\n% TEST FOR IXAXPY\n% ===========" << endl;
	size_t N = 4;
	n = 2;

	// first test for complementing and concatenating indirect arrays
	////////////////////////////////
	jblas::ind_array iax(N);
	for (size_t i = 0; i < N; i++)
		iax(i) = i;
	cout << "iax = " << (MATLAB) iax << endl;

	jblas::ind_array iar(n);
	iar(0) = 2;
	iar(1) = 0;
	cout << "iar = " << (MATLAB) iar << endl;

	jblas::ind_array iac = ublasExtra::ia_complement(iax, iar);
	cout << "iac = " << (MATLAB) iac << endl;

	jblas::ind_array iacat = ublasExtra::ia_union(iax, iar);
	cout << "iacat_iax_iar = " << (MATLAB) iacat << endl;
	jblas::ind_array itest(2);
	itest(0) = 9;
	itest(1) = 4;
	cout << "itest =" << (MATLAB) itest << endl;
	iacat = ublasExtra::ia_union(itest, iar);
	cout << "iacat_itest_iar = " << (MATLAB) iacat << endl;
	jblas::ind_array itest0(0);
	cout << "itest0 = " << (MATLAB) itest0 << endl;
	iacat = ublasExtra::ia_union(itest0, iar);
	cout << "iacat_itest0_iar = " << (MATLAB) iacat << endl;

	// now test for ixaxpy_prod()
	///////////////////////////

	// the full, non-sym matrix M, adapted MS, symmetric S, adapted SS
	randMatrix(M, N, N);
	symmetric_adaptor<matrix<double> > MS(M);
	//	symmetric_matrix<double> S(MS);
	jblas::sym_mat S(MS);
	symmetric_matrix<double> S2(S); // copy of S
	symmetric_adaptor<symmetric_matrix<double> > SS(S2);

	// the jacobian
	jblas::mat Hr;
	randMatrix(Hr, n, n);
	cout << "Hr = " << (MATLAB) Hr << endl;

	// Here we print the Matlab operations for generating the full Jacobian
	cout << "H = eye(" << N << "); H(iar,iar) = Hr" << endl;

	// Here we perform P = [Hr 0 ; 0 I] * P * [Hr 0 ; 0 I]'
	// in the optimized fashion: P = [Hr*Srr*Hr' Hr*Srm ; XXXXX Smm]
	// where XXXX is not computed because P is symmetrical,
	// and Smm neither because it's a copy of the original.

	cout << "% With MS = symmetric_adaptor<matrix>" << endl;
	cout << "MS = " << (MATLAB) MS << endl;
	ublasExtra::ixaxpy_prod(MS, iax, Hr, iar);
	cout << "MS_cpp = " << (MATLAB) MS << endl;
	cout << "MS_mat = H * MS * H';" << endl;
	cout << "MS_err = MS_cpp - MS_mat;" << endl;
	cout << "norm_MS_err = norm(MS_err)" << endl; //OK

	cout << "% With S = symmetric_matrix " << endl;
	cout << "S = " << (MATLAB) S << endl;
	ublasExtra::ixaxpy_prod(S, iax, Hr, iar);
	cout << "S_cpp = " << (MATLAB) S << endl;
	cout << "S_mat = H * S * H';" << endl;
	cout << "S_err = S_cpp - S_mat;" << endl;
	cout << "norm_S_err = norm(S_err)" << endl; //NOK

	cout << "% With SS = symmetric_adaptor<symmetric_matrix> " << endl;
	cout << "SS = " << (MATLAB) SS << endl;
	ublasExtra::ixaxpy_prod(SS, iax, Hr, iar);
	cout << "SS_cpp = " << (MATLAB) SS << endl;
	cout << "SS_mat = H * SS * H';" << endl;
	cout << "SS_err = SS_cpp - SS_mat;" << endl;
	cout << "norm_SS_err = norm(SS_err)" << endl; //OK

}

BOOST_AUTO_TEST_CASE( test_ixaxpy )
{
	test_ixaxpy02();
}

