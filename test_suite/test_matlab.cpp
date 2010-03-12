/**
 * test_matlab.cpp
 *
 *  Created on: 11/03/2010
 *      Author: jsola
 *
 *  \file test_matlab.cpp
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

#include "jmath/jblas.hpp"
#include "jmath/matlab.hpp"
#include "jmath/random.hpp"

using namespace std;
using namespace jafar::jmath;

void test_matlab01(void) {

	jblas::ind_array i(2);
	i(0) = 2;
	i(1) = 0;
	ublas::range r(1, 3);
	ublas::slice s(0, 2, 3);
	jblas::vec v(8);
	jblas::vec8 vb;
	randVector(v);
	randVector(vb);
	jblas::vec_range vr(v, r);
	jblas::vec_slice vs(v, s);
	jblas::vec_indirect vi(v, i);

	cout << "%\n Indirect arrays, ranges and slices \n%=================================" << endl;
	cout << "i   = " << (MATLAB) i << endl;
	cout << "r   = " << (MATLAB) r << endl;
	cout << "s   = " << (MATLAB) s << endl;
	cout << "v   = " << (MATLAB) v << endl;
	cout << "vi  = " << (MATLAB) vi << endl;
	cout << "vr  = " << (MATLAB) vr << endl;
	cout << "vs  = " << (MATLAB) vs << endl;

	cout << "vb  = " << (MATLAB) vb << endl;
	cout << "vbi = " << (MATLAB) ublas::project(vb, i) << endl;
	cout << "vbr = " << (MATLAB) ublas::project(vb, r) << endl;
	cout << "vbs = " << (MATLAB) ublas::project(vb, s) << endl;

	jblas::mat M(5, 5);
	randMatrix(M);
	jblas::mat_indirect Mi(M, i, i);
	jblas::mat_range Mr(M, r, r);
	jblas::mat_slice Ms(M, s, s);
	cout << "M   = " << (MATLAB) M << endl;
	cout << "Mi  = " << (MATLAB) Mi << endl;
	cout << "Mr  = " << (MATLAB) Mr << endl;
	cout << "Ms  = " << (MATLAB) Ms << endl;

}

void test_matlab02(void) {

	jblas::mat M(5, 5);
	cout << "\n% Boost serializer for 1xN and Nx1 matrices \n% =================================" << endl;
	randMatrix(M, 1, 3);
	cout << "M  = " << M << endl;
	randMatrix(M, 3, 1);
	cout << "M  = " << M << endl;
}

void test_matlab03(void) {

	ifstream file;

	cout << "\n% MATLAB TEST READ-WRITE AND CHECK \n% =================================" << endl;
	// Here we just initialize a vector.
	// We will see if Matlab renders a good result by comparing C++ and Matlab outputs in boost format.
	jblas::vec V;
	randVector(V, 4);

	file.open("/Users/jsola/jafar/modules/jmath/test_suite/test_matlab_V.boost");
	if (file.good()) { //open file succeeded

		jblas::vec V_mat; // Here we will store the Matlab result
		file >> V_mat; //read from file it
//		cout << V_mat; //result
		file.close(); //close it

		JFR_CHECK_VEC_EQUAL(V,V_mat);
	}
	else { // open file failed
		cout << "%vvvvvvv COPY AND EXECUTE IN MATLAB vvvvvv" << endl;
		cout << "%vvvvvvvvvvvvvvv FROM HERE vvvvvvvvvvvvvvv\n" << endl;
		cout << "slamrc; clear;" << endl;
		cout << "V = " << (MATLAB) V << endl;
		cout << "BoostFileWrite(V,'test_matlab_V','/Users/jsola/jafar/modules/jmath/test_suite/')" << endl;
		cout << "\n%^^^^^^^^^^^^^^^^ TO HERE ^^^^^^^^^^^^^^^^" << endl;
	}

}

BOOST_AUTO_TEST_CASE( test_matlab )
{
//	test_matlab01();
//	test_matlab02();
	test_matlab03();
}

