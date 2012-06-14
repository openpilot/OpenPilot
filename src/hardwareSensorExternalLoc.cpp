/**
 * \file hardwareSensorExternalLoc.cpp
 *
 * File for getting external localization from another agent
 *
 * \date 13/06/2012
 * \author croussil
 *
 * \ingroup rtslam
 */

#include "kernel/timingTools.hpp"
#include "jmath/jblas.hpp"
#include "jmath/ublasExtra.hpp"
#include "rtslam/quatTools.hpp"
#include "rtslam/pinholeTools.hpp"
#include "rtslam/hardwareSensorExternalLoc.hpp"

#ifdef HAVE_POSTERLIB
#include "h2timeLib.h"
#endif


namespace jafar {
namespace rtslam {
namespace hardware {


	void HardwareSensorExternalLoc::preloadTask(void)
	{ try {
		ExtLoc data;
		ExtLocType data_type = elNExtLocType;
#ifdef HAVE_POSTERLIB
		H2TIME h2timestamp;
#endif
		unsigned long prev_ntick = 0;
		double prev_date = 0.0;
		jblas::vec datavec;

		std::fstream f;
		if (mode == 1 || mode == 2)
		{
			std::ostringstream oss; oss << dump_path << "/extloc.log";
			f.open(oss.str().c_str(), (mode == 1 ? std::ios_base::out : std::ios_base::in));
			if (mode == 2)
			{
				int tmp; f >> tmp; data_type = (ExtLocType)tmp;
				datavec.resize(ExtLocSizes(data_type)+1);
			}
		}

		while (true)
		{
			if (mode == 2)
			{
				f >> datavec;
				boost::unique_lock<boost::mutex> l(mutex_data);
				if (isFull(true)) cond_offline_full.notify_all();
				if (f.eof()) { no_more_data = true; cond_offline_full.notify_all(); f.close(); return; }
				while (isFull(true)) cond_offline_freed.wait(l);
				
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
							if (posterRead(posterId, 0, &data, sizeof(data)) != ERROR)
							{
								if (data.date != prev_date)
								{
									prev_date = data.date;
									break;
								}
							}
						}
					}
				}
#endif
				reading.arrival = kernel::Clock::getTime();
				if (data_type == elNExtLocType)
				{
					data_type = data.type;
					datavec.resize(ExtLocSizes(data_type)+1);
					if (mode == 1) f << data_type << std::endl;
				}
				else if (data_type != data.type)
				{
					std::cout << "HardwareSensorExternalLoc: data type changed from " << data_type << " to " << data.type << ", ignoring this data" << std::endl;
					continue;
				}

				switch (data_type)
				{
					case elCameraBundle: {
						datavec(0) = data.date;
						for(size_t i = 0; i < 14; ++i) datavec(1+i) = data.meta_x[i];
						for(size_t i = 0; i < 14; ++i) datavec(1+14+i) = data.meta_P[i];
						for(size_t i = 0; i < 9; ++i) datavec(15+14+i) = data.obs_x[i];
						for(size_t i = 0; i < 32; ++i) datavec(1+7+9+i) = data.obs_P[i];
						break;
					}
					default: {
						break;
					}
				}

			}

			switch (data_type)
			{
				case elCameraBundle: {
					// extract data
					double date;
					jblas::vec intrinsic, distortion;
					jblas::vec p, q, o;
					jblas::sym_mat P, Q, O;
					date = datavec(0);
					intrinsic = ublas::subrange(datavec, 1, 5);
					distortion = ublas::subrange(datavec, 5, 8);
					p = ublas::subrange(datavec, 8+0, 11);
					q = ublas::subrange(datavec, 8+3, 15);
					o = ublas::subrange(datavec, 8+7, 17);
					P = jmath::ublasExtra::createSymMat(datavec, 17+0, 3, 0, 3);
					Q = jmath::ublasExtra::createSymMat(datavec, 17+6, 4, 0, 4);
					O = jmath::ublasExtra::createSymMat(datavec, 17+16, 2, 0, 2);

					// compute mean of u (standard representation)
					jblas::vec v(3);
					jblas::mat V_o(3,2);
					jblas::mat V_d(3,1);
					pinhole::backProjectPoint(intrinsic, distortion, o, 1.0, v, V_o, V_d);
					jblas::vec w(3);
					jblas::mat W_q(3,4);
					jblas::mat W_v(3,3);
					quaternion::rotate(q, v, w, W_q, W_v);
					jblas::vec u = w;
					jblas::mat U_w(3,3);
					jmath::ublasExtra::normalize(u);
					jmath::ublasExtra::normalizeJac(w, U_w);

					// compute covariance matrix of u
					jblas::mat W_o = ublas::prod(W_v, V_o);
					jblas::mat U_o = ublas::prod(U_w, W_o);
					jblas::mat U_q = ublas::prod(U_w, W_q);
					jblas::sym_mat U = jmath::ublasExtra::prod_JPJt(O, U_o) + jmath::ublasExtra::prod_JPJt(Q, U_q);

					// fill standard data vector
					reading.data.clear();
					reading.data(0) = date;
					ublas::subrange(reading.data, 1, 4) = p;
					ublas::subrange(reading.data, 4, 7) = u;
					jmath::ublasExtra::explodeSymMat(P, reading.data, 7, 6, 0, 3);
					jmath::ublasExtra::explodeSymMat(U, reading.data, 13, 6, 3, 3);

					break;
				}
				default: {
					std::cout << "hardwareSensorExternalLoc: data type " << data.type << " not managed." << std::endl;
					continue;
				}
			}

			
			int buff_write = getWritePos();
			buffer(buff_write).data = reading.data;
			buffer(buff_write).data(0) += timestamps_correction;
			last_timestamp = reading.data(0);
			incWritePos();
			
			if (mode == 1)
			{
				// we put the maximum precision because we want repeatability with the original run
				f << std::setprecision(50) << datavec << std::endl;
			}
			
		}
	} catch (kernel::Exception &e) { std::cout << e.what(); throw e; } }
	
	
	HardwareSensorExternalLoc::HardwareSensorExternalLoc(kernel::VariableCondition<int> &condition, unsigned bufferSize, const std::string machine, int mode, std::string dump_path):
		HardwareSensorProprioAbstract(condition, bufferSize, true), mode(mode), dump_path(dump_path)
	{
		addQuantity(qBundleobs);
		reading.resize(readingSize());
		// configure
		if (mode == 0 || mode == 1)
		{
#ifdef HAVE_POSTERLIB
			char *backup_poster_path = getenv("POSTER_PATH");
			setenv("POSTER_PATH", machine.c_str(), 1);
			if (posterFind("GPSInfo", &this->posterId) == ERROR) {
				JFR_ERROR(RtslamException, RtslamException::GENERIC_ERROR, "Poster GPSInfo could not be found");
			}
			if (backup_poster_path) setenv("POSTER_PATH", backup_poster_path, 1); else unsetenv("POSTER_PATH");
#else
			JFR_ERROR(RtslamException, RtslamException::MISSING_DEPENDENCY,
			          "You must install posterlib (pocolibs) if you want to use online GPS sensor");
#endif
		}
	}

	void HardwareSensorExternalLoc::start()
	{
		// start acquire task
		//preloadTask();
		if (started) { std::cout << "Warning: This HardwareSensorGpsGenom has already been started" << std::endl; return; }
		started = true;
		last_timestamp = kernel::Clock::getTime();
		preloadTask_thread = new boost::thread(boost::bind(&HardwareSensorExternalLoc::preloadTask,this));
		
		if (mode == 2)
		{ // wait that log has been read before first frame
			boost::unique_lock<boost::mutex> l(mutex_data);
			cond_offline_full.wait(l);
		}
	}
	
}}}

