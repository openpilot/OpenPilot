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

#include "kernel/timingTools.hpp"
#include "rtslam/hardwareSensorCameraFirewire.hpp"

#ifdef HAVE_VIAM
#include <viam/viamcv.h>
#endif

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

	viam_hwtrigger_t HardwareSensorCameraFirewire::trigger_to_viamTrigger(int trigger)
	{
		viam_hwtrigger_t hwtrigger;
		switch (trigger)
		{
			case 1: hwtrigger = VIAM_HWTRIGGER_MODE1_HIGH; break;
			case 2: hwtrigger = VIAM_HWTRIGGER_MODE0_HIGH; break;
			case 3: std::cout << "error: trigger mode 14 not implemented in viam, switch to mode 0" << std::endl;
			        hwtrigger = VIAM_HWTRIGGER_MODE0_HIGH; break;
			default:
			case 0: hwtrigger = VIAM_HWTRIGGER_INTERNAL;
		}
		return hwtrigger;
	}

#endif

	void HardwareSensorCameraFirewire::preloadTask(void)
	{ try {
		struct timeval ts, *pts = &ts;
		int r;
		//bool emptied_buffers = false;
		//double date = 0.;
		int ndigit = 0;

		while(true)
		{
			// acquire the image
			if (mode == 2)
			{
				boost::unique_lock<boost::mutex> l(mutex_data);
				while (isFull(true)) cond_offline_freed.wait(l);
				l.unlock();
				int buff_write = getWritePos();
				while (true)
				{
					// FIXME manage multisensors : put sensor id in filename
					std::ostringstream oss;
					for (int i = 3; i <= 7; ++i)
					{
						if (!found_first) ndigit = i;
						oss.str(""); oss << dump_path << "/image_" << std::setw(ndigit) << std::setfill('0') << index_load+first_index;
						if (found_first != 2 && bufferSpecPtr[buff_write]->img->load(oss.str() + std::string(".pgm")) && found_first == 0) found_first = 1;
						if (found_first != 1 && bufferSpecPtr[buff_write]->img->load(oss.str() + std::string(".png")) && found_first == 0) found_first = 2;
						if (found_first) break;
					}
					if (!found_first) { first_index++; continue; }
					
					if (bufferSpecPtr[buff_write]->img->data() == NULL)
					{
						boost::unique_lock<boost::mutex> l(mutex_data);
						no_more_data = true;
						//std::cout << "No more images to read." << std::endl;
						break;
					}
					std::fstream f((oss.str() + std::string(".time")).c_str(), std::ios_base::in);
					f >> bufferSpecPtr[buff_write]->timestamp; f.close();
					index_load++;
					break;
				} //else  { boost::this_thread::yield(); continue; }
				if (no_more_data) break;
			} else
			{
#ifdef HAVE_VIAM
				int buff_write = getWritePos();
				//if (!emptied_buffers) date = kernel::Clock::getTime();
				r = viam_oneshot(handle, bank, &(bufferImage[buff_write]), &pts, 1);
				//if (!emptied_buffers) { date = kernel::Clock::getTime()-date; if (date < 0.004) continue; else emptied_buffers = true; }
				bufferSpecPtr[buff_write]->arrival = kernel::Clock::getTime();
				bufferSpecPtr[buff_write]->timestamp = ts.tv_sec + ts.tv_usec*1e-6;
				last_timestamp = bufferSpecPtr[buff_write]->timestamp;
#endif
			}
			incWritePos();
			condition.setAndNotify(1);
		}
	} catch (kernel::Exception &e) { std::cout << e.what(); throw e; } }

#if 0
	void HardwareSensorCameraFirewire::saveTask(void)
	{ try {
		int last_processed_index = index();
		//if (mode == 1)
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
		
		while (true)
		{
			index.wait(boost::lambda::_1 != last_processed_index);
			// dump the images
			//if (mode == 1)
			{
				std::ostringstream oss; oss << dump_path << "/image_" << std::setw(4) << std::setfill('0') << index();
				bufferSpecPtr[last_sent_pos]->img->save(oss.str() + std::string(".pgm"));
				std::fstream f; f.open((oss.str() + std::string(".time")).c_str(), std::ios_base::out); 
				f << std::setprecision(20) << bufferSpecPtr[last_sent_pos]->timestamp << std::endl; f.close();
				last_processed_index = index();
			}
		}
	} catch (kernel::Exception &e) { std::cout << e.what(); throw e; } }
#endif
	
	void HardwareSensorCameraFirewire::savePushTask(void)
	{ try {
		int last_processed_index = index();
		
		// clean previously existing files
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
		
		while (true)
		{
			index.wait(boost::lambda::_1 != last_processed_index);
			// push image to file for saving
			saveTask_cond.lock();
			bufferSave.push_front(rawimage_ptr_t(static_cast<RawImage*>(bufferSpecPtr[last_sent_pos]->clone())));
			saveTask_cond.var++;
			saveTask_cond.unlock();
			saveTask_cond.notify();
			last_processed_index = index();
		}
	} catch (kernel::Exception &e) { std::cout << e.what(); throw e; } }
	
	
	void HardwareSensorCameraFirewire::saveTask(void)
	{ try {
		
		int save_index = index();
		int remain = 0, prev_remain = 0;
		
		while (true)
		{
			// wait for and get next data to save
			saveTask_cond.wait(boost::lambda::_1 != 0, false);
			rawimage_ptr_t image = bufferSave.back();
			bufferSave.pop_back();
			saveTask_cond.var--;
			remain = saveTask_cond.var;
			saveTask_cond.unlock();
			
			std::ostringstream oss; oss << dump_path << "/image_" << std::setw(7) << std::setfill('0') << save_index;
			image->img->save(oss.str() + std::string(".pgm"));
			std::fstream f; f.open((oss.str() + std::string(".time")).c_str(), std::ios_base::out); 
			f << std::setprecision(20) << image->timestamp << std::endl; f.close();
			
			if (remain > prev_remain || (remain == 0 && prev_remain != 0))
				std::cout << save_index << ": " << remain << " in queue." << std::endl;
			prev_remain = remain;
			
			++save_index;
		}
	} catch (kernel::Exception &e) { std::cout << e.what(); throw e; } }
	
	
	void HardwareSensorCameraFirewire::init(int mode, std::string dump_path, cv::Size imgSize)
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
			saveTask_thread = new boost::thread(boost::bind(&HardwareSensorCameraFirewire::saveTask,this));
			savePushTask_thread = new boost::thread(boost::bind(&HardwareSensorCameraFirewire::savePushTask,this));
		}
		
	}
	
	void HardwareSensorCameraFirewire::start()
	{
		// start acquire task
		if (started) { std::cout << "Warning: This HardwareSensorCameraFirewire has already been started" << std::endl; return; }
		started = true;
		last_timestamp = kernel::Clock::getTime();
		preloadTask_thread = new boost::thread(boost::bind(&HardwareSensorCameraFirewire::preloadTask,this));
	}
		
	
	HardwareSensorCameraFirewire::HardwareSensorCameraFirewire(kernel::VariableCondition<int> &condition, cv::Size imgSize, std::string dump_path):
		HardwareSensorExteroAbstract(condition, 3), saveTask_cond(0)
	{
		init(2, dump_path, imgSize);
	}
	

#ifdef HAVE_VIAM
	void HardwareSensorCameraFirewire::init(const std::string &camera_id, viam_hwmode_t &hwmode, double shutter, int mode, std::string dump_path)
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
			
			if (hwmode.trigger != VIAM_HWTRIGGER_MODE1_HIGH)
			{ 
				viam_filter_t shutter_filter = NULL;
				viam_image_ref image = viam_image_getbyname(handle, "image1");
				shutter_filter = viam_filter_luminance_create(handle, "shutter", VIAM_FILTER_SHUTTER, VIAM_VALUE_ABSOLUTE, shutter);
				if (shutter >= 1e-6)
					viam_filter_push(handle, image, shutter_filter, VIAM_FILTER_HARDWARE, VIAM_FILTER_MANUAL);
				else
					viam_filter_push(handle, image, shutter_filter, VIAM_FILTER_HARDWARE, VIAM_FILTER_HARDWARE_AUTO);
			}
			
			r = viam_datatransmit(handle, bank, VIAM_ON);
		}

		init(mode, dump_path, viamSize_to_size(hwmode.size));
	}

	HardwareSensorCameraFirewire::HardwareSensorCameraFirewire(kernel::VariableCondition<int> &condition, int bufferSize, const std::string &camera_id, cv::Size size, int format, int depth, viam_hwcrop_t crop, double freq, int trigger, double shutter, int mode, std::string dump_path):
		HardwareSensorExteroAbstract(condition, bufferSize), saveTask_cond(0)
	{
		viam_hwmode_t hwmode = { size_to_viamSize(size), format_to_viamFormat(format, depth), crop, freq_to_viamFreq(freq), trigger_to_viamTrigger(trigger) };
		realFreq = viamFreq_to_freq(hwmode.fps);
		std::cout << "Camera set to freq " << realFreq << " Hz (external trigger " << trigger << ")" << std::endl;
		init(camera_id, hwmode, shutter, mode, dump_path);
	}
#endif


	HardwareSensorCameraFirewire::~HardwareSensorCameraFirewire()
	{
#ifdef HAVE_VIAM
		if (mode == 0 || mode == 1)
			viam_release(handle);
		saveTask_cond.wait(boost::lambda::_1 == 0);
#endif
	}
	
#if 0
	
	int HardwareSensorCameraFirewire::acquireRaw(raw_ptr_t &rawPtr)
	{
		boost::unique_lock<boost::mutex> l(mutex_data);
		int missed_count = image_count-1;
		if (image_count > 0)
		{
			std::swap(buff_in_use, buff_ready);
			rawPtr = buffer[buff_in_use];
			image_count = 0;
			index.applyAndNotify(boost::lambda::_1++);
			//index++;
		}
		if (no_more_data && missed_count == -1) return -2; else return missed_count;
	}
#endif

}}}

