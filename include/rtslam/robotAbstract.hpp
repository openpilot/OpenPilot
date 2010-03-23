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
 * \author: jsola@laas.fr
 *
 * File defining the abstract robot class
 *
 * \ingroup rtslam
 */

#ifndef __RobotAbstract_H__
#define __RobotAbstract_H__

/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */

#include <jmath/jblas.hpp>

#include "rtslam/rtSlam.hpp"
#include "rtslam/gaussian.hpp"
#include "rtslam/mapObject.hpp"
// include parents
#include "rtslam/mapAbstract.hpp"
#include "rtslam/mapObject.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;

		//  Forward declarations of children
		class SensorAbstract;

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

				friend ostream& operator <<(ostream & s, jafar::rtslam::RobotAbstract & rob) ;

			public:

				/**
				 * Remote constructor from remote map and size of state and control vectors.
				 * \param _map the map.
				 * \param _size_state the size of the robot state vector
				 * \param _size_control the size of the control vector
				 */
				RobotAbstract(MapAbstract & _map, const size_t _size_state, const size_t _size_control);

				// Mandatory virtual destructor.
				virtual ~RobotAbstract() {}

				map_ptr_t slamMap; ///< parent map

				sensors_ptr_set_t sensors; ///<	A set of sensors

				Gaussian pose; ///< Robot pose

				Control control; ///< Control Gaussian vector

				jblas::sym_mat Q; ///< Perturbation matrix in state space, Q = dx_by_dcontrol * control.P * trans(dx_by_dcontrol);

				jblas::mat dx_by_dstate; ///< Jacobian wrt state

				jblas::mat dx_by_dcontrol; ///< Jacobian wrt control

				static size_t size_control() {
					return 0;
				}

				static size_t size_pert() {
					return 0;
				}

				/**
				 * Add a sensor to this robot
				 */
				void linkToSensor(sensor_ptr_t _senPtr);

				/**
				 * Link To Map
				 */
				void linkToMap(map_ptr_t _mapPtr);

				/**
				 * Acquire control structure
				 */
				virtual void set_control(const Control & _control) {
					control = _control;
				}

				/**
				 * Move the robot.
				 */
				virtual void move() = 0;

				void move(const Control & _control) {
					set_control(_control);
					move();
				}

				void move(const jblas::vec & _u) {
					JFR_ASSERT(_u.size() == control.size(), "robotAbstract.hpp: move: wrong control size.");
					control.x(_u);
					move();
				}

				/**
				 * Retro-project perturbation to robot state.
				 */
				void computeStatePerturbation(){
					Q = jmath::ublasExtra::prod_JPJt(control.P(),dx_by_dcontrol);
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
