/*
 * \file robotConstantVelocity.hpp
 *
 * Header file for the robot with constant velocity motion model.
 *
 *  Created on: 14/02/2010
 *     \author: jsola
 *
 * \ingroup rtslam
 */

#ifndef ROBOTCONSTANTVELOCITY_HPP_
#define ROBOTCONSTANTVELOCITY_HPP_

#include "rtslam/robotAbstract.hpp"

namespace jafar {
	namespace rtslam {


		/**
		 * Constant velocity model robot class.
		 *
		 * \author jsola@laas.fr
		 *
		 * This class implements a rigid frame in 3D moving with a constant velocity motion model. This model is the following:\n
		 * - p += v*dt <-- position\n
		 * - q = q**(w*dt) <-- orientation quaternion\n
		 * - v += vi <-- linear velocity\n
		 * - w += wi <-- angular velocity
		 *
		 * where \a vi and \a wi are linear and angular velocity random impulses,
		 * and ** is the quaternion product.
		 *
		 * This model is embedded into the system variables as follows:
		 * - the prediction function is x+ = f(x,u), implemented with method move().
		 * - the state vector is x = state.x = [p q v w]
		 * - the state covariance is in the map, P = state.P = (map.filter).P(state.ia, state.ia)
		 * - the control vector is u = control.x = [vi wi]
		 * - the control covariances matrix is U = control.P
		 * - the Jacobians of f(x,u) provided by move() are XNEW_x and XNEW_control.
		 * - the perturbation covariance is obtained with the method computeStatePerturbation(), as follows:
		 * 		- Q = XNEW_control * control.P * XNEW_control'\n
		 * .
		 *
		 * \sa Explore the comments in file robotConstantVelocity.cpp for full algebraic details.
		 *
		 * \ingroup rtslam
		 */
		class RobotConstantVelocity: public RobotAbstract {
			public:


				/**
				 * Remote constructor from remote map.
				 * \param _map the remote map
				 */
				RobotConstantVelocity(MapAbstract & _map);

				~RobotConstantVelocity(void) {
				}


				/**
				 * Move one step ahead.
				 *
				 * This function predicts the robot state one step of length \a dt ahead in time,
				 * according to the control input \a control.x and the time interval \a control.dt.
				 * It updates the state and computes the convenient Jacobian matrices.
				 */
				void move_func();

				void computeControlJacobian();


				static size_t size() {
					return 13;
				}

				static size_t size_control() {
					return 6;
				}

			protected:
				/**
				 * Split state vector.
				 *
				 * Extracts \a p, \a q, \a v and \a w from the state vector, \a x = [\a p, \a q, \a v, \a w].
				 * \param p the position
				 * \param q the quaternion
				 * \param v the linear velocity
				 * \param w the angular velocity
				 */
				template<class Vp, class Vq, class Vv, class Vw>
				inline void splitState(Vp & p, Vq & q, Vv & v, Vw & w) {
					p = ublas::subrange(state.x(), 0, 3);
					q = ublas::subrange(state.x(), 3, 7);
					v = ublas::subrange(state.x(), 7, 10);
					w = ublas::subrange(state.x(), 10, 13);
				}


				/**
				 * Compose state vector.
				 *
				 * Composes the state vector with \a p, \a q, \a v and \a w, \a x = [\a p, \a q, \a v, \a w].
				 * \param p the position
				 * \param q the quaternion
				 * \param v the linear velocity
				 * \param w the angular velocity
				 */
				template<class Vp, class Vq, class Vv, class Vw>
				inline void unsplitState(const Vp & p, const Vq & q, const Vv & v, const Vw & w) {
					ublas::subrange(state.x(), 0, 3) = p;
					ublas::subrange(state.x(), 3, 7) = q;
					ublas::subrange(state.x(), 7, 10) = v;
					ublas::subrange(state.x(), 10, 13) = w;
				}


				/**
				 * Split control vector.
				 *
				 * Extracts impulses \a vi and \a wi from the control vector.
				 * \param vi the linear impulse.
				 * \param wi the angular impulse.
				 */
				template<class V>
				inline void splitControl(V & vi, V & wi) {
					vi = project(control.x(), ublas::range(0, 3));
					wi = project(control.x(), ublas::range(3, 6));
				}

			private:
				// temporary matrices to speed up Jacobian computation
				mat33 PNEW_v; ///<      Temporary Jacobian matrix
				mat43 QWDT_wdt; ///< Temporary Jacobian matrix
				mat44 QNEW_qwdt; ///<   Temporary Jacobian matrix
				mat43 QNEW_wdt; ///<    Temporary Jacobian matrix
				mat44 QNEW_q; ///<      Temporary Jacobian matrix

		};
	}
}

#endif /* ROBOTCONSTANTVELOCITY_HPP_ */
