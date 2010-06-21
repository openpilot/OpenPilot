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

#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>

#include "rtslam/hardwareSensorAbstract.hpp"
#include "image/Image.hpp"

// TODO move this generic swap function to jmath of kernel
namespace jafar {
namespace jmath {
	template<typename T>
	inline void swap(T &a, T &b) { T tmp = a; a = b; b = tmp; }
}}

namespace jafar {
namespace rtslam {


class HardwareSensorCameraFirewire: public HardwareSensorAbstract
{
	private:
		viam_bank_t bank;
		viam_handle_t handle;
		
		boost::mutex mutex_data;
		
		int buff_in_use;
		int buff_ready;
		int buff_write;
		bool has_ready;
		int missed_count;
		IplImage* buffer[3];
		raw_ptr_t bufferPtr[3];
		rawimage_ptr_t bufferSpecPtr[3];
		
		// TODO count missed frames
		
		boost::thread *preloadTask_thread;
		void preloadTask(void)
		{
			struct timeval ts, *pts = &ts;
			int r;

			while(true)
			{
				r = viam_oneshot(handle, bank, buffer+buff_write, &pts, 1);
				boost::unique_lock<boost::mutex> l(mutex_data);
				jmath::swap(buff_write, buff_ready);
				bufferPtr[buff_ready]->timestamp = ts.tv_sec + ts.tv_usec*1e-6;
				has_ready = true;
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
				buffer[i] = cvCreateImage(cvSize(640,480), 8, 1); // TODO choose right size
				bufferPtr[i].reset(new RawImage()); bufferSpecPtr[i] = SPTR_CAST<RawImage>(bufferPtr[i]);
				bufferSpecPtr[i]->setJafarImage(jafarImage_ptr_t(new image::Image(buffer[i])));
			}
			
			buff_in_use = 0;
			buff_write = 1;
			buff_ready = 2;
			has_ready = false;

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
			if (!has_ready) return -1;
			jmath::swap(buff_in_use, buff_ready);
			has_ready = false;
			rawPtr = bufferPtr[buff_in_use];
			l.unlock();
			return 0;
		}
		
		virtual void releaseRaw()
		{
//			boost::unique_lock<boost::mutex> l(mutex_data);
//			buff_in_use = -1;
//			l.unlock();
		}
		
};


}}

#endif

