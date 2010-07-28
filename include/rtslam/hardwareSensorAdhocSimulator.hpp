/**
 * \file hardwareSensorAdhocSimulator.hpp
 *
 * Interface between the slam and the ad-hoc simulator
 *
 * \author croussil@laas.fr
 * \date 24/07/2010
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
			double t;
			boost::shared_ptr<simu::AdhocSimulator> simulator;
			size_t robId, senId;
		public:
			HardwareSensorAdhocSimulator(double freq, boost::shared_ptr<simu::AdhocSimulator> simulator, size_t robId, size_t senId):
				dt(1./freq), t(0), simulator(simulator), robId(robId), senId(senId) {}
			int acquireRaw(raw_ptr_t &rawPtr)
			{
				if (simulator->hasEnded(robId, senId, t)) return -2;
				rawPtr = simulator->getRaw(robId, senId, t);
				t += dt;
				return 1;
			}
	};


}}}

#endif