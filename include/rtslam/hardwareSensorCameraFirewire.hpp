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


namespace jafar {
namespace rtslam {

	
class HardwareSensorCameraFirewire: public HardwareSensorAbstract
{
	private:
		viam_bank_t bank;
		viam_handle_t handle;
		
		boost::mutex mutex_data;
		boost::mutex mutex_in_use;
		
		int buff_in_use;
		int buff_ready;
		int buff_write;
		int missed_count;
		image::Image* buffer[2];
		raw_ptr_t bufferPtr[2];
		rawimage_ptr_t bufferSpecPtr[2];
		
		boost::thread *preloadTask_thread;
		void preloadTask(void)
		{
			struct timeval ts, *pts = &ts;
			int r;
			
			IplImage iplIm[2] = { (IplImage)(*(buffer[0])), (IplImage)(*(buffer[1])) };
			IplImage *piplIm[2] = { &(iplIm[0]), &(iplIm[1]) };
				
			while(true)
			{
				r = viam_oneshot(handle, bank, piplIm+buff_write, &pts, 1);
				boost::unique_lock<boost::mutex> l(mutex_data);
				bufferPtr[buff_write]->timestamp = ts.tv_sec + ts.tv_usec*1e-6;
				buff_ready = buff_write;
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
			buffer[0] = new image::Image(640,480,CV_8U,JfrImage_CS_GRAY); // TODO choose right size
			bufferPtr[0].reset(new RawImage()); bufferSpecPtr[0] = SPTR_CAST<RawImage>(bufferPtr[0]);
			bufferSpecPtr[0]->setJafarImage(jafarImage_ptr_t(buffer[0]));
			
			buffer[1] = new image::Image(640,480,CV_8U,JfrImage_CS_GRAY); // TODO choose right size
			bufferPtr[1].reset(new RawImage()); bufferSpecPtr[1] = SPTR_CAST<RawImage>(bufferPtr[0]);
			bufferSpecPtr[1]->setJafarImage(jafarImage_ptr_t(buffer[1]));
			
			buff_in_use = -1;
			buff_ready = -1;
			buff_write = 0;
			
			// start acquire task
			preloadTask_thread = new boost::thread(boost::bind(&HardwareSensorCameraFirewire::preloadTask,this));
		}

		
		~HardwareSensorCameraFirewire()
		{
			viam_release(handle);
		}
		
		
		virtual int getRaw(raw_ptr_t &rawPtr)
		{
			boost::unique_lock<boost::mutex> l(mutex_data);
			if (buff_ready < 0) return -1;
			buff_in_use = buff_ready;
			buff_write = 1-buff_in_use;
			buff_ready = -1;
			rawPtr = bufferPtr[buff_in_use];
			l.unlock();
			mutex_in_use.lock();
		}
		
		virtual void releaseRaw()
		{
			mutex_in_use.unlock();
			boost::unique_lock<boost::mutex> l(mutex_data);
			buff_in_use = -1;
			l.unlock();
		}
		
};


}}

#endif

