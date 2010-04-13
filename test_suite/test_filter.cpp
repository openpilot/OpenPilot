/**
 * test_filter.cpp
 *
 * \date 05/03/2010
 * \author jsola@laas.fr
 *
 *  \file test_filter.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

// boost unit test includes
#include <boost/test/auto_unit_test.hpp>

// jafar debug include
#include "kernel/jafarDebug.hpp"


#include "rtslam/kalmanFilter.hpp"
#include <iostream>
#include "jmath/matlab.hpp"
#include "jmath/random.hpp"
#include "jmath/indirectArray.hpp"

void test_filter01(void) {

	using namespace jafar::rtslam;
	using namespace jafar::jmath;
	using namespace std;

	size_t max_size = 5;
	size_t used_size = 4;
	size_t motion_size = 3;
	size_t pert_size = 2;
	ExtendedKalmanFilterIndirect filter(max_size);
	randVector(filter.x());
	randMatrix(filter.P());
	jblas::mat F_v(motion_size, motion_size);
	randMatrix(F_v);
	jblas::sym_mat U(pert_size);
	randMatrix(U);
	jblas::mat F_u(motion_size, pert_size);
	randMatrix(F_u);
	jblas::ind_array iax(used_size);
	iax(0) = 0;
	iax(1) = 1;
	iax(2) = 2;
	iax(3) = 3;//iax(4)=4;iax(5)=5;
	jblas::ind_array iav(motion_size);
	iav(0) = 0;
	iav(1) = 1;
	iav(2) = 2;//iav(3)=3;

	cout << "used_size = " << used_size << ";" << endl;
	cout << "iax = " << (MATLAB) iax << endl;
	cout << "iav = " << (MATLAB) iav << endl;
	cout << "P =  " << (MATLAB) filter.P() << endl;
	cout << "F_v = " << (MATLAB) F_v << endl;
	cout << "F_u = " << (MATLAB) F_u << endl;
	cout << "U = " << (MATLAB) U << endl;
	filter.predict(iax, F_v, iav, F_u, U);
	cout << "Po =  " << (MATLAB) filter.P() << endl;
	cout << "F = eye(used_size); F(iav,iav) = F_v" << endl;
	cout << "Po_mat = P; Po_mat(iax,iax) = F*P(iax,iax)*F';" << endl;
	cout << "Q = F_u*U*F_u';" << endl;
	cout << "Po_mat(iav,iav) = Po_mat(iav,iav) + Q" << endl;
//	cout << "Po_err = Po(iax,iax) - Po_mat(iax,iax);" << endl;
	cout << "Po_err = Po - Po_mat;" << endl;
	cout << "norm(Po_err,'fro')" << endl;

}


BOOST_AUTO_TEST_CASE( test_filter )
{
	test_filter01();
}

