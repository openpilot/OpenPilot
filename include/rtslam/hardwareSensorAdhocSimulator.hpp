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

	class HardwareSensorAdhocSimulator: public HardwareSensorAbstract<raw_ptr_t>
	{
		private:
			double dt;
			size_t n;
			boost::shared_ptr<simu::AdhocSimulator> simulator;
			size_t robId, senId;
		public:
			HardwareSensorAdhocSimulator(kernel::VariableCondition<int> &condition, double freq, boost::shared_ptr<simu::AdhocSimulator> simulator, size_t robId, size_t senId):
				HardwareSensorAbstract(condition, 3),
				dt(1./freq), n(0), simulator(simulator), robId(robId), senId(senId) {}
			int getLastUnreadRaw(raw_ptr_t& raw)
			{
				double t = n*dt;
				if (simulator->hasEnded(robId, senId, t)) return -2;
				raw = simulator->getRaw(robId, senId, t);
				n++;
				return 0;
			}
	};


}}}

#endif
