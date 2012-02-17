/**
 * \file hardwareSensorCameraFirewire.hpp
 *
 * Header file for getting data from a firewire camera
 *
 * \date 18/06/2010
 * \author croussil
 *
 * \ingroup rtslam
 */

#ifndef HARDWARE_SENSOR_CAMERA_HPP_
#define HARDWARE_SENSOR_CAMERA_HPP_

#include <image/Image.hpp>
#include <kernel/threads.hpp>

#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "rtslam/hardwareSensorAbstract.hpp"
#include "rtslam/rawImage.hpp"


namespace jafar {
namespace rtslam {
namespace hardware {

/**
This class allows to get images from firewire with non blocking procedure,
using triple-buffering.
*/
class HardwareSensorCamera: public HardwareSensorExteroAbstract
{
	protected:
		std::vector<IplImage*> bufferImage;
		std::vector<rawimage_ptr_t> bufferSpecPtr;
		std::list<rawimage_ptr_t> bufferSave;
		unsigned index_load;
		unsigned first_index;
		int found_first; /// 0 = not found, 1 = found pgm, 2 = found png
		
		std::string dump_path;
		
		boost::thread *preloadTask_thread;
		void preloadTaskOffline(void);
		boost::thread *savePushTask_thread;
		void savePushTask(void);
		kernel::VariableCondition<size_t> saveTask_cond;
		boost::thread *saveTask_thread;
		void saveTask(void);
	
	
		void init(std::string dump_path, cv::Size imgSize);
	public:
		
		/**
		Same as before but assumes that mode=2, and doesn't need a camera
		*/
		HardwareSensorCamera(kernel::VariableCondition<int> &condition, cv::Size imgSize, std::string dump_path = ".");
		HardwareSensorCamera(kernel::VariableCondition<int> &condition, int bufferSize);
};


}}}

#endif
