/*
 * robotConstantVelocity.hpp
 *
 *  Created on: 14/02/2010
 *      Author: jsola
 */

/**
 * \file robotConstantVelocity.hpp
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
		 * This class implements a rigid frame in 3D moving with a constant velocity motion model. This model is the following:\n
		 * - p += v*dt \n
		 * - q = q**(w*dt)\n
		 * - v += vi \n
		 * - w += wi
		 *
		 * where \a vi and \a wi are linear and angular velocity random impulses,
		 * and ** is the quaternion product.
		 *
		 * See robotConstantVelocity.cpp comments for full algebric details.
		 *
		 * \ingroup rtslam
		 */
		class Robot3DConstantVelocity: public RobotAbstract {
			public:

//				/**
//				 * Local constructor
//				 */
//				Robot3DConstantVelocity(void);

			/**
			 * Remote constructor from remote map.
			 * \param _map the remote map
			 * \param _iar the indirect array pointing to the remote storage
			 */
			Robot3DConstantVelocity(MapAbstract & _map, jblas::ind_array & _iar);

			/**
			 * Remote constructor from remote map.
			 * \param _map the remote map
			 */
			Robot3DConstantVelocity(MapAbstract & _map);

				~Robot3DConstantVelocity(void) {
				}

				/**
				 * Move one step ahead.
				 *
				 * This function predicts the robot state one step of length \a dt ahead in time,
				 * according to the control input \a control.
				 * In this case \a control only contains the covariances matrix of the impulses.
				 * All parameters exist in the class and hence the funciton is void.
				 * \sa See other prototypes of move() with additional input parameters in the RobotAbstract class.
				 */
				void move(void);

				static size_t size(void) {
					return 13;
				}

				static size_t size_control(void) {
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
				inline void composeState(const Vp & p, const Vq & q, const Vv & v, const Vw & w) {
					ublas::subrange(state.x(), 0, 3) = p;
					ublas::subrange(state.x(), 3, 7) = q;
					ublas::subrange(state.x(), 7, 10) = v;
					ublas::subrange(state.x(), 10, 13) = w;
				}

			private:

		};
	}
}

#endif /* ROBOTCONSTANTVELOCITY_HPP_ */
