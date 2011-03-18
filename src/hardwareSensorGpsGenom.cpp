/**
 * \file hardwareSensorGpsGenom.cpp
 *
 * File for getting data from the genom gps module
 *
 * \date 16/03/2011
 * \author croussil
 *
 * \ingroup rtslam
 */

#include "rtslam/hardwareSensorGpsGenom.hpp"

#ifdef HAVE_POSTERLIB
#include "h2timeLib.h"
#endif


namespace jafar {
namespace rtslam {
namespace hardware {


	void HardwareSensorGpsGenom::preloadTask(void)
	{
		char data[256];
#ifdef HAVE_POSTERLIB
		H2TIME h2timestamp;
#endif
		unsigned long prev_ntick = 0;
		double *date, prev_date = 0.0;
		double *pos; float *var;
		
		std::fstream f;
		if (mode == 1 || mode == 2)
		{
			std::ostringstream oss; oss << dump_path << "/GPS.log";
			f.open(oss.str().c_str(), (mode == 1 ? std::ios_base::out : std::ios_base::in));
		}
		
		while (true)
		{
			if (mode == 2)
			{
				f >> reading.data;
				boost::unique_lock<boost::mutex> l(mutex_data);
				if (isFull()) cond_offline_full.notify_all();
				if (f.eof()) { f.close(); return; }
				while (isFull()) cond_offline_freed.wait(l);
				
			} else
			{
#ifdef HAVE_POSTERLIB
				while (true) // wait for new data
				{
					usleep(1000);
					if (posterIoctl(posterId, FIO_GETDATE, &h2timestamp) != ERROR)
					{
						if (h2timestamp.ntick != prev_ntick)
						{
							prev_ntick = h2timestamp.ntick;
							posterRead(posterId, 0, data, 256);
							date = (double*)(data+80+44);
							if (*date != prev_date)
							{
								prev_date = *date;
								break;
							}
						}
					}
				}
#endif
				reading.arrival = getNowTimestamp();
				pos = (double*)(data+16);
				var = (float*)(data+48);
				reading.data(0) = *date;
				reading.data(1) = pos[0];
				reading.data(2) = pos[1];
				reading.data(3) = pos[2];
				reading.data(4) = var[0];
				reading.data(5) = var[1];
				reading.data(6) = var[2];
				//std::cout << "GPS poster : " << std::setprecision(15) << reading << std::endl;
				
				buffer(getWritePos()).data = reading.data;
				buffer(getWritePos()).data(0) += timestamps_correction;
				incWritePos();
				
				if (mode == 1)
				{
					// we put the maximum precision because we want repeatability with the original run
					f << std::setprecision(50) << reading.data << std::endl;
				}
				prev_date = *date;
			}
		}
	}
	
	
	HardwareSensorGpsGenom::HardwareSensorGpsGenom(kernel::VariableCondition<int> &condition, unsigned bufferSize, const std::string machine, int mode, std::string dump_path):
		HardwareSensorProprioAbstract(condition, bufferSize), reading(7), mode(mode), dump_path(dump_path)
	{
		// configure
#ifdef HAVE_POSTERLIB
		char *backup_poster_path = getenv("POSTER_PATH");
		setenv("POSTER_PATH", machine.c_str(), 1);
		if (posterFind("GPSInfo", &this->posterId) == ERROR) {
			JFR_ERROR(RtslamException, RtslamException::GENERIC_ERROR, "Poster GPSInfo could not be found");
		}
		if (backup_poster_path) setenv("POSTER_PATH", backup_poster_path, 1); else unsetenv("POSTER_PATH");
#endif
		
		// start acquire task
		//preloadTask();
		preloadTask_thread = new boost::thread(boost::bind(&HardwareSensorGpsGenom::preloadTask,this));
		
		if (mode == 2)
		{ // wait that log has been read before first frame
			boost::unique_lock<boost::mutex> l(mutex_data);
			cond_offline_full.wait(l);
		}
	}

}}}

