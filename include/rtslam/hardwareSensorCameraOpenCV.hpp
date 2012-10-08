/**
 * \file hardwareSensorCameraOpenCV.hpp
 *
 * Header file for getting data from an OpenCV video source
 *
 * \date 18/09/2012
 * \author corvuscorax
 *
 * \ingroup rtslam
 */

#ifndef HARDWARE_SENSOR_CAMERA_OPENCV_HPP_
#define HARDWARE_SENSOR_CAMERA_OPENCV_HPP_

#include <jafarConfig.h>

#include "rtslam/hardwareSensorCamera.hpp"
#include "rtslam/rawImage.hpp"
#include "opencv/highgui.h"

namespace jafar {
namespace rtslam {
namespace hardware {

/**
This class allows to get images from firewire with non blocking procedure,
using triple-buffering.
*/
class HardwareSensorCameraOpenCV: public HardwareSensorCamera
{
	private:
		
		double realFreq;
		double last_timestamp;

		cv::VideoCapture *VideoSource;
		
		int mode;
		
		void preloadTask(void);
	
	
		void init(int mode, std::string dump_path, cv::Size imgSize);
		void init(int camera_id, int mode, std::string dump_path, cv::Size imgSize);
		void init(std::string videofile, int mode, std::string dump_path, cv::Size imgSize);
	public:
		
		/**
		@param camera_id the Firewire camera id (0x....)
		@param hwmode the hardware mode
		@param mode 0 = normal, 1 = dump used images, 2 = from dumped images
		@param dump_path the path where the images are saved/read... Use a ram disk !!!
		*/
		HardwareSensorCameraOpenCV(kernel::VariableCondition<int> &condition, int bufferSize, int camera_id, cv::Size size, int mode = 0, std::string dump_path = ".");
		
		HardwareSensorCameraOpenCV(kernel::VariableCondition<int> &condition, int bufferSize, std::string videofile, cv::Size size, int mode = 0, std::string dump_path = ".");
		/**
		Same as before but assumes that mode=2, and doesn't need a camera
		*/
		HardwareSensorCameraOpenCV(kernel::VariableCondition<int> &condition, cv::Size imgSize, std::string dump_path = ".");
		
		~HardwareSensorCameraOpenCV();

		virtual void start();
		virtual double getLastTimestamp() { boost::unique_lock<boost::mutex> l(mutex_data); return last_timestamp; }
		double getFreq() { return realFreq; }
};

typedef boost::shared_ptr<HardwareSensorCameraOpenCV> hardware_sensor_opencv_ptr_t;

}}}

#endif
