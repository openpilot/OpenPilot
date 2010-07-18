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

#include "rtslam/rawImage.hpp"
//#include "rtslam/FeaturePoint.hpp"

#include <iostream>
#include "image/Image.hpp"
#include "qdisplay/init.hpp"
#include "qdisplay/ImageView.hpp"
#include "qdisplay/Viewer.hpp"
#include "qdisplay/Shape.hpp"

using namespace jafar::rtslam;
using namespace std;
using namespace jafar;

qdisplay::Viewer *viewer;

void test_raw01_display(void*)
{
		image::Image img;
		img.load("data/imageSample.ppm");
		qdisplay::ImageView *view = new qdisplay::ImageView;
		viewer = new qdisplay::Viewer;
		viewer->setImageView(view, 0, 0);
		
		qdisplay::Shape *s = new qdisplay::Shape(qdisplay::Shape::ShapeRectangle, 23,24 ,10,15);
		s->setVisible(true);
		s->setColor(255, 0, 0); // red
		view->addShape(s);
	
		view->setImage(img);
		viewer->resize(400,300);
}

void test_raw01_main(void*)
{
	sleep(2);
	delete viewer;
}

void test_raw01(void) {
	cout << "\n% TEST OF RAW STRUCTURE\n% ==============" << endl;
	RawImage    imgSimu  ;
	cout << imgSimu << endl ;

	qdisplay::QtAppStart((qdisplay::FUNC)&test_raw01_display,0,(qdisplay::FUNC)&test_raw01_main,0,0,NULL);


}

BOOST_AUTO_TEST_CASE( test_raw )
{
	test_raw01();
}

