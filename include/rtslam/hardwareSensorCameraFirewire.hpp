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

#include <algorithm>

#include <viam/viamlib.h>
#include <viam/viamcv.h>

#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>

#include "rtslam/hardwareSensorAbstract.hpp"
#include <image/Image.hpp>
#include <kernel/misc.hpp>


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
		
		boost::thread *preloadTask_thread;
		void preloadTask(void)
		{
			struct timeval ts, *pts = &ts;
			int r;

			while(true)
			{
				r = viam_oneshot(handle, bank, buffer+buff_write, &pts, 1);
				boost::unique_lock<boost::mutex> l(mutex_data);
				std::swap(buff_write, buff_ready);
				bufferPtr[buff_ready]->timestamp = ts.tv_sec + ts.tv_usec*1e-6;
				image_count++;
				l.unlock();
			}
		}
	
	public:
		
		HardwareSensorCameraFirewire(const std::string &camera_id, viam_hwmode_t &mode)
		{
			// configure camera
			int r;
			handle = viam_init();
			viam_camera_t camera = viam_camera_create(handle, camera_id.c_str(), "camera1");
			bank = viam_bank_create(handle,"bank1");
			r = viam_bank_cameraadd(handle,bank,camera,"image1");
			r = viam_camera_sethwmode(handle, camera, &mode);
			r = viam_hardware_load(handle,"dc1394");
			r = viam_hardware_attach(handle);
			r = viam_bank_configure(handle, bank);
			r = viam_datatransmit(handle, bank, VIAM_ON);

			// configure data
			for(int i = 0; i < 3; ++i)
			{
				buffer[i] = cvCreateImage(viamSize_to_size(mode.size, 8, 1);
				bufferPtr[i].reset(new RawImage()); bufferSpecPtr[i] = SPTR_CAST<RawImage>(bufferPtr[i]);
				bufferSpecPtr[i]->setJafarImage(jafarImage_ptr_t(new image::Image(buffer[i])));
			}
			
			buff_in_use = 0;
			buff_write = 1;
			buff_ready = 2;
			image_count = 0;

			// start acquire task
			preloadTask_thread = new boost::thread(boost::bind(&HardwareSensorCameraFirewire::preloadTask,this));
		}

		
		~HardwareSensorCameraFirewire()
		{
			viam_release(handle);
		}
		
		
		virtual int acquireRaw(raw_ptr_t &rawPtr)
		{
			boost::unique_lock<boost::mutex> l(mutex_data);
			int missed_count = image_count-1;
			if (image_count > 0)
			{
				std::swap(buff_in_use, buff_ready);
				rawPtr = bufferPtr[buff_in_use];
				image_count = 0;
			}
			l.unlock();
			return missed_count;
		}
		
		
	private:
		cv::Size viamSize_to_size(viam_hwsize_t viamsize)
		{
			switch (viamsize)
			{
				case VIAM_HWSZ_160x120: return cv::Size(160,120);
				case VIAM_HWSZ_320x240: return cv::Size(320,240);
				case VIAM_HWSZ_512x384: return cv::Size(512,384);
				case VIAM_HWSZ_640x480: return cv::Size(640,480);
				case VIAM_HWSZ_800x600: return cv::Size(800,600);
				case VIAM_HWSZ_1024x768: return cv::Size(1024,768);
				case VIAM_HWSZ_1280x960: return cv::Size(1280,960);
				case VIAM_HWSZ_1600x1200: return cv::Size(1600,1200);
				default: return cv::Size(0,0);
			}
		}

};


}}

#endif

