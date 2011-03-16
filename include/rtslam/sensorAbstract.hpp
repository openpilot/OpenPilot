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
				// define the function linkToParentRobot().
				ENABLE_LINK_TO_PARENT(RobotAbstract,Robot,SensorAbstract);
				// define the functions robotPtr() and robot().
				ENABLE_ACCESS_TO_PARENT(RobotAbstract,robot);
				
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

//				virtual RawInfos queryAvailableRaws() = 0; ///< get information about the available raws and the estimated dates for next one
				virtual void process(unsigned id) = 0; ///< process the given raw and throw away the previous unprocessed ones

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
			public:
				SensorProprioAbstract(const robot_ptr_t & robPtr, const filtered_obj_t inFilter = UNFILTERED):
				  SensorAbstract(robPtr, inFilter) { kind = PROPRIOCEPTIVE; }

				void setHardwareSensor(hardware::hardware_sensorprop_ptr_t hardwareSensorPtr_)
					{ hardwareSensorPtr = hardwareSensorPtr_; }
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
			
			public:
				
				SensorExteroAbstract(const robot_ptr_t & robPtr, const filtered_obj_t inFilter = UNFILTERED):
					SensorAbstract(robPtr, inFilter)
				{
					kind = EXTEROCEPTIVE; 
					rawCounter = 0;
				}

				raw_ptr_t rawPtr;
				unsigned rawCounter;

				void setHardwareSensor(hardware::hardware_sensorext_ptr_t hardwareSensorPtr_)
					{ hardwareSensorPtr = hardwareSensorPtr_; }
				
				virtual int acquireRaw() = 0;
				virtual raw_ptr_t getRaw() = 0;

				void process(unsigned id);


		};

	}
}

#endif /* SENSORABSTRACT_H_ */
