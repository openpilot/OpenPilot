/* $Id$ */

// boost unit test includes
#define BOOST_TEST_MAIN 
#define BOOST_TEST_DYN_LINK 
#include <boost/test/auto_unit_test.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

// jafar debug include
#include "kernel/jafarDebug.hpp"

// include here your defined test suite
#include "correl/explorer.hpp"
using namespace jafar;
using namespace jafar::correl;

BOOST_AUTO_TEST_CASE( dummy )
{
	image::Image *im1 = new image::Image(10, 10, IPL_DEPTH_8U, JfrImage_CS_GRAY);
	image::Image *im2 = new image::Image(10, 10, IPL_DEPTH_8U, JfrImage_CS_GRAY);
	float xres, yres;
	Explorer<Zncc>::exploreTranslation(im1, im2, 0, 10, 1, 0, 10, 1, xres, yres, NULL);
	Explorer<Zncc>::exploreRotation(im1, im2, 0, 360, 10, xres, NULL);
}


/*
 * standard init_unit_test_suite functione
 */

test_suite*
init_unit_test_suite( int, char* [] ) {

  // we set the debug level to Warning
  jafar::debug::DebugStream::setDefaultLevel(jafar::debug::DebugStream::Warning);

  return 0;
}

