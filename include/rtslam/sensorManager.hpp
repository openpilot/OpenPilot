/**
 * \file sensorManager.hpp
 *
 * Header file for the sensor managers, that decide which data of 
 * which sensor should be integrated in the filter in which order.
 *
 * \date 18/03/2011
 * \author croussil
 *
 * \ingroup rtslam
 */

#ifndef SENSOR_MANAGER_HPP_
#define SENSOR_MANAGER_HPP_

#include "rtslam/sensorAbstract.hpp"

namespace jafar {
namespace rtslam {

	/**
		Base class for sensor managers, that decide which data of which
		sensor should be integrated in the filter next.
		
		The goal is to avoid that a sensor be "starved" (never used)
		because it has large latencies and always arrives too late, once
		the filter has already moved after. In order to do that it can
		use a prediction of the timestamp and arrival time of the next
		data.
		
		The general problem to solve is to chose which updates to do and when,
		in order to maximize the gain of information with the constraints of real time
		computations and chronological updates in filter.
		
		\ingroup rtslam
	*/
	class SensorManagerAbstract
	{
		protected:
			map_ptr_t mapPtr;
			double start_date;
			bool all_init;
		public:
		
		struct ProcessInfo
		{
			sensor_ptr_t sen;
			unsigned id;
			bool no_more_data; // in offline mode, all of the sensors has no more data, so we stop everything
			ProcessInfo(sensor_ptr_t sen, unsigned id): sen(sen), id(id), no_more_data(false) {}
			ProcessInfo(bool no_more_data): sen(), id(0), no_more_data(no_more_data) {}
			ProcessInfo(): sen(), id(0), no_more_data(false) {}
		};
		
		SensorManagerAbstract(map_ptr_t mapPtr): mapPtr(mapPtr), start_date(0.), all_init(false) {}
		
		void setStartDate(double start_date) { this->start_date = start_date; }
		virtual ProcessInfo getNextDataToUse() = 0;
		
		/**
			@return 0 if no more sensor to init, 1 if needs to wait for data to init, 2 if returned correctly a data for init
			*/
		int getNextDataToInit(ProcessInfo &result)
		{
			// TODO set an order for init
			RawInfos infos;
			bool hasSensorMissingData = false;
			for (MapAbstract::RobotList::iterator robIter = mapPtr->robotList().begin();
				robIter != mapPtr->robotList().end(); ++robIter)
			{
				for (RobotAbstract::SensorList::iterator senIter = (*robIter)->sensorList().begin();
					senIter != (*robIter)->sensorList().end(); ++senIter)
				{
					if ((*senIter)->getUseForInit())
					{
						(*senIter)->queryAvailableRaws(infos);
						if (infos.available.size() > 0)
						{
							RawInfo &info = infos.available.front();
							if (info.timestamp > start_date) // if we anyway don't have data before the start date, use the first one
							{
								result = ProcessInfo((*senIter), info.id);
								return 2;
							} else // else find the last one before the start date
							{
								for(std::vector<RawInfo>::reverse_iterator rawIter = infos.available.rbegin(); rawIter != infos.available.rend(); ++rawIter)
									if ((*rawIter).timestamp <= start_date)
									{
										result = ProcessInfo((*senIter), (*rawIter).id);
										return 2;
									}
							}
						} else
							hasSensorMissingData = true;
					}
				}
			}
			
			if (hasSensorMissingData) return 1; else return 0;
		}
	};

	
	/**
		This sensor managers is made for offline processing, it integrates
		everything because we want repeatability and only the integrated data
		was saved for offline replay.
		So just return the oldest unprocessed data.
		
		\ingroup rtslam
	*/
	class SensorManagerReplay: public SensorManagerAbstract
	{
		public:
			SensorManagerReplay(map_ptr_t mapPtr): SensorManagerAbstract(mapPtr) {}
			
			virtual ProcessInfo getNextDataToUse()
			{
				ProcessInfo result;
				RawInfo info;
				double oldest_timestamp = -1;
				result.no_more_data = true;
				
				if (!all_init)
				{
					ProcessInfo result;
					int res;
					do {
						res = getNextDataToInit(result);
						if (res == 0) { all_init = true; break; } else
						if (res == 2) { return result; } else
						usleep(1000);
					} while (res == 1);
				}
				
				for (MapAbstract::RobotList::iterator robIter = mapPtr->robotList().begin();
					robIter != mapPtr->robotList().end(); ++robIter)
				{
					for (RobotAbstract::SensorList::iterator senIter = (*robIter)->sensorList().begin();
						senIter != (*robIter)->sensorList().end(); ++senIter)
					{
						int missed = (*senIter)->queryNextAvailableRaw(info);
						if (missed != -2) result.no_more_data = false;
						if (missed == 0 && (oldest_timestamp < 0 || info.timestamp < oldest_timestamp))
							{ result.id = info.id; result.sen = (*senIter); oldest_timestamp = info.timestamp; }
					}
				}
				
				return result;
			}
		
	};


	/**
		This sensor managers deals in a simple way with one sensor 
		with integrate_all policy and one sensor with integrate_last policy.
		It is temporary waiting for a good generic solution.
		
		\ingroup rtslam
	*/
	class SensorManagerOneAndOne: public SensorManagerAbstract
	{
		protected:
			sensor_ptr_t senAllPtr;
			sensor_ptr_t senLastPtr;
			RawInfos infosAll;
			RawInfos infosLast;
		public:
			SensorManagerOneAndOne(map_ptr_t mapPtr): SensorManagerAbstract(mapPtr) {}
			
			virtual ProcessInfo getNextDataToUse()
			{
				if (!all_init)
				{
					ProcessInfo result;
					int res;
					do {
						res = getNextDataToInit(result);
						if (res == 0) { all_init = true; break; } else
						if (res == 2) { return result; } else
						usleep(1000);
					} while (res == 1);
				}

    
				if (!senAllPtr || !senLastPtr)
				{
					for (MapAbstract::RobotList::iterator robIter = mapPtr->robotList().begin();
						robIter != mapPtr->robotList().end(); ++robIter)
					{
						for (RobotAbstract::SensorList::iterator senIter = (*robIter)->sensorList().begin();
							senIter != (*robIter)->sensorList().end(); ++senIter)
						{
							if ((*senIter)->getIntegrationPolicy()) senAllPtr = (*senIter); else senLastPtr = (*senIter);
						}
					}
				}
				
				
				int resAll=0, resLast=0;
				if (senAllPtr) resAll = senAllPtr->queryAvailableRaws(infosAll);
				if (senLastPtr) resLast = senLastPtr->queryAvailableRaws(infosLast);
				
				// only sen last
				if (!senAllPtr)
				{
					bool no_more_data = (resLast == -2);
					if (infosLast.available.size() > 0)
					{
						RawInfo &infoLast = infosLast.available.back();
						return ProcessInfo(senLastPtr, infoLast.id);
					} else
						return ProcessInfo(no_more_data); // wait
				}
				
				// only sen all
				if (!senLastPtr)
				{
					bool no_more_data = (resLast == -2);
					if (infosAll.available.size() > 0)
					{
						RawInfo &infoAll = infosAll.available.front();
						return ProcessInfo(senAllPtr, infoAll.id);
					} else
						return ProcessInfo(no_more_data); // wait
				}
				
				// one of each
				bool no_more_data = ((resAll == -2) && (resLast == -2));
				double tnow = getNowTimestamp();
					
				if (infosAll.available.size() > 0)
				{ // has all
					RawInfo &infoAll = infosAll.available.front();
				
					if (infosLast.available.size() > 0)
					{ // has all and last => use the oldest of oldest all and newest last
						RawInfo &infoLast = infosLast.available.back();
						
						if (infoAll.timestamp < infoLast.timestamp)
							return ProcessInfo(senAllPtr, infoAll.id);
						else
							return ProcessInfo(senLastPtr, infoLast.id);
					} else
					{ // has all but not last => use it if won't block next last (or next last hasn't arrived in time)
						if (infoAll.timestamp < infosLast.next.timestamp || tnow > infosLast.next.arrival)
							return ProcessInfo(senAllPtr, infoAll.id);
						else
							return ProcessInfo(no_more_data); // wait
					}
				} else
				{ // has not all
					if (infosLast.available.size() > 0)
					{ // has not all but last => use newest last that won't block next all (or next all hasn't arrived in time)
						for(std::vector<RawInfo>::reverse_iterator it = infosLast.available.rbegin(); it != infosLast.available.rend(); ++it)
						{
							RawInfo &infoLast = *it;
							if (infoLast.timestamp < infosAll.next.timestamp || tnow > infosAll.next.arrival)
								return ProcessInfo(senLastPtr, infoLast.id);
						}
						return ProcessInfo(no_more_data); // wait
					} else
					{
						return ProcessInfo(no_more_data); // wait
					}
				}
			}
		
		
	};
	
	
}}

#endif // SENSORMANAGER_HPP
