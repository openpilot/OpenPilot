/**
 * \file sensorAbstract.hpp
 *
 * Header file for the abstract sensor.
 *
 * \date 28/01/2010
 * \author jsola
 *
 * \ingroup rtslam
 */

#ifndef SENSORABSTRACT_H_
#define SENSORABSTRACT_H_

/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */

#include "jmath/jblas.hpp"
#include "rtslam/rtSlam.hpp"
#include "kernel/IdFactory.hpp"
//include parents
#include "rtslam/parents.hpp"
#include "rtslam/mapAbstract.hpp"
#include "rtslam/mapObject.hpp"
#include "rtslam/robotAbstract.hpp"
#include "rtslam/hardwareSensorAbstract.hpp"
#include <boost/smart_ptr.hpp>

namespace jafar {
	namespace rtslam {

		// Forward declarations of children
		class ObservationAbstract;
		class DataManagerAbstract;

		/** 
			Base class for all types of sensors.
			\ingroup rtslam
		*/
		class SensorAbstract: public MapObject,
		                      public ChildOf<RobotAbstract>,
		                      public boost::enable_shared_from_this<SensorAbstract>
		{
			private:
				// define the function linkToParentRobot().
				ENABLE_LINK_TO_PARENT(RobotAbstract,Robot,SensorAbstract);
				// define the functions robotPtr() and robot().
				ENABLE_ACCESS_TO_PARENT(RobotAbstract,robot);
		
			protected:
				bool integrate_all;
				bool use_for_init; ///< use this sensor to init the state, so needs to process it before those that are not used to init the state
				bool need_init; ///< needs a few seconds of readings to initialize
				
			public:
				
				/**
				 * Selectable LOCAL or REMOTE pose constructor.
				 * Creates a sensor with the pose indexed in a map.
				 * \param robPtr the robot
				 * \param inFilter flag indicating if the sensor state is part of the filter (REMOTE).
				 */
				SensorAbstract(const robot_ptr_t & robPtr, const filtered_obj_t inFilter = UNFILTERED);
				
				/**
				 * Mandatory virtual destructor.
				 */
				virtual ~SensorAbstract() {
				}
				
				void setIntegrationPolicy(bool integrate_all) { this->integrate_all = integrate_all; }
				bool getIntegrationPolicy() { return integrate_all; }
				void setUseForInit(bool use_for_init) { this->use_for_init = use_for_init; }
				bool getUseForInit() { return use_for_init; }
				void setNeedInit(bool need_init) { this->need_init = need_init; }
				bool getNeedInit() { return need_init; }

				virtual int queryAvailableRaws(RawInfos &infos) = 0; ///< get information about the available raws and the estimated dates for next one
				virtual int queryNextAvailableRaw(RawInfo &info) = 0; ///< get information about the next available raw
				virtual double getRawTimestamp(unsigned id) = 0;
				virtual void process(unsigned id) = 0; ///< process the given raw and throw away the previous unprocessed ones \return innovation
				virtual void process_fake(unsigned id) = 0; ///< don't do any predict or update, but let the data acquisition run smoothly
				virtual void init(double date) { use_for_init = false; } ///< use previous data to initialize the robot if needed
				virtual void start() = 0;

				enum type_enum {
					PINHOLE, BARRETO
				} type;
				enum kind_enum {
					PROPRIOCEPTIVE, EXTEROCEPTIVE
				} kind;
				virtual std::string categoryName() const {
					return "SENSOR";
				}

				static IdFactory sensorIds;
				void setId() { id(sensorIds.getId()); }
				
				void setPose(double x, double y, double z, double rollDeg,
				    double pitchDeg, double yawDeg);
				void setPoseStd(double x, double y, double z, double rollDeg,
				    double pitchDeg, double yawDeg, double xStd, double yStd,
				    double zStd, double rollDegStd, double pitchDegStd,
				    double yawDegStd);
				void setPoseStd(double x, double y, double z, double rollDeg,
				    double pitchDeg, double yawDeg, double posStd, double oriDegStd) {
					setPoseStd(x, y, z, rollDeg, pitchDeg, yawDeg, posStd, posStd,
					           posStd, oriDegStd, oriDegStd, oriDegStd);
				}


				/// Sensor pose in robot
				Gaussian pose;

				/// Indices of sensor's global pose in map (this is either the \a ia of the robot, \a rob.ia to make it short, or the \b ia_union() of \a rob.ia and \a sen.ia)
				ind_array ia_globalPose;
				/// Flag indicating if the sensor pose is being filtered
				bool isInFilter;
				
				/**
				 * Get sensor pose in global frame.
				 * This function composes robot pose with sensor pose to obtain the global sensor pose.
				 *
				 * \return the global pose.
				 */
				vec7 globalPose();

				/**
				 * Get sensor pose in global frame.
				 * This function composes robot pose with sensor pose to obtain the global sensor pose.
				 * It renders the Jacobians of the composed frame wrt all variables that are in the map (either robot only, or robot and sensor),
				 * depending on the sensor pose storage being LOCAL or REMOTE (see class Gaussian for doc on storage options).
				 * Therefore, this Jacobian is either 7-by-7 (LOCAL sensor pose) or 7-by-14 (REMOTE sensor pose).
				 *
				 * The concerned states are available as an indirect array \a ia_rs, member of the class and defined at construction time.
				 * \param poseG the global pose.
				 * \param PG_rs the Jacobian wrt the mapped states of robot and sensor.
				 */
				void globalPose(jblas::vec7 & senGlobalPose, jblas::mat & SG_rs);

		};
		
		
		/** 
			Base class for proprioceptive sensors (that directly observe the state of the robot)
			(IMU, odometry, GPS, movement models ...)
			\ingroup rtslam
		*/
		class SensorProprioAbstract: public SensorAbstract
		{
			protected:
				hardware::hardware_sensorprop_ptr_t hardwareSensorPtr;
				RawVec reading;
			public:
				SensorProprioAbstract(const robot_ptr_t & robPtr, const filtered_obj_t inFilter = UNFILTERED):
				  SensorAbstract(robPtr, inFilter) { kind = PROPRIOCEPTIVE; }

				void setHardwareSensor(hardware::hardware_sensorprop_ptr_t hardwareSensorPtr_)
					{ hardwareSensorPtr = hardwareSensorPtr_; }
				virtual void start() { hardwareSensorPtr->start(); }
				
				virtual int queryAvailableRaws(RawInfos &infos)
					{ int res = hardwareSensorPtr->getUnreadRawInfos(infos); infos.integrate_all = integrate_all; return res; }
				virtual int queryNextAvailableRaw(RawInfo &info)
					{ return hardwareSensorPtr->getNextRawInfo(info); }
				virtual double getRawTimestamp(unsigned id) { return hardwareSensorPtr->getRawTimestamp(id); } 
				//process(id) will do the filtering, so it is specific to each hardware
				void process_fake(unsigned id) { hardwareSensorPtr->getRaw(id, reading); robotPtr()->move_fake(reading.data(0)); }
		};
		
		/** 
			Base class for proprioceptive sensors that can be used for filter prediction step.
			They must be able to predict the full pose, and be faster than all other sensors used
			as observations or easily interpolated to simulate it.
			Usually a movement model (eg constant velocity), odometry, 6D IMU...
			\ingroup rtslam
		*/
		class SensorProprioPredictorAbstract: public SensorProprioAbstract
		{
			public:
				SensorProprioPredictorAbstract(const robot_ptr_t & robPtr, const filtered_obj_t inFilter = UNFILTERED):
					SensorProprioAbstract(robPtr, inFilter) {}
				virtual void predict(double t) = 0;
		
		};
		
		/** 
			Base class for exteroceptive sensors (that need to map the environment)
			(cameras, lasers, ...)
			\ingroup rtslam
		*/
		class SensorExteroAbstract: public SensorAbstract,
		                            public ParentOf<DataManagerAbstract>
		{
			protected:
				// define the type DataManagerList, and the function dataManagerList().
				ENABLE_ACCESS_TO_CHILDREN(DataManagerAbstract,DataManager,dataManager);
			
				hardware::hardware_sensorext_ptr_t hardwareSensorPtr;
				raw_ptr_t rawPtr;
			
			public:
				
				SensorExteroAbstract(const robot_ptr_t & robPtr, const filtered_obj_t inFilter = UNFILTERED):
					SensorAbstract(robPtr, inFilter)
				{
					kind = EXTEROCEPTIVE; 
					rawCounter = 0;
				}

				unsigned rawCounter;

				void setHardwareSensor(hardware::hardware_sensorext_ptr_t hardwareSensorPtr_)
					{ hardwareSensorPtr = hardwareSensorPtr_; }
				virtual void start() { hardwareSensorPtr->start(); }
				
//				virtual int acquireRaw() = 0;
//				virtual raw_ptr_t getRaw() = 0;

				virtual raw_ptr_t getLastProcessedRaw() { raw_ptr_t raw; hardwareSensorPtr->getLastProcessedRaw(raw); return raw; }
				virtual int queryAvailableRaws(RawInfos &infos)
					{ int res = hardwareSensorPtr->getUnreadRawInfos(infos); infos.integrate_all = integrate_all; return res; }
				virtual int queryNextAvailableRaw(RawInfo &info)
					{ return hardwareSensorPtr->getNextRawInfo(info); }
				virtual double getRawTimestamp(unsigned id) { return hardwareSensorPtr->getRawTimestamp(id); } 
				void process(unsigned id);
				void process_fake(unsigned id) { hardwareSensorPtr->getRaw(id, rawPtr); robotPtr()->move_fake(rawPtr->timestamp); rawCounter++; }
		};

	}
}

#endif /* SENSORABSTRACT_H_ */
