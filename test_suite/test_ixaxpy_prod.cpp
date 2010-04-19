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
using namespace jblas;

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
	jblas::mat J;
	randMatrix(J, n, n);
	cout << "J = " << (MATLAB) J << endl;


	// Here we print the Matlab operations for generating the full Jacobian
	cout << "H = eye(" << N << "); H(iar,iar) = J" << endl;


	// Here we perform P = [J 0 ; 0 I] * P * [J 0 ; 0 I]'
	// in the optimized fashion: P = [J*Srr*J' J*Srm ; XXXXX Smm]
	// where XXXX is not computed because P is symmetrical,
	// and Smm neither because it's a copy of the original.

	ind_array ia_inv = ublasExtra::ia_complement(iax, iar);
	cout << "ia_inv = " << (MATLAB) ia_inv << endl;

	cout << "% With S = symmetric_matrix " << endl;
	cout << "S = " << (MATLAB) S << endl;
	ublasExtra::ixaxpy_prod(S, ia_inv, J, iar, iar);
	cout << "S_cpp = " << (MATLAB) S << endl;
	cout << "S_mat = H * S * H';" << endl;
	cout << "S_err = S_cpp - S_mat;" << endl;
	cout << "norm_S_err = norm(S_err)" << endl; //NOK

}

void test_ixaxpy03(void) {

	cout << "\n% TEST FOR IXAXPY\n% ===========" << endl;
	size_t s = 6; // size of S(s,s)
	size_t N = 5; // size of P(N,N)
	size_t n = 2; // size of OUT_in(m,n)
	size_t m = 1; // size of OUT_in(m,n)

	//	symmetric_matrix<double> S(s,s);
	jblas::sym_mat S(s, s);
	randMatrix(S);

	jblas::ind_array ia_x(N);
	for (size_t i = 0; i < N; i++)
		ia_x(i) = i;
	cout << "ia_x = " << (MATLAB) ia_x << endl;

	jblas::ind_array ia_in(n);
	ia_in(0) = 2;
	ia_in(1) = 0;
	cout << "ia_in = " << (MATLAB) ia_in << endl;

	cout << "% With S = symmetric_matrix." << endl;
	cout << "% With output not in the original map range." << endl;

	jblas::ind_array ia_out(m);
	//	ia_out(0) = 3; // this one inside P
	ia_out(0) = 5; // this one outside P
	cout << "ia_out = " << (MATLAB) ia_out << endl;


	// the jacobian
	jblas::mat OUT_in;
	randMatrix(OUT_in, m, n);
	cout << "OUT_in = " << (MATLAB) OUT_in << endl;


	// Here we print the Matlab operations for generating the full Jacobian
	cout << "J = zeros(" << s << "); J(ia_x,ia_x) = eye(" << N << "); J(ia_out,ia_out) = 0; J(ia_out,ia_in) = OUT_in" << endl;

	// Here we perform P = [OUT_in 0 ; 0 I] * P * [OUT_in 0 ; 0 I]'
	// in the optimized fashion: P = [OUT_in*Srr*OUT_in' OUT_in*Srm ; XXXXX Smm]
	// where XXXX is not computed because P is symmetrical,
	// and Smm neither because it's a copy of the original.

	ind_array ia_inv = ublasExtra::ia_complement(ia_x, ia_in);
	cout << "ia_inv = " << (MATLAB) ia_inv << endl;

	cout << "% Without added covariance." << endl;
	cout << "S = " << (MATLAB) S << endl;
	ublasExtra::ixaxpy_prod(S, ia_inv, OUT_in, ia_in, ia_out);
	cout << "S_cpp = " << (MATLAB) S << endl;
	cout << "S_mat = J * S * J';" << endl;
	cout << "ia_res = " << (MATLAB) ublasExtra::ia_union(ia_inv, ia_out) << endl;
	cout << "S_err = S_cpp(ia_res,ia_res) - S_mat(ia_res,ia_res);" << endl;
	cout << "norm_S_err = norm(S_err)" << endl;


	cout << "% With output not in the original map range." << endl;
	cout << "% With output not in the input range." << endl;
	ia_out(0) = 4;
	cout << "S = " << (MATLAB) S << endl;
	cout << "ia_out = " << (MATLAB) ia_out << endl;
	cout << "J = zeros(" << s << "); J(ia_x,ia_x) = eye(" << N << "); J(ia_out,ia_out) = 0; J(ia_out,ia_in) = OUT_in" << endl;
	ublasExtra::ixaxpy_prod(S, ia_inv, OUT_in, ia_in, ia_out);
	cout << "S_cpp = " << (MATLAB) S << endl;
	cout << "S_mat = J * S * J';" << endl;
	cout << "ia_res = " << (MATLAB) ublasExtra::ia_union(ia_inv, ia_out) << endl;
	cout << "S_err = S_cpp(ia_res,ia_res) - S_mat(ia_res,ia_res);" << endl;
	cout << "norm_S_err = norm(S_err)" << endl;

	cout << "% With output not in the original map range." << endl;
	cout << "% With output in the input range." << endl;
	ia_out(0) = 2;
	cout << "S = " << (MATLAB) S << endl;
	cout << "ia_out = " << (MATLAB) ia_out << endl;
	cout << "J = zeros(" << s << "); J(ia_x,ia_x) = eye(" << N << "); J(ia_out,ia_out) = 0; J(ia_out,ia_in) = OUT_in" << endl;
	ublasExtra::ixaxpy_prod(S, ia_inv, OUT_in, ia_in, ia_out);
	cout << "S_cpp = " << (MATLAB) S << endl;
	cout << "S_mat = J * S * J';" << endl;
	cout << "ia_res = " << (MATLAB) ublasExtra::ia_union(ia_inv, ia_out) << endl;
	cout << "S_err = S_cpp(ia_res,ia_res) - S_mat(ia_res,ia_res)" << endl;
	cout << "norm_S_err = norm(S_err)" << endl;

}

BOOST_AUTO_TEST_CASE( test_ixaxpy )
{
//	test_ixaxpy02();
	test_ixaxpy03();
}

