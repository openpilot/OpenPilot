/*
 * \file sensorAbstract.h
 *
 * Header file for the abstract sensor.
 *
 * \date Jan 28, 2010
 * \author jsola@laas.fr
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
		using namespace std;

		// Forward declarations of children
		class ObservationAbstract;
		class DataManagerAbstract;

		//		/**
		//		 * Base class for all parameter sets in module rtslam.
		//		 * \ingroup rtslam
		//		 */
		//		class ParametersAbstract {
		//			public:
		//				/**
		//				 * Mandatory virtual destructor.
		//				 */
		//				inline virtual ~ParametersAbstract(void) {
		//				}
		//
		//		};


		/**
		 * Base class for all sensors defined in the module rtslam.
		 * \ingroup rtslam
		 */
		class SensorAbstract: public MapObject,
		    public ChildOf<RobotAbstract> ,
		    public boost::enable_shared_from_this<SensorAbstract>,
		    public ParentOf<DataManagerAbstract> {

				friend std::ostream& operator <<(std::ostream & s,
				    jafar::rtslam::SensorAbstract & sen);

				// define the function linkToParentRobot().
			ENABLE_LINK_TO_PARENT(RobotAbstract,Robot,SensorAbstract)
				;
				// define the functions robotPtr() and robot().
			ENABLE_ACCESS_TO_PARENT(RobotAbstract,robot)
				;
				// define the type DataManagerList, and the function dataManagerList().
			ENABLE_ACCESS_TO_CHILDREN(DataManagerAbstract,DataManager,dataManager)
				;

				hardware_sensor_ptr_t hardwareSensorPtr;

			public:

				enum type_enum {
					PINHOLE, BARRETO
				};
				type_enum type;

				/**
				 * Selectable LOCAL or REMOTE pose constructor.
				 * Creates a sensor with the pose indexed in a map.
				 * \param _rob the robot
				 * \param inFilter flag indicating if the sensor state is part of the filter (REMOTE).
				 */
				SensorAbstract(const robot_ptr_t & _robPtr,
				    const filtered_obj_t inFilter = UNFILTERED);

				/**
				 * Selectable LOCAL or REMOTE pose constructor.
				 * Creates a sensor with the pose indexed in a map.
				 * \param dummy a marker for simulation. Give value ObjectAbstract::FOR_SIMULATION.
				 * \param _rob the robot
				 */
				SensorAbstract(const simulation_t dummy, const robot_ptr_t & _robPtr);

				/**
				 * Mandatory virtual destructor.
				 */
				virtual ~SensorAbstract() {
				}

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

				static IdFactory sensorIds;
				void setId(){id(sensorIds.getId());}

				/**
				 * Sensor pose in robot
				 */
				Gaussian pose;

				/**
				 * Indices of sensor's global pose in map (this is either the \a ia of the robot, \a rob.ia to make it short, or the \b ia_union() of \a rob.ia and \a sen.ia)
				 */
				ind_array ia_globalPose;
				/**
				 * Flag indicating if the sensor pose is being filtered
				 */
				bool isInFilter;
				raw_ptr_t rawPtr;

			protected:
				/**
				 * Sensor parameters.
				 * Derive this class for real sensors,
				 * and use dynamic down-cast in the associated observation classes to access the derived parameters.
				 */
				//				ParametersAbstract* paramsAbs;

			public:

				void setHardwareSensor(hardware_sensor_ptr_t hardwareSensorPtr_)
				{
					hardwareSensorPtr = hardwareSensorPtr_;
				}
				
				/*
				 * Acquire raw data.
				 */
				virtual int acquireRaw() = 0;
				virtual void releaseRaw() = 0;

				virtual raw_ptr_t getRaw() = 0;


			protected:

			public:

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

	}
}

#endif /* SENSORABSTRACT_H_ */
