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
		public:
		
		struct ProcessInfo
		{
			sensor_ptr_t sen;
			unsigned id;
			bool no_more_data; // in offline mode, all of the sensors has no more data, so we stop everything
			ProcessInfo(sensor_ptr_t sen, unsigned id): sen(sen), id(id), no_more_data(false) {}
			ProcessInfo(bool no_more_data): sen(), id(0), no_more_data(true) {}
			ProcessInfo(): sen(), id(0), no_more_data(false) {}
		};
		
		SensorManagerAbstract(map_ptr_t mapPtr): mapPtr(mapPtr) {}
		
		virtual ProcessInfo getNextDataToUse() = 0;
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
				
				int resAll, resLast;
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
