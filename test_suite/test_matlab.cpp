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

#include <iostream>
//#include <boost/numeric/ublas/io.hpp>

#include "jmath/jblas.hpp"
#include "jmath/matlab.hpp"
#include "jmath/random.hpp"

using namespace std;
using namespace jafar::jmath;


void test_matlab01(void){

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

	jblas::mat M(5,5);
	randMatrix(M);
	jblas::mat_indirect Mi(M, i, i);
	jblas::mat_range Mr(M, r, r);
	jblas::mat_slice Ms(M, s, s);
	cout << "M   = " << (MATLAB) M << endl;
	cout << "Mi  = " << (MATLAB) Mi << endl;
	cout << "Mr  = " << (MATLAB) Mr << endl;
	cout << "Ms  = " << (MATLAB) Ms << endl;

}


BOOST_AUTO_TEST_CASE( test_matlab )
{
	test_matlab01();
}

