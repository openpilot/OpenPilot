/**
 * \file sensorPinHole.cpp
 * \date 10/03/2010
 * \author jsola
 * \ingroup rtslam
 */

#include "rtslam/pinholeTools.hpp"
#include "rtslam/sensorPinHole.hpp"
#include "rtslam/rawImage.hpp"

#include "image/Image.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;
		using namespace jafar::image;


		///////////////////////////////////////
		// Class sensor pin hole
		///////////////////////////////////////

		SensorPinHole::SensorPinHole(const robot_ptr_t & _robPtr, filtered_obj_t inFilter) :
			SensorAbstract(_robPtr, inFilter) {
			type = PINHOLE ;
		}

		SensorPinHole::SensorPinHole(const simulation_t dummy, const robot_ptr_t & _robPtr) :
				SensorAbstract(ObjectAbstract::FOR_SIMULATION, _robPtr) {
			type = PINHOLE;
		}


		int SensorPinHole::acquireRaw(){

			if (hardwareSensorPtr)
			{
				int res = hardwareSensorPtr->acquireRaw(rawPtr);
				if (res >= 0) rawCounter++;
				return res;
			} else
			{
				static jafarImage_ptr_t i(Image::loadImage("data/imageSample.ppm",0));
				if (i->data() == NULL) throw "IMAGE_NOT_FOUND";
				RawImage* imgRawPtr = new RawImage;
				imgRawPtr->setJafarImage(i) ;
				struct timeval tv; struct timezone tz; gettimeofday(&tv,&tz);
				imgRawPtr->timestamp = tv.tv_sec + tv.tv_usec*1e-6; 
				
				rawPtr.reset(imgRawPtr);
				rawCounter++;
				return 0;
			}
		}

		raw_ptr_t SensorPinHole::getRaw() {
//			image_ptr_t ip = rawPtr;
			return rawPtr;
		}

	}
}

