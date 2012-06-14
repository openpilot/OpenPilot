/**
 * \file hardwareSensorExternalLoc.hpp
 *
 * Header file for getting external localization from another agent
 *
 * \date 13/06/2012
 * \author croussil
 *
 * \ingroup rtslam
 */

#ifndef HARDWARE_SENSOR_EXTERNALLOC_HPP_
#define HARDWARE_SENSOR_EXTERNALLOC_HPP_

#include <stdlib.h>
#include <unistd.h>

#include <jafarConfig.h>
#include "kernel/jafarMacro.hpp"
#include "rtslam/hardwareSensorAbstract.hpp"

#ifdef HAVE_POSTERLIB
#include "posterLib.h"
#endif


namespace jafar {
namespace rtslam {
namespace hardware {


class HardwareSensorExternalLoc: public HardwareSensorProprioAbstract
{
	private:
		boost::thread *preloadTask_thread;
		void preloadTask(void);
		
#ifdef HAVE_POSTERLIB
		POSTER_ID posterId;
#endif
		RawVec reading;
		int mode;
		std::string dump_path;
		double last_timestamp;
		
	public:
		HardwareSensorExternalLoc(kernel::VariableCondition<int> &condition, unsigned bufferSize, const std::string machine, int mode = 0, std::string dump_path = ".");
		
		virtual void start();
		virtual double getLastTimestamp() { boost::unique_lock<boost::mutex> l(mutex_data); return last_timestamp; }
		
};

/**
 @param elCameraBundle meta_x = (7)[u0 v0 fu fv r1 r2 r3 x y z qx qy qz qw], meta_P = (14) diag, x = (15)[x y z qx qy qz qw tx ty tz tex tey tez u v]
                       P = (26)[(xyz) full sym cov, (qxqyqzqz) full sym cov, (txtytztexteytez) diag cov, (uv) full sym cov]
*/
// FIXME also change .cpp
enum ExtLocType { elCameraBundle = 0, elNExtLocType };
inline size_t ExtLocSizes(ExtLocType t)
{
	static size_t ExtLocSizes[elNExtLocType] = { 48 };
	return ExtLocSizes[t];
}
struct ExtLoc
{
	double date;
	ExtLocType type;
	double meta_x[14]; // things that don't change over time (eg calibration)
	double meta_P[14]; // only variances for this
	double obs_x[9]; // observations
	double obs_P[45]; // full covariance (upper triangle)
};


}}}

#endif // HARDWARE_SENSOR_GPSGENOM_HPP_
