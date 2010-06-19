/**
 * sensorPinHole.cpp
 *
 * \date 10/03/2010
 * \author jsola@laas.fr
 *
 *  \file sensorPinHole.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/pinholeTools.hpp"
#include "rtslam/sensorPinHole.hpp"
#include "rtslam/rawImage.hpp"

#include "image/Image.hpp"

#include "qdisplay/Viewer.hpp"
#include "qdisplay/ViewerManager.hpp"
#include "qdisplay/ImageView.hpp"

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
				return hardwareSensorPtr->acquireRaw(rawPtr);
			} else
			{
				jafarImage_ptr_t i(Image::loadImage("test_suite/imageSample.ppm",0));

				RawImage* imgRawPtr = new RawImage;
				imgRawPtr->setJafarImage(i) ;

				rawPtr.reset(imgRawPtr);
				return 0;
			}
		}

		void SensorPinHole::releaseRaw(){

			if (hardwareSensorPtr)
			{
				hardwareSensorPtr->releaseRaw();
			}
		}
		
		raw_ptr_t SensorPinHole::getRaw() {
//			image_ptr_t ip = rawPtr;
			return rawPtr;
		}

	}
}

