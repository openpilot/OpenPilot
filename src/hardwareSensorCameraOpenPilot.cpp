/**
 * \file hardwareSensorCameraOpenPilot.cpp
 * \date 18/09/2012
 * \author corvuscorax
 * \ingroup rtslam
 */

#include <algorithm>
#include <sstream>
#include <fstream>

#if 0
// creates conflict with boost sandbox with boost 1.42 in debug
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#endif

#include "kernel/timingTools.hpp"
#include "rtslam/hardwareSensorCameraOpenPilot.hpp"

#include <image/Image.hpp>

namespace jafar {
namespace rtslam {
namespace hardware {

	void HardwareSensorCameraOpenPilot::capture(IplImage* image)
	{ try {
		if (!started) return;
		// acquire the image
		cv::Mat t=cv::Mat(image);
		int buff_write = getWritePos();
		double time = kernel::Clock::getTime();
		cv::Mat t2(bufferImage[buff_write]);
		cv::cvtColor(t,t2,CV_RGB2GRAY);
		bufferSpecPtr[buff_write]->arrival = kernel::Clock::getTime();
		bufferSpecPtr[buff_write]->timestamp = time;
		last_timestamp = bufferSpecPtr[buff_write]->timestamp;
		incWritePos();
		condition.setAndNotify(1);
	} catch (kernel::Exception &e) { std::cout << e.what(); throw e; } }

	
	void HardwareSensorCameraOpenPilot::init(int mode, std::string dump_path, cv::Size imgSize)
	{
		this->mode = mode;
		this->dump_path = dump_path;

		// configure data
		bufferImage.resize(bufferSize);
		bufferSpecPtr.resize(bufferSize);
		for(int i = 0; i < bufferSize; ++i)
		{
			bufferImage[i] = cvCreateImage(imgSize, 8, 1);
			buffer(i).reset(new RawImage());
			bufferSpecPtr[i] = SPTR_CAST<RawImage>(buffer(i));
			bufferSpecPtr[i]->setJafarImage(jafarImage_ptr_t(new image::Image(bufferImage[i])));
		}
		
		found_first = 0;
		first_index = 0;
		index_load = 0;

		// start save tasks
		if (mode == 1)
		{
			saveTask_thread = new boost::thread(boost::bind(&HardwareSensorCameraOpenPilot::saveTask,this));
			savePushTask_thread = new boost::thread(boost::bind(&HardwareSensorCameraOpenPilot::savePushTask,this));
		}

	}
	
	void HardwareSensorCameraOpenPilot::start()
	{
		// start acquire task
		if (started) { std::cout << "Warning: This HardwareSensorCameraOpenPilot has already been started" << std::endl; return; }
		started = true;
		last_timestamp = kernel::Clock::getTime();
		if (mode == 2)
			preloadTask_thread = new boost::thread(boost::bind(&HardwareSensorCameraOpenPilot::preloadTaskOffline,this));
	}
		
	
	HardwareSensorCameraOpenPilot::HardwareSensorCameraOpenPilot(kernel::VariableCondition<int> &condition, cv::Size imgSize, std::string dump_path):
		HardwareSensorCamera(condition, imgSize, dump_path)
	{}
	

	HardwareSensorCameraOpenPilot::HardwareSensorCameraOpenPilot(kernel::VariableCondition<int> &condition, int bufferSize, cv::Size size, int mode, std::string dump_path):
		HardwareSensorCamera(condition, bufferSize)
	{
		init(mode, dump_path, size);
	}
	

	HardwareSensorCameraOpenPilot::~HardwareSensorCameraOpenPilot()
	{
	}



}}}

