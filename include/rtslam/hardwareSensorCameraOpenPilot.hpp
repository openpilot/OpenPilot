/**
 * \file hardwareSensorCameraOpenPilot.hpp
 *
 * Header file for getting data from an OpenPilot video source
 *
 * \date 9/10/2012
 * \author corvuscorax
 *
 * \ingroup rtslam
 */

#ifndef HARDWARE_SENSOR_CAMERA_OPENPILOT_HPP_
#define HARDWARE_SENSOR_CAMERA_OPENPILOT_HPP_

#include <jafarConfig.h>

#include "rtslam/hardwareSensorCamera.hpp"
#include "rtslam/rawImage.hpp"
#include "opencv/highgui.h"

namespace jafar {
namespace rtslam {
namespace hardware {

/**
This class allows to get images from openpilot. OpenPilot will "push" the images into the hardware sensor buffer
whenever they are captured
*/
class HardwareSensorCameraOpenPilot: public HardwareSensorCamera
{
	private:
		
		double realFreq;
		double last_timestamp;

		int mode;
		
	
		void init(int mode, std::string dump_path, cv::Size imgSize);
	public:
		
		/**
		@param camera_id the Firewire camera id (0x....)
		@param hwmode the hardware mode
		@param mode 0 = normal, 1 = dump used images, 2 = from dumped images
		@param dump_path the path where the images are saved/read... Use a ram disk !!!
		*/
		HardwareSensorCameraOpenPilot(kernel::VariableCondition<int> &condition, int bufferSize, cv::Size size, int mode = 0, std::string dump_path = ".");
		
		/**
		Same as before but assumes that mode=2, and doesn't need a camera
		*/
		HardwareSensorCameraOpenPilot(kernel::VariableCondition<int> &condition, cv::Size imgSize, std::string dump_path = ".");
		
		~HardwareSensorCameraOpenPilot();

		
		virtual void start();
		virtual double getLastTimestamp() { boost::unique_lock<boost::mutex> l(mutex_data); return last_timestamp; }
		double getFreq() { return realFreq; }

		void capture(IplImage* Image);
	
};

typedef boost::shared_ptr<HardwareSensorCameraOpenPilot> hardware_sensor_openpilot_ptr_t;

}}}

#endif
