


// jafar debug include
#include "kernel/jafarDebug.hpp"
#include "kernel/jafarTestMacro.hpp"

#include <iostream>

#include "qdisplay/init.hpp"
#include "qdisplay/imout.hpp"



using namespace jafar;

void display_cv() {
	image::Image img;
	img.load(std::string("data/qt.png"), 0);
	
	image::imout << img << image::flush;
	
	cv::waitKey(2000);
	
	image::imout << image::clear;
	image::imout << img << img << image::endl << img << image::endl << image::vsep;
	image::imout << img << img << image::hsep << img << image::flush;
	
	cv::waitKey(2000);
}

void display_qt(void *) {
	image::Image img;
	img.load(std::string("data/qt.png"), 0);

	qdisplay::imout << img << image::flush;
	
	qdisplay::qtSleep(2000);
	
	qdisplay::imout << image::clear;
	qdisplay::imout << img << img << image::endl << img << image::endl << image::vsep;
	qdisplay::imout << img << img << image::hsep << img << image::flush;
	
	qdisplay::qtSleep(2000);
	
	display_cv();
}


int main()
{
	qdisplay::QtAppStart((qdisplay::FUNC)&display_qt);
	
	//sleep(4);
	//display_cv();

	
}

