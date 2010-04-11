/**
 * \file test_acsearch.cpp
 *
 *  Created on: 10/04/2010
 *     \author: jsola@laas.fr
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

#include <iostream>
#include "jmath/matlab.hpp"
#include "rtslam/activeSearch.hpp"

using namespace std;
using namespace jafar;
using namespace jafar::rtslam;
using namespace jblas;
using namespace jafar::jmath;

void test_acsearch01() {
	cout << "\n% ACTIVE SEARCH GRID \n%======================\n" << endl;
	ActiveSearchGrid grid(320, 240, 5, 4);
	vec2 pix;
	ROI roi;
	int i = 0;
	while (true) {
		if (grid.pickEmptyROI(roi)) {
			pix = (roi.upleft() + roi.downright()) / 2;
			grid.addPixel(pix);
			cout << "pix{" << i << "}: " << pix << " , " << grid << endl;
			i ++;
		}
		else
			break;
	}
}

BOOST_AUTO_TEST_CASE( test_acsearch )
{
	test_acsearch01();
}
