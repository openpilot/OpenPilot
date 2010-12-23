


// jafar debug include
#include "kernel/jafarDebug.hpp"
#include "kernel/jafarTestMacro.hpp"

#include <iostream>

#include "qdisplay/init.hpp"
#include "qdisplay/imout.hpp"



using namespace jafar;

void display(void *) {
	image::Image img;
	img.load(std::string("data/qt.png"), 0);

	qdisplay::imout << img << qdisplay::flush;
	
	qdisplay::qtSleep(2000);
	
	qdisplay::imout << qdisplay::clear;
	qdisplay::imout << img << img << qdisplay::endl << img << qdisplay::endl << qdisplay::vsep;
	qdisplay::imout << img << img << qdisplay::hsep << img << qdisplay::flush;
	
}

int main()
{
	qdisplay::QtAppStart((qdisplay::FUNC)&display);
}

