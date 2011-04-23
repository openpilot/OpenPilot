/**
 * \file hardwareSensorAdhocSimulator.hpp
 *
 * Interface between the slam and the ad-hoc simulator
 *
 * \date 24/07/2010
 * \author croussil
 *
 * \ingroup rtslam
 */

#ifndef HARDWARESENSORADHOCSIMULATOR_HPP_
#define HARDWARESENSORADHOCSIMULATOR_HPP_

#include "rtslam/simulator.hpp"
#include "rtslam/hardwareSensorAbstract.hpp"

namespace jafar {
namespace rtslam {
namespace hardware {

	class HardwareSensorAdhocSimulator: public HardwareSensorExteroAbstract
	{
		private:
			double dt;
			size_t n;
			boost::shared_ptr<simu::AdhocSimulator> simulator;
			size_t robId, senId;
		protected:
			virtual void getTimingInfos(double &data_period, double &arrival_delay) { data_period=dt; arrival_delay=0.; }
		public:
			HardwareSensorAdhocSimulator(kernel::VariableCondition<int> &condition, double freq, boost::shared_ptr<simu::AdhocSimulator> simulator, size_t robId, size_t senId):
				HardwareSensorExteroAbstract(condition, 3),
				dt(1./freq), n(0), simulator(simulator), robId(robId), senId(senId) {}
			virtual void start() {}
			
			int getRawInfo(size_t m, RawInfo &info)
			{
				info.timestamp = m*dt;
				info.arrival = info.timestamp;
				info.id = m;
				if (simulator->hasEnded(robId, senId, info.timestamp)) return -2;
				return 0;
			} 
	
			virtual double getLastTimestamp() { return (n-1)*dt; }
			
			virtual int getUnreadRawInfos(RawInfos &infos)
			{
				infos.available.clear();
				RawInfo info;
				int res = getRawInfo(n, info);
				if (res != -2) infos.available.push_back(info);
				infos.integrate_all = false;
				getRawInfo(n+1, info);
				infos.next = info;
				infos.process_time = 0;
				return res;
			}
		
			virtual int getNextRawInfo(RawInfo &info)
			{
				return getRawInfo(n, info);
			}
		
			virtual void getRaw(unsigned id, raw_ptr_t& raw)
			{
				double t = id*dt;
				raw = simulator->getRaw(robId, senId, t);
				n = id+1;
				return;
			}
		
			virtual double getRawTimestamp(unsigned id)
			{
				return id*dt;
			}

			virtual int getLastUnreadRaw(raw_ptr_t& raw)
			{
				getRaw(n++, raw);
				if (simulator->hasEnded(robId, senId, (n-1)*dt)) return -2;
				return 0;
			}
		
			virtual void getLastProcessedRaw(raw_ptr_t& raw)
			{
				getRaw(n-1, raw);
			}
		
			virtual void release() {}

	};

}}}

#endif
