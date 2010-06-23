/**
 * \file hardwareSensorCameraFirewire.hpp
 *
 * Header file for getting data from a firewire camera
 *
 * \author croussil@laas.fr
 * \date 18/06/2010
 *
 * \ingroup rtslam
 */

#ifndef HARDWARE_SENSOR_CAMERA_FIREWIRE_HPP_
#define HARDWARE_SENSOR_CAMERA_FIREWIRE_HPP_

#include <viam/viamlib.h>
#include <viam/viamcv.h>

#include "rtslam/hardwareSensorAbstract.hpp"
#include "rtslam/rawImage.hpp"


namespace jafar {
namespace rtslam {

/**
This class allows to get images from firewire with non blocking procedure,
using triple-buffering.
*/
class HardwareSensorCameraFirewire: public HardwareSensorAbstract
{
	private:
		viam_bank_t bank;
		viam_handle_t handle;
		
		boost::mutex mutex_data;
		
		int buff_in_use;
		int buff_ready;
		int buff_write;
		int image_count;
		IplImage* buffer[3];
		raw_ptr_t bufferPtr[3];
		rawimage_ptr_t bufferSpecPtr[3];
		
		int mode;
		int index;
		std::string dump_path;
		
		boost::thread *preloadTask_thread;
		void preloadTask(void);
	
		cv::Size viamSize_to_size(viam_hwsize_t viamsize);
	
	public:
		
		/**
		@param camera_id the Firewire camera id (0x....)
		@param hwmode the hardware mode
		@param mode 0 = normal, 1 = dump used images, 2 = from dumped images
		@param dump_path the path where the images are saved/read... Use a ram disk !!!
		*/
		HardwareSensorCameraFirewire(const std::string &camera_id, viam_hwmode_t &hwmode, int mode = 0, std::string dump_path = ".");
		
		~HardwareSensorCameraFirewire();
		
		virtual int acquireRaw(raw_ptr_t &rawPtr);
};


}}

#endif

