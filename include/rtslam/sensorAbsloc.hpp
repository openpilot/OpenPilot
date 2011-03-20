/**
 * \file sensorAbsloc.hpp
 *
 * Header file for absolute localisation sensors (gps, motion capture...)
 *
 * \date 16/03/2011
 * \author croussil
 *
 * \ingroup rtslam
 */

#ifndef SENSORABSLOC_HPP_
#define SENSORABSLOC_HPP_

#include "jmath/jblas.hpp"
#include "jmath/ublasExtra.hpp"

#include "rtslam/rtSlam.hpp"
#include "rtslam/sensorAbstract.hpp"
#include "rtslam/innovation.hpp"

namespace jafar {
	namespace rtslam {

		class SensorAbsloc;
		typedef boost::shared_ptr<SensorAbsloc> absloc_ptr_t;
		
		/**
		 * Class for absolute localization sensors (gps, motion capture...)
		 * \ingroup rtslam
		 */
		class SensorAbsloc: public SensorProprioAbstract {
			protected:
				jblas::ind_array ia_rs;
				Innovation *innovation;
				jblas::mat INN_rs;
				bool hasVar;
			public:
				SensorAbsloc(const robot_ptr_t & robPtr, const filtered_obj_t inFilter = UNFILTERED):
				  SensorProprioAbstract(robPtr, inFilter),
					ia_rs(ia_globalPose), innovation(NULL), hasVar(false)
				{}
				~SensorAbsloc() { delete innovation; }
				virtual void setHardwareSensor(hardware::hardware_sensorprop_ptr_t hardwareSensorPtr_)
				{
					hardwareSensorPtr = hardwareSensorPtr_;
					// initialize jacobians and innovation sizes
					innovation = new Innovation(hardwareSensorPtr->dataSize());
					INN_rs.resize(innovation->size(), ia_rs.size(), false);
					// compute INN_rs now as it is constant, the problem is linear!
					INN_rs.clear();
					switch (innovation->size())
					{ // TODO velocities for gps
						case 7:
							ublas::subrange(INN_rs, 3,7, 3,7) = -jblas::identity_mat(4);
						case 3:
							ublas::subrange(INN_rs, 0,3, 0,3) = -jblas::identity_mat(3);
							break;
						default:
							JFR_ERROR(RtslamException, RtslamException::GENERIC_ERROR,
							          "SensorAbsloc reading size " << innovation->size()+1 << " not supported.");
					}
					if (hardwareSensorPtr->varianceSize() == hardwareSensorPtr->dataSize())
						hasVar = true;
				}
				
				virtual void process(unsigned id)
				{
					hardwareSensorPtr->getRaw(id, reading);
					
					// TODO a vec hardware sensor should return some information about what it is returning
					// just like robots should be able to do with, including whether the info is measure or variance.
					// pos(x/y/z)(gps,baro,mocap), ori (mocap,mag), vel(gps), gyr(y/p/r), acc(x/y/z)
					
					switch (innovation->size())
					{
						case 7: // POS+ORI
							ublas::subrange(innovation->x(), 4, 8) = ublas::subrange(robotPtr()->pose.x(), 4, 8) - ublas::subrange(reading.data, 4, 8);
							if (hasVar)
							{
								innovation->P()(4,4) = reading.data(4+7); innovation->P()(5,5) = reading.data(5+7);
								innovation->P()(6,6) = reading.data(6+7); innovation->P()(7,7) = reading.data(7+7);
							}
						case 3: // POS
							ublas::subrange(innovation->x(), 1, 4) = ublas::subrange(robotPtr()->pose.x(), 1, 4) - ublas::subrange(reading.data, 1, 4);
							if (hasVar)
							{
								innovation->P()(1,1) = reading.data(1+7); innovation->P()(2,2) = reading.data(2+7);
								innovation->P()(3,3) = reading.data(3+7);
							}
							break;
						default:
							JFR_ERROR(RtslamException, RtslamException::GENERIC_ERROR,
							          "SensorAbsloc reading size " << reading.data.size() << " not supported.");
					}
					
					map_ptr_t mapPtr = robotPtr()->mapPtr();
					ind_array ia_x = mapPtr->ia_used_states();
					mapPtr->filterPtr->correct(ia_x,*innovation,INN_rs,ia_rs);
					
					//hardwareSensorPtr->release();
				}
		};
}}

#endif
