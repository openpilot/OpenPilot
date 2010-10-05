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
	int count = 0;
	
	#ifdef HAVE_MODULE_QDISPLAY
	display::ViewerQt vqt;
	JFR_CHECK_EQUAL(vqt.id(), count);
	count++;
	#endif
	
	#ifdef HAVE_MODULE_GDHE
	display::ViewerGdhe vgdhe;
	JFR_CHECK_EQUAL(vgdhe.id(), count);
	count++;
	#endif
	
	JFR_CHECK_EQUAL(display::ViewerAbstract::idFactory().countUsed(), count);

	
	/*
	use :
	
	
	
	*/
}

