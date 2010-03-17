/**
 * robotConstantVelocity.cpp
 *
 *  Created on: 07/03/2010
 *      Author: jsola
 *
 *  \file robotConstantVelocity.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/robotConstantVelocity.hpp"
#include "jmath/jblas.hpp"
#include "boost/numeric/ublas/operation.hpp"
#include "boost/numeric/ublas/matrix_proxy.hpp"
#include "boost/numeric/ublas/vector_proxy.hpp"
#include "rtslam/quatTools.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;


		/**
		 * Remote constructor from remote map.
		 * \param _map the remote map
		 */
		Robot3DConstantVelocity::Robot3DConstantVelocity(MapAbstract & _map) :
			RobotAbstract(_map, Robot3DConstantVelocity::size(), Robot3DConstantVelocity::size_control()) {
			// Build constant perturbation Jacobian
			jblas::identity_mat I(3);
			F_u.clear();
			ublas::subrange(F_u, 7, 10, 0, 3) = I;
			ublas::subrange(F_u, 10, 13, 3, 6) = I;
			type("Constant-Velocity");
		}

		void Robot3DConstantVelocity::move(void) {

			using namespace jblas;
			using namespace ublas;


			/*
			 * This motion model is defined by
			 * - p = p + v*dt
			 * - q = q**(w*dt)    <-- ** : quaternion product
			 * - v = v + vi       <-- vi : impulse in linear velocity
			 * - w = w + wi       <-- wi : impulse in angular velocity
			 * -----------------------------------------
			 *
			 * The Jacobian F_r is built with
			 *          p    q     v     w    |
			 *       -------------------------+---
			 * F_r = [ I_3        P_v       ] | p
			 *       [      Q_q         Q_w ] | q
			 *       [            I_3       ] | v
			 *       [                  I_3 ] | w
			 * -----------------------------------------
			 *
			 * The Jacobian F_u is built with
			 *          vi     wi   |
			 *       ---------------+---
			 * F_u = [            ] | p
			 *       [            ] | q
			 *       [ I_3        ] | v
			 *       [        I_3 ] | w
			 * this Jacobian is however constant and is computed once at Construction time.
			 * -----------------------------------------
			 */

			// split robot state vector
			vec3 p, v, w;
			vec4 q;
			splitState(p, q, v, w);
			double dt = control.dt;


			// Non-trivial Jacobian blocks
			mat P_v(3, 3);
			mat Q_wdt(4, 3);
			mat Q_q(4, 4);
			identity_mat I_3(3);


			// predict each part of the state, give or build non-trivial Jacobians
			p += v * dt;
			P_v = I_3 * dt;
			vec4 q_old(q);
			quaternion::qProd(q_old, w * dt, q, Q_q, Q_wdt);
			// v and w are constant velocity ---> do not predict!

			// Compose state
			composeState(p, q, v, w);


			// Build transition Jacobian matrix F_r
			F_r.assign(identity_mat(state.size()));
			project(F_r, range(0, 3), range(7, 10)) = P_v;
			project(F_r, range(3, 7), range(3, 7)) = Q_q;
			project(F_r, range(3, 7), range(10, 13)) = Q_wdt * dt;


			// Build control Jacobian matrix F_u
			// NOTE: F_u is constant and it has been build in the constructor.
			// NOTE: These lines below just for reference:
			// F_u.clear();
			// project(F_u, range(7,10), range(0,3)) = I_3;
			// project(F_u, range(10,13), range(3,6)) = I_3;

		}

	}
}
