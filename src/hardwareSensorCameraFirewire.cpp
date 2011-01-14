/**
 * \file hardwareSensorCameraFirewire.cpp
 * \date 18/06/2010
 * \author croussil
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
	cv::Size HardwareSensorCameraFirewire::viamSize_to_size(viam_hwsize_t hwsize)
	{
		switch (hwsize)
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
	
	double HardwareSensorCameraFirewire::viamFreq_to_freq(viam_hwfps_t hwfreq)
	{
		switch (hwfreq)
		{
			case VIAM_HWFPS_FREE: return 0.0;
			case VIAM_HWFPS_1_875: return 1.875;
			case VIAM_HWFPS_3_75: return 3.75;
			case VIAM_HWFPS_7_5: return 7.5;
			case VIAM_HWFPS_15: return 15.;
			case VIAM_HWFPS_30: return 30.;
			case VIAM_HWFPS_60: return 60.;
			case VIAM_HWFPS_120: return 120.;
			case VIAM_HWFPS_240: return 240;;
			default: return 0.;
		}
	}
	
	viam_hwsize_t HardwareSensorCameraFirewire::size_to_viamSize(cv::Size size)
	{
		viam_hwsize_t hwsize;
		switch (size.width)
		{
			case 160: if (size.height == 120) hwsize = VIAM_HWSZ_160x120; else hwsize = VIAM_HWSZ_INVALID; break;
			case 320: if (size.height == 240) hwsize = VIAM_HWSZ_320x240; else hwsize = VIAM_HWSZ_INVALID; break;
			case 512: if (size.height == 384) hwsize = VIAM_HWSZ_512x384; else hwsize = VIAM_HWSZ_INVALID; break;
			case 640: if (size.height == 480) hwsize = VIAM_HWSZ_640x480; else hwsize = VIAM_HWSZ_INVALID; break;
			case 800: if (size.height == 600) hwsize = VIAM_HWSZ_800x600; else hwsize = VIAM_HWSZ_INVALID; break;
			case 1024: if (size.height == 768) hwsize = VIAM_HWSZ_1024x768; else hwsize = VIAM_HWSZ_INVALID; break;
			case 1280: if (size.height == 960) hwsize = VIAM_HWSZ_1280x960; else hwsize = VIAM_HWSZ_INVALID; break;
			case 1600: if (size.height == 1200) hwsize = VIAM_HWSZ_1600x1200; else hwsize = VIAM_HWSZ_INVALID; break;
			default: hwsize = VIAM_HWSZ_INVALID;
		}
		return hwsize;
	}
/*
	viam_hwfps_t HardwareSensorCameraFirewire::freq_to_viamFreq(double freq)
	{
		viam_hwfps_t hwfreq;
		if (freq < (1.875+3.75)/2) hwfreq = VIAM_HWFPS_1_875; else
		if (freq < (3.75+7.5)/2) hwfreq = VIAM_HWFPS_3_75; else
		if (freq < (7.5+15)/2) hwfreq = VIAM_HWFPS_7_5; else
		if (freq < (15+30)/2) hwfreq = VIAM_HWFPS_15; else
		if (freq < (30+60)/2) hwfreq = VIAM_HWFPS_30; else
		if (freq < (60+120)/2) hwfreq = VIAM_HWFPS_60; else
		if (freq < (120+240)/2) hwfreq = VIAM_HWFPS_120; else
			hwfreq = VIAM_HWFPS_240;
		return hwfreq;
	}
*/
	viam_hwfps_t HardwareSensorCameraFirewire::freq_to_viamFreq(double freq)
	{
		freq -= 0.1;
		viam_hwfps_t hwfreq;
		if (freq <= 0.0) hwfreq = VIAM_HWFPS_FREE; else
		if (freq <= 1.875) hwfreq = VIAM_HWFPS_1_875; else
		if (freq <= 3.75) hwfreq = VIAM_HWFPS_3_75; else
		if (freq <= 7.5) hwfreq = VIAM_HWFPS_7_5; else
		if (freq <= 15) hwfreq = VIAM_HWFPS_15; else
		if (freq <= 30) hwfreq = VIAM_HWFPS_30; else
		if (freq <= 60) hwfreq = VIAM_HWFPS_60; else
		if (freq <= 120) hwfreq = VIAM_HWFPS_120; else
			hwfreq = VIAM_HWFPS_240;
		return hwfreq;
	}

	
	viam_hwformat_t HardwareSensorCameraFirewire::format_to_viamFormat(int format, int depth)
	{
		viam_hwformat_t hwformat;
		switch (format)
		{
			case 0: { // MONO
				switch (depth) {
					case 8: hwformat = VIAM_HWFMT_MONO8; break;
					case 16: hwformat = VIAM_HWFMT_MONO16; break;
					default: hwformat = VIAM_HWFMT_INVALID;
				}
				break;
			}
			case 1: { // YUV
				switch (depth) {
					case 6: hwformat = VIAM_HWFMT_YUV411; break;
					case 8: hwformat = VIAM_HWFMT_YUV422_UYVY; break;
					case 12: hwformat = VIAM_HWFMT_YUV444; break;
					default: hwformat = VIAM_HWFMT_INVALID;
				}
				break;
			}
			case 2: { // RGB
				if (depth == 24) hwformat = VIAM_HWFMT_RGB888;
				else hwformat = VIAM_HWFMT_INVALID;
				break;
			}
			default: hwformat = VIAM_HWFMT_INVALID;
		}
		return hwformat;
	}

	viam_hwtrigger_t HardwareSensorCameraFirewire::trigger_to_viamTrigger(bool trigger)
	{
		viam_hwtrigger_t hwtrigger;
		if (trigger) hwtrigger = VIAM_HWTRIGGER_MODE1_HIGH;
		else hwtrigger = VIAM_HWTRIGGER_INTERNAL;
		return hwtrigger;
	}

#endif

	void HardwareSensorCameraFirewire::preloadTask(void)
	{
		struct timeval ts, *pts = &ts;
		int r;
		int last_processed_index = index()-1;

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
				index.wait(boost::lambda::_1 != last_processed_index);
				//if (index != last_processed_index)
				{
					// FIXME manage multisensors : put sensor id in filename
					std::ostringstream oss; oss << dump_path << "/image_" << std::setw(4) << std::setfill('0') << index();
					bufferSpecPtr[buff_write]->img->load(oss.str() + std::string(".pgm"));
					if (bufferSpecPtr[buff_write]->img->data() == NULL)
					{
						boost::unique_lock<boost::mutex> l(mutex_data);
						no_more_data = true;
						//std::cout << "No more images to read." << std::endl;
						break;
					}
					std::fstream f((oss.str() + std::string(".time")).c_str(), std::ios_base::in);
					f >> bufferPtr[buff_write]->timestamp; f.close();
					last_processed_index = index();
				} //else  { boost::this_thread::yield(); continue; }
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
				std::ostringstream oss; oss << dump_path << "/image_" << std::setw(4) << std::setfill('0') << index();
				bufferSpecPtr[buff_ready]->img->save(oss.str() + std::string(".pgm"));
				std::fstream f; f.open((oss.str() + std::string(".time")).c_str(), std::ios_base::out); 
				f << std::setprecision(20) << bufferPtr[buff_ready]->timestamp << std::endl; f.close();
			}
			l.unlock();
			boost::unique_lock<boost::mutex> rawdata_lock(rawdata_mutex);
			rawdata_condition.notify_all();
			rawdata_lock.unlock();
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
		//index = 0;

		// start acquire task
		preloadTask_thread = new boost::thread(boost::bind(&HardwareSensorCameraFirewire::preloadTask,this));
	}
	
	HardwareSensorCameraFirewire::HardwareSensorCameraFirewire(boost::condition_variable &rawdata_condition, boost::mutex &rawdata_mutex, cv::Size imgSize, std::string dump_path):
		HardwareSensorAbstract(rawdata_condition, rawdata_mutex),index(0)
	{
		init(2, dump_path, imgSize);
		no_more_data = false;
	}
	

#ifdef HAVE_VIAM
	void HardwareSensorCameraFirewire::init(const std::string &camera_id, viam_hwmode_t &hwmode, int mode, std::string dump_path)
	{
		// configure camera
		if (mode == 0 || mode == 1)
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

	HardwareSensorCameraFirewire::HardwareSensorCameraFirewire(boost::condition_variable &rawdata_condition, boost::mutex &rawdata_mutex, const std::string &camera_id, cv::Size size, int format, int depth, double freq, bool trigger, int mode, std::string dump_path):
		HardwareSensorAbstract(rawdata_condition, rawdata_mutex), index(0)
	{
		viam_hwmode_t hwmode = { size_to_viamSize(size), format_to_viamFormat(format, depth), VIAM_HW_FIXED, freq_to_viamFreq(freq), trigger_to_viamTrigger(trigger) };
		realFreq = viamFreq_to_freq(hwmode.fps);
		std::cout << "Camera set to freq " << realFreq << " Hz (external trigger " << trigger << ")" << std::endl;
		init(camera_id, hwmode, mode, dump_path);
		no_more_data = false;
	}
#endif


	HardwareSensorCameraFirewire::~HardwareSensorCameraFirewire()
	{
#ifdef HAVE_VIAM
		if (mode == 0 || mode == 1)
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
			index.applyAndNotify(boost::lambda::_1++);
			//index++;
		}
		if (no_more_data && missed_count == -1) return -2; else return missed_count;
	}


}}}

