/**
 * \file hardwareSensorCameraOpenCV.cpp
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
#include "rtslam/hardwareSensorCameraOpenCV.hpp"

#include <image/Image.hpp>

namespace jafar {
namespace rtslam {
namespace hardware {

	void HardwareSensorCameraOpenCV::preloadTask(void)
	{ try {
		//struct timeval ts, *pts = &ts;
		//int r;
		//bool emptied_buffers = false;
		//double date = 0.;
		cvNamedWindow("debug2",CV_WINDOW_AUTOSIZE);
		while(true)
		{
			// acquire the image
			cv::Mat t;
			int buff_write = getWritePos();
			double time = 0;
			if (VideoSource) {
				VideoSource->grab();
				//time = VideoSource->get(CV_CAP_PROP_POS_MSEC);
				if (time==0) {
					time = kernel::Clock::getTime();
				}
				VideoSource->retrieve(t,0);
				cv::Mat t2(bufferImage[buff_write]);
				// TODO: convert to greyscale
				//t.copyTo(t2);
				cv::cvtColor(t,t2,CV_RGB2GRAY);
				cvShowImage("debug2",bufferImage[buff_write]);
				//cv::imshow("debug2",t2);
				cvWaitKey(1);

			}
			if (time==0) {
				time = kernel::Clock::getTime();
			}
			//if (!emptied_buffers) date = kernel::Clock::getTime();
			//r = viam_oneshot(handle, bank, &(bufferImage[buff_write]), &pts, 1);
			//if (!emptied_buffers) { date = kernel::Clock::getTime()-date; if (date < 0.004) continue; else emptied_buffers = true; }
			bufferSpecPtr[buff_write]->arrival = kernel::Clock::getTime();
			//bufferSpecPtr[buff_write]->timestamp = ts.tv_sec + ts.tv_usec*1e-6;
			bufferSpecPtr[buff_write]->timestamp = time;
			last_timestamp = bufferSpecPtr[buff_write]->timestamp;
			incWritePos();
			condition.setAndNotify(1);
		}
	} catch (kernel::Exception &e) { std::cout << e.what(); throw e; } }

	
	void HardwareSensorCameraOpenCV::init(int mode, std::string dump_path, cv::Size imgSize)
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
			saveTask_thread = new boost::thread(boost::bind(&HardwareSensorCameraOpenCV::saveTask,this));
			savePushTask_thread = new boost::thread(boost::bind(&HardwareSensorCameraOpenCV::savePushTask,this));
		}

		if (mode!=2 && VideoSource) {
			VideoSource->set(CV_CAP_PROP_FRAME_WIDTH,imgSize.width);
			VideoSource->set(CV_CAP_PROP_FRAME_HEIGHT,imgSize.height);
			VideoSource->set(CV_CAP_PROP_FRAME_HEIGHT,imgSize.height);
			VideoSource->set(CV_CAP_PROP_FORMAT,CV_8UC1);
		}
		
	}
	
	void HardwareSensorCameraOpenCV::start()
	{
		// start acquire task
		if (started) { std::cout << "Warning: This HardwareSensorCameraOpenCV has already been started" << std::endl; return; }
		started = true;
		last_timestamp = kernel::Clock::getTime();
		if (mode == 2)
			preloadTask_thread = new boost::thread(boost::bind(&HardwareSensorCameraOpenCV::preloadTaskOffline,this));
		else
			preloadTask_thread = new boost::thread(boost::bind(&HardwareSensorCameraOpenCV::preloadTask,this));
	}
		
	
	HardwareSensorCameraOpenCV::HardwareSensorCameraOpenCV(kernel::VariableCondition<int> &condition, cv::Size imgSize, std::string dump_path):
		HardwareSensorCamera(condition, imgSize, dump_path)
	{}
	

	void HardwareSensorCameraOpenCV::init(int camera_id, int mode, std::string dump_path, cv::Size imgSize)
	{
		VideoSource = NULL;
		// configure camera
		if (mode == 0 || mode == 1)
		{
			VideoSource = new cv::VideoCapture(camera_id);

		}

		init(mode, dump_path, imgSize);
	}

	void HardwareSensorCameraOpenCV::init(std::string videofile, int mode, std::string dump_path, cv::Size imgSize)
	{
		VideoSource = NULL;
		// configure camera
		if (mode == 0 || mode == 1)
		{
			VideoSource = new cv::VideoCapture(videofile);

		}

		init(mode, dump_path, imgSize);
	}

	HardwareSensorCameraOpenCV::HardwareSensorCameraOpenCV(kernel::VariableCondition<int> &condition, int bufferSize, std::string videofile, cv::Size size, int mode, std::string dump_path):
		HardwareSensorCamera(condition, bufferSize)
	{
		init(videofile, mode, dump_path, size);
	}
	
	HardwareSensorCameraOpenCV::HardwareSensorCameraOpenCV(kernel::VariableCondition<int> &condition, int bufferSize, int camera_id, cv::Size size, int mode, std::string dump_path):
		HardwareSensorCamera(condition, bufferSize)
	{
		init(camera_id, mode, dump_path, size);
	}


	HardwareSensorCameraOpenCV::~HardwareSensorCameraOpenCV()
	{
	}



}}}

