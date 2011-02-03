/* $Id: $ */

#include <boost/test/auto_unit_test.hpp>

#include "jmath/random.hpp"
#include "kernel/jafarTestMacro.hpp"

using namespace jafar;

const double eps = 1e-10;

BOOST_AUTO_TEST_CASE( test_random )
{
	//!!! nizar 20110203 : leave it for when the average test is done
	//	double mean = 0.0;
	std::cout << "== test_random ========================" << std::endl 
	          << "=======================================" << std::endl;
	jmath::MultiDimUniformDistribution unidist(2, rand());
	for(int i = 0; i < 1000; ++i)
	{
		jblas::vec result = unidist.get();

		for(size_t j = 0; j < result.size(); ++j)
		{
			JFR_CHECK(result(j) >= 0.0-eps);
			JFR_CHECK(result(j) <= 1.0+eps);
		}
		
	}

	// TODO test at least that average = mean, and all constructors
	
}
