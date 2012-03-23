/**
 * \file HardwareSensorCameraUeye.hpp
 *
 * Header file for getting data from a firewire camera
 *
 * \date 18/06/2010
 * \author croussil
 *
 * \ingroup rtslam
 */

#ifndef HARDWARE_SENSOR_CAMERA_UEYE_HPP_
#define HARDWARE_SENSOR_CAMERA_UEYE_HPP_

#include <jafarConfig.h>

#ifdef HAVE_UEYE
#include <ueye.h>
#endif

#include "rtslam/hardwareSensorCamera.hpp"
#include "rtslam/rawImage.hpp"


namespace jafar {
namespace rtslam {
namespace hardware {

/**
This class allows to get images from firewire with non blocking procedure,
using triple-buffering.
*/
class HardwareSensorCameraUeye: public HardwareSensorCamera
{
	private:
#ifdef HAVE_UEYE
		HIDS camera;
#endif
		
		double realFreq;
		double last_timestamp;
		
		int mode;

		void preloadTask(void);
		
		void init(int mode, std::string dump_path, cv::Size imgSize);
#ifdef HAVE_UEYE
		void init(const std::string &camera_id, cv::Size imgSize, double shutter, double freq, int trigger, int mode, std::string dump_path);
#endif
	public:
		
#ifdef HAVE_UEYE
		/**
		@param camera_id the Firewire camera id (0x....)
		@param hwmode the hardware mode
		@param mode 0 = normal, 1 = dump used images, 2 = from dumped images
		@param dump_path the path where the images are saved/read... Use a ram disk !!!
		*/
		HardwareSensorCameraUeye(kernel::VariableCondition<int> &condition, int bufferSize, const std::string &camera_id, cv::Size size, double freq, int trigger, double shutter, int mode = 0, std::string dump_path = ".");
#endif
		/**
		Same as before but assumes that mode=2, and doesn't need a camera
		*/
		HardwareSensorCameraUeye(kernel::VariableCondition<int> &condition, cv::Size imgSize, std::string dump_path = ".");
		
		~HardwareSensorCameraUeye();

		virtual void start();
		virtual double getLastTimestamp() { boost::unique_lock<boost::mutex> l(mutex_data); return last_timestamp; }
		double getFreq() { return realFreq; }
};

typedef boost::shared_ptr<HardwareSensorCameraUeye> hardware_sensor_ueye_ptr_t;

}}}

#endif
