/* $Id$ */

// boost unit test includes
#include <boost/test/unit_test.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

// jafar debug include
#include "kernel/jafarDebug.hpp"

// include here your defined test suite
//#include "test_suite_HelloWorld.cpp"


//using namespace jafar::jmath;


/*
 * standard init_unit_test_suite function
 */

test_suite*
init_unit_test_suite( int, char* [] ) {

  // we set the debug level to Warning
  jafar::debug::DebugStream::setLevel(DebugStream::Warning);

  // module helloworld test suite
  test_suite* test= BOOST_TEST_SUITE( "jmath module test suite" );

  // add here your test suite to the module test suite
  //test->add( new test_suite_HelloWorld() );

  return test;
}

