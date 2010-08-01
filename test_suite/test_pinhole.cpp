/*
 * test_pinhole.cpp
 *
 *  Created on: 2010/08/01
 *      Author: croussil
 */

// boost unit test includes
#include <boost/test/auto_unit_test.hpp>

// jafar debug include
#include "kernel/jafarDebug.hpp"
#include "kernel/jafarTestMacro.hpp"

#include <iostream>
#include "jmath/random.hpp"
#include "jmath/ublasExtra.hpp"
#include "rtslam/sensorImageParameters.hpp"
#include "rtslam/pinholeTools.hpp"

using namespace jafar;
using namespace jafar::rtslam;

const unsigned IMG_WIDTH = 640;
const unsigned IMG_HEIGHT = 480;
const double INTRINSIC[4] = { 301.27013,   266.86136,   497.28243,   496.81116 };
const double DISTORTION[2] = { -0.23193,   0.11306 }; //{-0.27965, 0.20059, -0.14215}; //{-0.27572, 0.28827};


void test_ph01(void) {
#undef TEST_MACRO_EPSILON
#define TEST_MACRO_EPSILON 1e-4

	vec intrinsic = jmath::ublasExtra::createVector<4> (INTRINSIC);
	vec distortion = jmath::ublasExtra::createVector<sizeof(DISTORTION)/sizeof(double)> (DISTORTION);
	SensorImageParameters params;
	params.setImgSize(IMG_WIDTH, IMG_HEIGHT);
	params.setIntrinsicCalibration(intrinsic, distortion, distortion.size()*3);
	params.setMiscellaneous(1.0, 0.1);
	
	
	jblas::vec2 p, pe, pep, pepe;
	jblas::vec2 peu, peud, peudu, peudp;
	const int nsets = 6;
	double set[nsets][2] = {{20, 20}, {620, 20}, {620, 460}, {20, 460}, {310, 250}, {620.5, 20.5}};
	for(int i = 0; i < nsets; ++i)
	{
		p(0) = set[i][0]; p(1) = set[i][1];
		pe = pinhole::depixellizePoint(params.intrinsic, p);
		pep = pinhole::pixellizePoint(params.intrinsic, pe);
		JFR_CHECK_VEC_EQUAL(p, pep);
		pepe = pinhole::depixellizePoint(params.intrinsic, pep);
		JFR_CHECK_VEC_EQUAL(pe, pepe);
		
		peu = pinhole::undistortPoint(params.correction, pe);
		peud = pinhole::distortPoint(params.distortion, peu);
		JFR_CHECK_VEC_EQUAL(pe, peud);
		peudu = pinhole::undistortPoint(params.correction, peud);
		JFR_CHECK_VEC_EQUAL(peu, peudu);
		
		peudp = pinhole::pixellizePoint(params.intrinsic, peud);
		JFR_CHECK_VEC_EQUAL(p, peudp);
		
	}

}

BOOST_AUTO_TEST_CASE( test_pinhole )
{
	std::cout << "##### TEST PINHOLE #####" << std::endl;
	test_ph01();
}