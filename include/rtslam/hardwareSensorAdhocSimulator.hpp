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

namespace jafar {
namespace rtslam {
namespace hardware {

	class HardwareSensorAdhocSimulator: public HardwareSensorAbstract
	{
		private:
			double dt;
			size_t n;
			boost::shared_ptr<simu::AdhocSimulator> simulator;
			size_t robId, senId;
		public:
			HardwareSensorAdhocSimulator(boost::condition_variable &rawdata_condition, boost::mutex &rawdata_mutex, double freq, boost::shared_ptr<simu::AdhocSimulator> simulator, size_t robId, size_t senId):
				HardwareSensorAbstract(rawdata_condition, rawdata_mutex),
				dt(1./freq), n(0), simulator(simulator), robId(robId), senId(senId) {}
			int acquireRaw(raw_ptr_t &rawPtr)
			{
				double t = n*dt;
				if (simulator->hasEnded(robId, senId, t)) return -2;
				rawPtr = simulator->getRaw(robId, senId, t);
				n++;
				return 1;
			}
	};


}}}

#endif
