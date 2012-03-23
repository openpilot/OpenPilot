/**
 * \file HardwareSensorCamera.cpp
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
#include "rtslam/hardwareSensorCamera.hpp"


#include <image/Image.hpp>

namespace jafar {
namespace rtslam {
namespace hardware {


	void HardwareSensorCamera::preloadTaskOffline(void)
	{ try {
		int ndigit = 0;

		while(true)
		{
			// acquire the image
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
					if (found_first != 2 && bufferSpecPtr[buff_write]->img->load(oss.str() + std::string(".pgm"), 0) && found_first == 0) { found_first = 1; std::cout << "First image " << oss.str() << ".pgm" << std::endl; }
					if (found_first != 1 && bufferSpecPtr[buff_write]->img->load(oss.str() + std::string(".png"), 0) && found_first == 0) { found_first = 2; std::cout << "First image " << oss.str() << ".png" << std::endl; }
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
			}
			if (no_more_data) break;
			incWritePos();
			condition.setAndNotify(1);
		}
	} catch (kernel::Exception &e) { std::cout << e.what(); throw e; } }



	void HardwareSensorCamera::savePushTask(void)
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
	
	
	void HardwareSensorCamera::saveTask(void)
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
	
	
	void HardwareSensorCamera::init(std::string dump_path, cv::Size imgSize)
	{
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
	}

	
	HardwareSensorCamera::HardwareSensorCamera(kernel::VariableCondition<int> &condition, cv::Size imgSize, std::string dump_path):
		HardwareSensorExteroAbstract(condition, 3), saveTask_cond(0)
	{
		init(dump_path, imgSize);
	}

	HardwareSensorCamera::HardwareSensorCamera(kernel::VariableCondition<int> &condition, int bufferSize):
		HardwareSensorExteroAbstract(condition, bufferSize), saveTask_cond(0)
	{}

	

}}}

