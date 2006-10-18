/* $Id$ */

// boost unit test includes
#include <boost/test/unit_test.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

// jafar debug include
#include "kernel/jafarDebug.hpp"

// include here your defined test suite
//#include "test_suite_MyClass.hpp"


//using namespace jafar::qdisplay;


/*
 * standard init_unit_test_suite functione
 */

test_suite*
init_unit_test_suite( int, char* [] ) {

  // we set the debug level to Warning
  jafar::debug::DebugStream::setDefaultLevel(jafar::debug::DebugStream::Warning);

  // module qdisplay test suite
  test_suite* test= BOOST_TEST_SUITE( "qdisplay module test suite" );

  // add here your test suite to the module test suite
  //test->add( new test_suite_MyClass() );

  return test;
}

