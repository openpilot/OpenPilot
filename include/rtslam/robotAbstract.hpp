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

#include <list>

#include <jmath/jblas.hpp>
#include "rtslam/blocks.hpp"
#include "rtslam/gaussian.hpp"
// include parents
//#include "rtslam/sensorAbstract.hpp"
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
				Control(size_t _size) :
					Gaussian(_size) {
					dt = 1.0;
				}
		};

		/** Base class for all robots defined in the module
		 * rtslam.
		 *
		 * \ingroup rtslam
		 */
		class RobotAbstract {
			public:

				// Mandatory virtual destructor.
				virtual ~RobotAbstract(void) {
				}

				std::size_t id;
				std::string name;
				std::string type;

				std::list<SensorAbstract*> sensorsList;
				MapAbstract * map;

				Gaussian state;
				Gaussian pose;
				Control control;

				/*
				 * Jacobians
				 * F_r: wrt state
				 * F_u: wrt control
				 */
				jblas::mat F_r;
				jblas::mat F_u;

				/**
				 * Sizes constructor
				 * \param size_state the state size
				 * \param size_control the control vector size
				 */
				RobotAbstract(size_t size_state, size_t size_control);

				/**
				 * Remote constructor from remote map and size of control vector
				 * \param _map the remote map
				 * \param _iar the indirect array pointing to the remote storage
				 * \param _size_control the size of the control vector
				 */
				RobotAbstract(MapAbstract & _map, jblas::ind_array & _iar, size_t _size_control);

				inline void set_type(std::string _type) {
					type = _type;
				}
				inline void set_name(std::string _name) {
					name = _name;
				}

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


				size_t getNextId(void);

				/**
				 * Operator << for class RobotAbstract.
				 * It shows information of the robot.
				 */
				friend std::ostream& operator <<(std::ostream & s, jafar::rtslam::RobotAbstract & rob) {
					s << "ROBOT " << rob.id << ": " << rob.name << " of type " << rob.type << std::endl;
					s << ".pose: " << rob.pose << std::endl;
					s << ".state:" << rob.state << std::endl;
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
