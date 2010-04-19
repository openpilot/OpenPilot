/**
 * test_indirect_array.cpp
 *
 *  Created on: 27/02/2010
 *      Author: jsola
 *
 *  \file test_indirect_array.cpp
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
#include <boost/numeric/ublas/symmetric.hpp>
#include <boost/numeric/ublas/blas.hpp>
#include <boost/numeric/ublas/io.hpp>

#include "jmath/jblas.hpp"
#include "jmath/ublasExtra.hpp"
#include "jmath/indirectArray.hpp"
#include "jmath/matlab.hpp"
#include "jmath/random.hpp"

using namespace std;
using namespace jafar::jmath;

// TEST INDIRECT ARRAY OPERATION
void test_indirect_array01(void) {
	size_t N = 7;
	ublas::matrix<int> M(N, N);
	ublas::symmetric_matrix<int> S(N);
	ublas::vector<int> V(N);


	// initialize M and V
	cout << "M=" << endl;
	for (size_t i = 0; i < M.size1(); ++i) {
		V(i) = 111 * i;
		for (size_t j = 0; j < M.size2(); ++j) {
			M(i, j) = 10 * i + j;
			cout.width(2);
			cout.fill('0');
			cout << M(i, j) << "  ";

			S(i, j) = 10 * i + j;
		}
		cout << endl;
	}
	cout << "V=\n" << V << endl;


	//define an indirect_array
	const size_t n = 3;
	jblas::ind_array ia(n);
	ia(0) = 1;
	ia(1) = 2;
	ia(2) = 3;


	// operator <<
	cout << "ia : " << ia << endl;
	cout << "ia = " << (MATLAB) ia << endl;

	jblas::ind_array ib(n);
	ib(0) = 2;
	ib(1) = 3;
	ib(2) = 4;

	ublas::matrix<int> m(n, n);
	m(0, 0) = 100;
	m(0, 1) = 101;
	m(0, 2) = 102;
	m(1, 0) = 110;
	m(1, 1) = 111;
	m(1, 2) = 112;
	m(2, 0) = 120;
	m(2, 1) = 221;
	m(2, 2) = 322;

	ublas::symmetric_matrix<int> s(n, n);
	s(0, 0) = 100;
	s(1, 0) = 110;
	s(1, 1) = 111;
	s(2, 0) = 120;
	s(2, 1) = 221;
	s(2, 2) = 322;


	//hint: use jblas::ind_array::all() to access all indices
	jblas::ind_array a(N);


	// display array's data <--- does not work !
	//		cout << "ia=\n" << ia.data() << endl;
	//		cout << "a=\n" << a.all().data() << endl;

	cout << "\n% OPERATIONS ON REGULAR MATRICES" << endl;
	ublas::matrix_indirect<ublas::matrix<int> > Mall(M, a.all(), a.all());
	cout << "Mat, all=\n" << Mall << endl;


	// use ia to extract some indices
	ublas::matrix_indirect<ublas::matrix<int> > Mindirect(M, ia, ia);
	cout << "Mat, ind=\n" << Mindirect << endl;


	// or use ublas::project
	cout << "Mat, proj=\n" << ublas::project(M, ia, ia) << endl;


	// Project on a matrix
	project(M, ia, ib) = m;
	cout << "mat, projected to Mat=" << m << endl;
	cout << "Mat, updated" << M << endl;


	// Access to all the matrix
	cout << "\n% OPERATIONS ON SYMMETRIC MATRICES" << endl;
	ublas::matrix_indirect<ublas::symmetric_matrix<int> > Sall(S, ia.all(), a.all());
	cout << "Sym Mat, all=\n" << Sall << endl;

	ublas::matrix_indirect<ublas::symmetric_matrix<int> > Sind(S, ia, ia);
	cout << "Sym Mat, sym ind=\n" << Sind << endl;

	ublas::matrix_indirect<ublas::symmetric_matrix<int> > NSind(S, ia, ib);
	cout << "Sym Mat, non sym ind=\n" << NSind << endl;

	ublas::matrix<double> A(5, 5);
	for (size_t i = 0; i < 5; ++i)
		for (size_t j = 0; j < 5; ++j)
			A(i, j) = 0;
	//cout << "A = " << (MATLAB)A << endl;

	ublas::symmetric_adaptor<ublas::matrix<double> > Asym(A);
	//cout << "Asym = " << (MATLAB)Asym << endl;
	Asym(1, 0) = 1;
	//cout << "Asym = " << (MATLAB)Asym << endl;
	//cout << "A = " << (MATLAB)A << endl;
	Asym(0, 0) = -12;
	cout << "Asym = " << (MATLAB) Asym << endl;
	cout << "A = " << (MATLAB) A << endl;

	ublas::matrix<double> H(3, 5);
	randMatrix(H, 3, 5);
	cout << "H = " << (MATLAB) H << endl;
	for (size_t i = 0; i < 5; ++i)
		for (size_t j = 0; j < 5; ++j)
			Asym(i, j) = (rand() + 0.0) / RAND_MAX * 2 - 1;
	cout << "Asym = " << (MATLAB) Asym << endl;
}

void test_indirect_array02(void) {

	cout << "\n% INDIRECT ARRAY MANIPULATION\n%===============" << endl;
	jblas::ind_array ilong(5);
	ilong(0) = 4;
	ilong(1) = 1;
	ilong(2) = 20;
	ilong(3) = 3;
	ilong(4) = 0;
	cout << "ilong = " << ilong << endl;
	jblas::ind_array ihead(jafar::jmath::ublasExtra::ia_head(ilong, 3));
	cout << "ihead = " << ihead << endl;
	jblas::ind_array icomp(jafar::jmath::ublasExtra::ia_complement(ilong, ihead));
	cout << "icomp = " << icomp << endl;
	jblas::ind_array irest(jafar::jmath::ublasExtra::ia_union(icomp, ihead));
	cout << "irest = " << irest << endl;
	jblas::vecb ba(10);
	std::stringstream ssv;
	ssv << "[10](0,1,0,1,0,0,1,1,0,1)";
	ssv >> ba;
	cout << "ba    : " << ba << endl;
	jblas::ind_array iset = jafar::jmath::ublasExtra::ia_set(ba);
	cout << "iset  = " << iset << endl;
	jblas::ind_array irng = ublasExtra::ia_set(0, 4);
	cout << "irng  = " << irng << endl;
	ublas::range r(3, 6);
	jblas::ind_array irng2 = ublasExtra::ia_set(r);
	cout << "irng2 = " << irng2 << endl;
	jblas::ind_array ihdba = ublasExtra::ia_head(ba, 3);
	cout << "ihdba = " << ihdba << endl;

	ba.resize(20);
	ssv << "[20](0,1,0,1,0,0,1,1,0,1,1,1,0,0,1,1,1,0,0,1)";
	ssv >> ba;
	cout << "ba    : " << ba << endl;
	jblas::ind_array f3 = ublasExtra::ia_head(ba, 3);
	cout << "first3: " << f3 << endl;
	ublas::vector<bool> z(3);
	z.clear();
	ublas::project(ba, f3) = z;
	cout << "ba    : " << ba << endl;
	jblas::ind_array n3 = ublasExtra::ia_popfront(ba, 3);
	cout << "next 3: " << n3 << endl;
	cout << "ba    : " << ba << endl;

}

void test_indirect_array03() {
	jblas::ind_array ia = ublasExtra::ia_set(2, 5);
	jblas::vec V(10);
	randVector(V);

	jblas::vec_indirect v(V, ia);
	cout << "V         = " << V << endl;
	cout << "ia        = " << ia << endl;
	cout << "v         = " << v << endl;
	cout << "&V        = " << &V << endl;
	cout << "&v.data() = " << &v.data() << endl;
	cout << "v.data()[2] = " << v.data()[2] << endl;
	cout << "v.data().size() = " << v.data().size() << endl;

	jblas::vec_indirect w(v);
	cout << "w         = " << w << endl;
	cout << "&w.data() = " << &w.data() << endl;
	cout << "w.data()[2] = " << w.data()[2] << endl;
	cout << "w.data().size() = " << w.data().size() << endl;

	V(2) = 0;
	cout << "v.data()[2] = " << v.data()[2] << endl;
	cout << "w.data()[2] = " << w.data()[2] << endl;

	jblas::ind_array ia2(ia);
	cout << "ia2   = " << ia2 << endl;

}

void test_indirect_array04() {
	cout << "\n BOOST IND_ARRAY COMPOSITION\n%========================" << endl;
	jblas::ind_array ia_base = ublasExtra::ia_set(4, 8);
	jblas::ind_array ia_ptr = ublasExtra::ia_set(1, 3);
	cout << "ia_base = " << ia_base << endl;
	cout << "ia_ptr = " << ia_ptr << endl;
	// try one among these 2:
	cout << "% ia_comp  = ia_base.compose(ia_equal);" << endl;
	jblas::ind_array ia_comp = ia_base.compose(ia_ptr);
	//	jblas::ind_array ia_comp  = ia_ptr.compose(ia_base);
	cout << "ia_comp = " << ia_comp << endl;
	cout << "We got ==> ia_comp(i) = ia_base(ia_ptr(i)) " << endl;
	JFR_CHECK_EQUAL( ia_comp(1) , ia_base(ia_ptr(1)) );
}

BOOST_AUTO_TEST_CASE( test_indirect_array )
{
	//	test_indirect_array01();
	//	test_indirect_array02();
	//	test_indirect_array03();
	test_indirect_array04();
}

