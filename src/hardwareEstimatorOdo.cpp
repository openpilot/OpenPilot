/**
 * \file hardwareEstimatorOdo.cpp
 * \date 18/01/2012
 * \author dmarquez
 * 
 * \ingroup rtslam
 */

#include "kernel/timingTools.hpp"
#include "rtslam/hardwareEstimatorOdo.hpp"
#include <sys/time.h>
#include <boost/bind.hpp>
#include "kernel/jafarMacro.hpp"
#include "jmath/misc.hpp"
#include "jmath/indirectArray.hpp"
#include "rtslam/rtslamException.hpp"

namespace jafar {
namespace rtslam {
namespace hardware {

	void HardwareEstimatorOdo::preloadTask(void)
	{ try {

		jblas::vec row(7);
		Position pos1;
		
		while (true)
		{
			boost::unique_lock<boost::mutex> l(mutex_data, boost::defer_lock_t());
			if (mode == 2)
			{
				pos1 = loadPosition(index_load_);
				l.lock();
				if (write_position == read_position) cond_offline.notify_all();
				while (write_position == read_position) cond_data.wait(l);
				if (!pos1.m_date)
					std::cout << " Failed to read position " << std::endl;
				row(0) = pos1.m_date;
				row(1) = pos1.m_mainToBase(0); //x
				row(2) = pos1.m_mainToBase(1); //y
				row(3) = pos1.m_mainToBase(2); //z
				row(4) = pos1.m_mainToBase(5); //roll
				row(5) = pos1.m_mainToBase(4); //pitch
				row(6) = pos1.m_mainToBase(3); //yaw
				index_load_++;
			} else
			{
				std::cout << "Read pom poster position: work in progress" << std::endl; 
			}
			ublas::matrix_row<jblas::mat>(buffer, write_position) = row;
// 			buffer(write_position,0) += timestamps_correction;
			++write_position; if (write_position >= bufferSize) write_position = 0;
			l.unlock();
		}
		
	} catch (kernel::Exception &e) { std::cout << e.what(); throw e; } }

	HardwareEstimatorOdo::HardwareEstimatorOdo(double trigger_mode, double trigger_freq, double trigger_shutter, int bufferSize_, int mode, std::string dump_path):
		buffer(bufferSize_, 7), bufferSize(bufferSize_), write_position(0), read_position(bufferSize_-1),
		timestamps_correction(0.0), mode(mode), dump_path(dump_path)
	{
		if (mode != 2)
		{
			std::cout << "Set read pom poster frequency: work in progress" << std::endl; 
		} else
		{
			realFreq = trigger_freq;
		}
		
	}
	
	HardwareEstimatorOdo::~HardwareEstimatorOdo()
	{

	}

	void HardwareEstimatorOdo::start()
	{
		if (started) { std::cout << "Warning: This HardwareEstimatorOdo has already been started" << std::endl; return; }
		started = true;
		for(int i = 0; i < bufferSize; ++i) buffer(i,0) = -1.;
		// start acquire task
		index_load_ = 0;
		preloadTask_thread = new boost::thread(boost::bind(&HardwareEstimatorOdo::preloadTask,this));
		if (mode == 2)
		{ // wait that log has been read before first frame
			boost::unique_lock<boost::mutex> l(mutex_data);
			cond_offline.wait(l);
		}		
		std::cout << " done." << std::endl;
	}
	
	void HardwareEstimatorOdo::setSyncConfig(double timestamps_correction)
	{
		this->timestamps_correction = timestamps_correction;
	}
	
	Position HardwareEstimatorOdo::loadPosition(unsigned int index_) const
	{
		Position pos;
		loadPosition(index_, pos);
		return pos;
	}
	
	void HardwareEstimatorOdo::loadPosition(unsigned int index_, Position& pos) const
	{
		 std::ostringstream filepath;
		 filepath << dump_path << "/image_" << std::setw(4) << std::setfill('0') << index_ << ".pos";
		 std::fstream f(filepath.str().c_str(), std::ios_base::in);
		 if (f.fail()){ std::cout<<"no more data"<<std::endl; return;}
		 pos.load(filepath.str()); 
	}
	
	void Position::loadKeyValueFile(jafar::kernel::KeyValueFile const& keyValueFile)
	{	
		keyValueFile.getItem("date", m_date);
		keyValueFile.getItem("mainToBase", m_mainToBase);
	}

	jblas::mat_indirect HardwareEstimatorOdo::acquireReadings(double t1, double t2)
	{
		JFR_ASSERT(t1 <= t2, "");
		boost::unique_lock<boost::mutex> l(mutex_data);
		int i1, i2;
		int i, j;

		// find indexes by dichotomy
		int i_left = write_position, i_right = write_position + bufferSize-1;
		while(i_left != i_right)
		{
			j = (i_left+i_right)/2;
			i = j % bufferSize;
			if (buffer(i,0) >= t1) i_right = j; else i_left = j+1;
		}
		i = i_left % bufferSize;
		i1 = (i-1 + bufferSize) % bufferSize;
		if (t1 <= 1.0 && buffer(i1,0) < 0.0) i1 = i;
		bool no_larger = (buffer(i,0) < t1);
		bool no_smaller = (i == write_position);
		if (no_larger && buffer(i1,0) < 0.0)  // no data at all
			return ublas::project(buffer, jmath::ublasExtra::ia_set(ublas::range(0,0)), jmath::ublasExtra::ia_set(ublas::range(0, buffer.size2())));
		if (no_smaller) JFR_ERROR(RtslamException, RtslamException::BUFFER_OVERFLOW, "Missing data: increase buffer size !");
		
		if (no_larger)
			i2 = i1;
		else
		{
			i_right = write_position + bufferSize-1;
			while(i_left != i_right)
			{
				j = (i_left+i_right)/2;
				i = j % bufferSize;
				if (buffer(i,0) >= t2) i_right = j; else i_left = j+1;
			}
			i = i_left % bufferSize;
			i2 = i;
		}
		
		read_position = i1;
		l.unlock();
		cond_data.notify_all();

		if (i1 < i2)
		{
			return ublas::project(buffer, 
				jmath::ublasExtra::ia_set(ublas::range(i1,i2+1)),
				jmath::ublasExtra::ia_set(ublas::range(0,buffer.size2())));
		} else
		{
			return ublas::project(buffer, 
				jmath::ublasExtra::ia_concat(jmath::ublasExtra::ia_set(ublas::range(i1,buffer.size1())),
				                            jmath::ublasExtra::ia_set(ublas::range(0,i2+1))),
				jmath::ublasExtra::ia_set(ublas::range(0,buffer.size2())));
		}
	}
}}}

