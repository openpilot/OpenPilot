/* $Id: $ */

#include <boost/test/auto_unit_test.hpp>

#include "jmath/ublasExtra.hpp"
#include "kernel/jafarTestMacro.hpp"

BOOST_AUTO_TEST_CASE( test_case_ublas_extra )
{
  jblas::mat22 m;
  m(0,0) = 2; m(1,0) = 1; m(0,1) = 1; m(1,1) = 2;
  jblas::vec2 v;
  v(0) = 3;
  v(1) = 1;
  JFR_CHECK_VEC_EQUAL( jafar::jmath::ublasExtra::eigenValue( m ) ,v);
}
