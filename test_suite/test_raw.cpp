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
#include "image/Image.hpp"
#include "qdisplay/ImageView.hpp"
#include "qdisplay/Viewer.hpp"
#include "qdisplay/Shape.hpp"

using namespace jafar::rtslam;
using namespace std;
using namespace jafar::image;

void test_raw01(void) {
	cout << "\n% TEST OF RAW STRUCTURE\n% ==============" << endl;
	RawImageSimu    imgSimu  ;
	cout << imgSimu << endl ;

	jafar::image::Image *img = Image::loadImage("/home/jeanmarie/Images/day_picture/afgan.jpg", 1) ;
	const Image i = img->clone();

	//	jafar::image::Image &i = &img ;
//	jafar::qdisplay::ImageView iv(i) ;
//	iv.setImage(&i) ;
	jafar::qdisplay::Viewer viewer = new jafar::qdisplay::Viewer() ;
	jafar::qdisplay::Shape shape = Qdisplay::Shape.new(Qdisplay::Shape::ShapeRectangle, 3,4 ,10,15) ;
	viewer.addShape(shape) ;


}

BOOST_AUTO_TEST_CASE( test_raw )
{
	test_raw01();
}

