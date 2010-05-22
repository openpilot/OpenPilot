/**
 * \file test_display.cpp
 *
 *  Created on: 25/03/2010
 *     \author: croussil@laas.fr
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
#include "kernel/jafarTestMacro.hpp"

// rtslam includes
#include "rtslam/display_qt.hpp"
#include "rtslam/display_gdhe.hpp"
#include "rtslam/display_example.hpp"

#include <iostream>

using namespace jafar::rtslam;
using namespace std;



BOOST_AUTO_TEST_CASE( test_display )
{
	display::ViewerQt vqt;
	display::ViewerGdhe vgdhe;
	JFR_CHECK_EQUAL(vqt.id(), 0);
	JFR_CHECK_EQUAL(vgdhe.id(), 1);
	JFR_CHECK_EQUAL(display::ViewerAbstract::idFactory().countUsed(), 1);

	
	/*
	use :
	
	
	
	*/
}

