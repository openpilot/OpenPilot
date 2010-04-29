/*
 * \file robotOdometry.hpp
 *
 * Header file for the robot with constant velocity motion model.
 *
 *  Created on: 14/02/2010
 *     \author: agonzale
 *
 * \ingroup rtslam
 */

//TODO : add comments on hpp and cpp

#ifndef ROBOTODOMETRY_HPP_
#define ROBOTODOMETRY_HPP_

#include "rtslam/robotAbstract.hpp"

namespace jafar {
	namespace rtslam {

		class RobotOdometry;
				typedef boost::shared_ptr<RobotOdometry> robodo_ptr_t;


		/**
		 * Odometry motion model robot class.
		 *
		 * \author agonzale@laas.fr
		 *
		 * This class implements a rigid frame in 3D moving with an odometry motion model.
		 * This model performs one step on the pose F of a vehicle, given odometry increments U=[dx,dv].
		 *
		 *
		 * This model is the following:\n
		 * -
		 *
		 * \sa Explore the comments in file robotOdometry.cpp for full algebraic details.
		 *
		 * \ingroup rtslam
		 */
		class RobotOdometry: public RobotAbstract {
			public:


				/**
				 * Remote constructor from remote map.
				 * \param _map the remote map
				 */
				RobotOdometry(const map_ptr_t _mapPtr);

				/**
				 * Remote constructor from remote map, for simulation.
				 * \param dummy flag for simulation. Give value FOR_SIMULATION.
				 * \param _map the remote map
				 */
				RobotOdometry(const simulation_t dummy, const map_ptr_t _mapPtr);

				~RobotOdometry(void) {
				}


				/**
				 * Move one step ahead.
				 *
				 * This function predicts the robot state one step of length \a dt ahead in time,
				 * according to the control input \a control.x and the time interval \a control.dt.
				 *
				 * \param _x the current state vecto
				 * \param _p the perturbation vector
				 * \param _dt the sampling time
				 * \param _xnew the new state vector
				 * \param _XNEW_x the Jacobian of xnew wrt x
				 * \param _XNEW_pert the Jacobian of xnew wrt p
				 */
				void move_func(const vec & _x, const vec & _u, const vec & _n, const double _dt, vec & _xnew, mat & _XNEW_x, mat & _XNEW_u);

				void computePertJacobian();

				static size_t size() {
					return 7;
				}

				static size_t size_control() {
					return 6;
				}

				static size_t size_perturbation() {
					return 6;
				}
				virtual size_t mySize() {return size();}
				virtual size_t mySize_control() {return size_control();}
				virtual size_t mySize_perturbation() {return size_perturbation();}


			protected:
				/**
				 * Split state vector.
				 *
				 * Extracts \a p, \a q, \a v and \a w from the state vector, \a x = [\a p, \a q, \a v, \a w].
				 * \param x the state vector
				 * \param p the position
				 * \param q the quaternion
				 * \param v the linear velocity
				 * \param w the angular velocity
				 */
				template<class Vx, class Vp, class Vq>
				inline void splitState(const Vx x, Vp & p, Vq & q) {
					p = ublas::subrange(x, 0, 3);
					q = ublas::subrange(x, 3, 7);
				}


				/**
				 * Compose state vector.
				 */
				template<class Vp, class Vq, class Vx>
				inline void unsplitState(const Vp & p, const Vq & q, Vx & x) {
					ublas::subrange(x, 0, 3) = p;
					ublas::subrange(x, 3, 7) = q;
				}


				/**
				 * Split control vector.
				 *
				 * Extracts impulses \a vi and \a wi from the control vector.
				 * \param vi the linear impulse.
				 * \param wi the angular impulse.
				 */
				template<class Vu, class V>
				inline void splitControl(Vu & u, V & dx, V & dv) {
					dx = project(u, ublas::range(0, 3));
					dv = project(u, ublas::range(3, 6));
				}

			private:
				// temporary matrices to speed up Jacobian computation
//				mat33 PNEW_v; ///<      Temporary Jacobian matrix
//				mat43 QWDT_wdt; ///< Temporary Jacobian matrix
//				mat44 QNEW_qwdt; ///<   Temporary Jacobian matrix
//				mat43 QNEW_wdt; ///<    Temporary Jacobian matrix
//				mat44 QNEW_q; ///<      Temporary Jacobian matrix

		};
	}
}

#endif /* ROBOTODOMETRY_HPP_ */
