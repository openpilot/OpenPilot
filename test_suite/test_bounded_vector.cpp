/**
 * test_bounded_vector.cpp
 *
 *  Created on: 27/02/2010
 *      Author: jsola
 *
 *  \file test_bounded_vector.cpp
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
#include "jmath/jblas.hpp"
#include "jmath/matlab.hpp"

void test_bounded_vector_in_matlab01(void) { // Test to check if bounded_vector is accepted by (MATLAB) cast.

	using namespace std;
	using namespace jafar::jmath;

	jblas::vec3 bv;
	cout << bv.size() << endl;
	bv(0) = 0;
	bv(1) = 1;
	bv(2) = 2;
	jblas::vec v(bv);
	cout << v.size() << endl;
	//		v(0) = 0; v(1)  = 1; v(2) = 2;
	cout << "v = " << (MATLAB) v << endl;
	cout << "bv = " << (MATLAB) bv << endl;
}




BOOST_AUTO_TEST_CASE( test_bounded_vector )
{
	test_bounded_vector_in_matlab01();
}


