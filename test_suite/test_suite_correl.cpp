/* $Id$ */

// boost unit test includes
#define BOOST_TEST_MAIN 
#define BOOST_TEST_DYN_LINK 
#include <boost/test/auto_unit_test.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

// jafar debug include
#include "kernel/jafarDebug.hpp"

#include <iostream>
#include <fstream>

// include here your defined test suite
#include "correl/explorer.hpp"
using namespace jafar;
using namespace jafar::correl;


		void trackPoint(char* filePattern, int fileNBegin, int fileNEnd, int x, int y, int winHalfSize, int searchHalfW, int searchHalfH )
		{
			std::ofstream logfile("track.dat");
			logfile << "# Correl.trackPoint" << std::endl;
			
			double currentX = x;
			double currentY = y;

			image::Image *im1 = NULL, *im2 = NULL;
			char imgName[256];
			
			for (int i = fileNBegin; i <= fileNEnd; ++i)
			{
				sprintf(imgName, filePattern, i);
				std::cout << imgName << ": tracking from " << x << ", " << y;;
				
				if (i != fileNBegin)
				{
					delete im1;
					im1 = im2;
					im1->setROI(x-winHalfSize,y-winHalfSize,2*winHalfSize+1,2*winHalfSize+1);
				}
				im2 = new image::Image();
				im2->load(imgName);
				
				if (i != fileNBegin)
				{
					correl::Explorer<correl::Zncc>::exploreTranslation(*im1, *im2, x-searchHalfW, x+searchHalfW, 1, y-searchHalfH, y+searchHalfH, 1, currentX, currentY);
					x = currentX+0.5;
					y = currentY+0.5;
					std::cout << " to " << currentX << ", " << currentY << std::endl;
					
				}
				
				logfile << i << "\t" << currentX << "\t" << currentY << std::endl;
			}
			
			logfile.close();
		}

		void trackPointRef(char* filePattern, int fileNBegin, int fileNEnd, int x, int y, int winHalfSize, int searchHalfW, int searchHalfH )
		{
			std::ofstream logfile("track.dat");
			logfile << "# Correl.trackPoint" << std::endl;
			
			double currentX = x;
			double currentY = y;

			image::Image *im1 = new image::Image(), *im2 = new image::Image();
			char imgName[256];
			
			for (int i = fileNBegin; i <= fileNEnd; ++i)
			{
				sprintf(imgName, filePattern, i);
				std::cout << imgName << ": tracking from " << x << ", " << y;;
				
				if (i == fileNBegin)
				{
					im1->load(imgName);
					im1->setROI(x-winHalfSize,y-winHalfSize,2*winHalfSize+1,2*winHalfSize+1);
				} else
				{
					im2->load(imgName);
					correl::Explorer<correl::Zncc>::exploreTranslation(*im1, *im2, x-searchHalfW, x+searchHalfW, 1, y-searchHalfH, y+searchHalfH, 1, currentX, currentY);
					x = currentX+0.5;
					y = currentY+0.5;
					std::cout << " to " << currentX << ", " << currentY << std::endl;
				}
				
				logfile << i << "\t" << currentX << "\t" << currentY << std::endl;
			}
			
			logfile.close();
		}


BOOST_AUTO_TEST_CASE( dummy )
{
	image::Image im1(6, 5, CV_8U, JfrImage_CS_GRAY);
	image::Image im2(6, 5, CV_8U, JfrImage_CS_GRAY);
	double xres, yres;

	Zncc::compute(im1, im2, NULL);
	
	im1.setROI(1,2,5,3);
	im2.setROI(1,0,5,3);
	Zncc::compute(im1, im2, NULL);

	im2.resetROI();
	Explorer<Zncc>::exploreTranslation(im1, im2, 1, 5, 1, 0, 4, 1, xres, yres, NULL);
	
//	trackPointRef("/net/pelican/data1/robots/dala/data/tests/2010-04-23_gyro-study/serie02/preproc2/img.l.%04d.png", 0, 235, 221, 220, 10, 80, 20);
//	trackPointRef("/net/pelican/data1/robots/dala/data/tests/2010-04-23_gyro-study/serie02/preproc4/img.l.%04d.png", 0, 235, 110, 110, 5, 40, 10);
//	trackPointRef("/net/pelican/data1/robots/dala/data/tests/2010-04-23_gyro-study/serie04/preproc1/img.l.%04d.png", 0, 528, 548, 439, 5, 40, 10);

	//Explorer<Zncc>::exploreRotation(im1, im2, 0, 360, 10, xres, NULL);
}



/*
 * standard init_unit_test_suite functione
 */

test_suite*
init_unit_test_suite( int, char* [] ) {

  // we set the debug level to Warning
  jafar::debug::DebugStream::setDefaultLevel(jafar::debug::DebugStream::Warning);

  return 0;
}

