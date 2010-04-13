/**
 * \file test_raw.cpp
 *
 * \date 1/04/2010
 * \author jmcodol@laas.fr
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

#include "rtslam/rawImageSimu.hpp"
//#include "rtslam/FeaturePoint.hpp"

#include <iostream>

using namespace jafar::rtslam;
using namespace std;

void test_raw01(void) {
	cout << "\n% TEST OF RAW STRUCTURE\n% ==============" << endl;
	RawImageSimu    imgSimu  ;
	cout << imgSimu << endl ;
//	FeaturePoint fp1   ;
//	imgSimu.addFeature( fp1 ) ;

}

BOOST_AUTO_TEST_CASE( test_raw )
{
	test_raw01();
}

