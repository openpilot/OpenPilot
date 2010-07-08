/**
 * \file hardwareSensorCameraFirewire.cpp
 *
 * \author croussil@laas.fr
 * \date 18/06/2010
 *
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

#include "rtslam/hardwareSensorCameraFirewire.hpp"
#include <image/Image.hpp>

namespace jafar {
namespace rtslam {
namespace hardware {

#ifdef HAVE_VIAM
	cv::Size HardwareSensorCameraFirewire::viamSize_to_size(viam_hwsize_t viamsize)
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
#endif

	void HardwareSensorCameraFirewire::preloadTask(void)
	{
		struct timeval ts, *pts = &ts;
		int r;
		int last_processed_index = index-1;

		if (mode == 1)
		{
			#if 0
			// TODO test
			boost::filesystem::path bdump_path(dump_path);
			if (!exists(bdump_path) || !is_directory(bdump_path)) create_directory(bdump_path);
			
			boost::regex pattern1("*.pgm");
			boost::regex pattern2("*.time");
			for (boost::filesystem::recursive_directory_iterator it(bdump_path), end; it != end; ++it)
			{
				std::string name = it->path().leaf();
				if (boost::regex_match(name, pattern1) || boost::regex_match(name, pattern2))
					remove(it->path());
			}
			//remove(bdump_path / "*.pgm"); // FIXME possible ?
			#else
			std::ostringstream oss; oss << "mkdir -p " << dump_path << " ; rm -f " << dump_path << "/*.pgm ; rm -f " << dump_path << "/*.time" << std::endl;
			int r = system(oss.str().c_str());
			if (!r) {} // don't care
			#endif
		}

		while(true)
		{
			// acquire the image
			if (mode == 2)
			{
				if (index != last_processed_index)
				{
					std::ostringstream oss; oss << dump_path << "/image_" << std::setw(4) << std::setfill('0') << index;
					bufferSpecPtr[buff_write]->img->load(oss.str() + std::string(".pgm"));
					if (bufferSpecPtr[buff_write]->img->data() == NULL)
					{
						std::cout << "No more images to read." << std::endl;
						break;
					}
					std::fstream f((oss.str() + std::string(".time")).c_str(), std::ios_base::in);
					f >> bufferPtr[buff_write]->timestamp; f.close();
					last_processed_index = index;
				} else continue;
			} else
			{
#ifdef HAVE_VIAM
				r = viam_oneshot(handle, bank, buffer+buff_write, &pts, 1);
				bufferPtr[buff_write]->timestamp = ts.tv_sec + ts.tv_usec*1e-6;
#endif
			}
			// update the buffer infos
			boost::unique_lock<boost::mutex> l(mutex_data);
			std::swap(buff_write, buff_ready);
			image_count++;
			// dump the images
			if (mode == 1)
			{
				std::ostringstream oss; oss << dump_path << "/image_" << std::setw(4) << std::setfill('0') << index;
				bufferSpecPtr[buff_ready]->img->save(oss.str() + std::string(".pgm"));
				fstream f; f.open((oss.str() + std::string(".time")).c_str(), ios_base::out); 
				f << std::setprecision(20) << bufferPtr[buff_ready]->timestamp << std::endl; f.close();
			}
			l.unlock();
		}
	}

	void HardwareSensorCameraFirewire::init(int mode, std::string dump_path, cv::Size imgSize)
	{
		this->mode = mode;
		this->dump_path = dump_path;

		// configure data
		for(int i = 0; i < 3; ++i)
		{
			buffer[i] = cvCreateImage(imgSize, 8, 1);
			bufferPtr[i].reset(new RawImage()); bufferSpecPtr[i] = SPTR_CAST<RawImage>(bufferPtr[i]);
			bufferSpecPtr[i]->setJafarImage(jafarImage_ptr_t(new image::Image(buffer[i])));
		}
		
		buff_in_use = 0;
		buff_write = 1;
		buff_ready = 2;
		image_count = 0;
		index = 0;

		// start acquire task
		preloadTask_thread = new boost::thread(boost::bind(&HardwareSensorCameraFirewire::preloadTask,this));
	}

#ifdef HAVE_VIAM
	void HardwareSensorCameraFirewire::init(const std::string &camera_id, viam_hwmode_t &hwmode, int mode, std::string dump_path)
	{
		// configure camera
		if (mode != 2)
		{
			int r;
			handle = viam_init();
			viam_camera_t camera = viam_camera_create(handle, camera_id.c_str(), "camera1");
			bank = viam_bank_create(handle,"bank1");
			r = viam_bank_cameraadd(handle,bank,camera,"image1");
			r = viam_camera_sethwmode(handle, camera, &hwmode);
			r = viam_hardware_load(handle,"dc1394");
			r = viam_hardware_attach(handle);
			r = viam_bank_configure(handle, bank);
			r = viam_datatransmit(handle, bank, VIAM_ON);
		}

		init(mode, dump_path, viamSize_to_size(hwmode.size));
	}

	HardwareSensorCameraFirewire::HardwareSensorCameraFirewire(const std::string &camera_id, viam_hwmode_t &hwmode, int mode, std::string dump_path)
	{
		init(camera_id, hwmode, mode, dump_path);
	}
#endif


	HardwareSensorCameraFirewire::HardwareSensorCameraFirewire(cv::Size imgSize, std::string dump_path)
	{
		init(2, dump_path, imgSize);
	}
	
	HardwareSensorCameraFirewire::~HardwareSensorCameraFirewire()
	{
#ifdef HAVE_VIAM
		viam_release(handle);
#endif
	}
	
	
	int HardwareSensorCameraFirewire::acquireRaw(raw_ptr_t &rawPtr)
	{
		boost::unique_lock<boost::mutex> l(mutex_data);
		int missed_count = image_count-1;
		if (image_count > 0)
		{
			std::swap(buff_in_use, buff_ready);
			rawPtr = bufferPtr[buff_in_use];
			image_count = 0;
			index++;
		}
		l.unlock();
		return missed_count;
	}
	
	


}}}

