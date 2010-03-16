/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Copyright RIA-LAAS/CNRS, 2010
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * File:      robotAbstract.h
 * Project:   RT-Slam
 * Author:    Joan Sola
 *
 * Version control
 * ===============
 *
 *  $Id$
 *
 * Description
 * ============
 *
 *
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/**
 * \file robotAbstract.hpp
 * File defining the abstract robot class
 * \ingroup rtslam
 */

#ifndef __RobotAbstract_H__
#define __RobotAbstract_H__

/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */

#include <map>

#include <jmath/jblas.hpp>
#include "rtslam/gaussian.hpp"
#include "rtslam/mapObject.hpp"
// include parents
#include "rtslam/sensorAbstract.hpp"
#include "rtslam/mapAbstract.hpp"

namespace jafar {

	namespace rtslam {


		//  Forward declarations of children
		class SensorAbstract;
		//		class MapAbstract;

		/** Base class for all Gaussian control vectors defined in the module rtslam.
		 *
		 * @ingroup rtslam
		 */
		class Control: public Gaussian {
			public:
				double dt;
				Control(const size_t _size) :
					Gaussian(_size) {
					dt = 1.0;
				}
		};


		/** Base class for all robots defined in the module
		 * rtslam.
		 *
		 * \ingroup rtslam
		 */
		class RobotAbstract: public MapObject {
			public:


				/**
				 * Remote constructor from remote map and size of state and control vectors.
				 * \param _map the map.
				 * \param _size_state the size of the robot state vector
				 * \param _size_control the size of the control vector
				 */
				RobotAbstract(MapAbstract & _map, const size_t _size_state, const size_t _size_control);

				// Mandatory virtual destructor.
				virtual ~RobotAbstract(void) {
				}

				/**
				 * Add a sensor to this robot
				 */
				void addSensor(SensorAbstract * _senPtr);

				/**
				 * A set of sensors
				 */
				typedef std::map<size_t, SensorAbstract*> sensors_t;
				sensors_t sensors;

				/**
				 * Parent map
				 */
				MapAbstract * map;

				Gaussian pose; ///< Robot pose
				Control control; ///< Control vector

				/*
				 * Jacobians
				 * F_r: wrt state
				 * F_u: wrt control
				 */
				jblas::mat F_r;
				jblas::mat F_u;

				/**
				 * Acquire control structure
				 */
				virtual void set_control(const Control & _control) {
					control = _control;
				}


				/**
				 * Move the robot.
				 */
				virtual void move(void) = 0;

				void move(const Control & _control) {
					set_control(_control);
					move();
				}

				void move(const jblas::vec & _u) {
					JFR_ASSERT(_u.size() == control.size(), "robotAbstract.hpp: move: wrong control size.");
					control.x(_u);
					move();
				}

				static size_t size_control(void) {
					return 0;
				}


				/**
				 * Operator << for class RobotAbstract.
				 * It shows different information of the robot.
				 */
				friend std::ostream& operator <<(std::ostream & s, jafar::rtslam::RobotAbstract & rob) {
					s << rob.categoryName() << " " << rob.id() << ": ";
					if (rob.name().size() > 0)
						s << rob.name() << ", ";
					s << "of type " << rob.type() << std::endl;
					s << ".state:  " << rob.state << std::endl;
					s << ".pose :  " << rob.pose << std::endl;
					s << ".sens << [";
					sensors_t::iterator senIter;
					for (senIter = rob.sensors.begin(); senIter != rob.sensors.end(); senIter++)
						s << senIter->first << " ";
					s << "]" << std::endl;
					return s;
				}

		};

	}
}

#endif // #ifndef __RobotAbstract_H__
/*
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * tab-width: 2
 * c-basic-offset: 2
 * End:
 */
