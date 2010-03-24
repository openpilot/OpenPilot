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
#include "rtslam/robotAbstract.hpp"

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
			dxnew_by_dcontrol.clear();
			ublas::subrange(dxnew_by_dcontrol, 7, 10, 0, 3) = I;
			ublas::subrange(dxnew_by_dcontrol, 10, 13, 3, 6) = I;
			RobotAbstract::computeStatePerturbation();
			type("Constant-Velocity");
		}

		void Robot3DConstantVelocity::move() {

			using namespace jblas;
			using namespace ublas;


			/*
			 * This motion model is defined by:
			 * The state vector, x = [p q v w] = [x y z, qw qx qy qz, vx vy vz, wx wy wz], of size 13.
			 * The transition equation x+ = move(x,i), with i = [vi wi] the control impulse, decomposed as:
			 * - p = p + v*dt
			 * - q = q**(w*dt)    <-- ** : quaternion product
			 * - v = v + vi       <-- vi : impulse in linear velocity  - vi = [vix viy viz]
			 * - w = w + wi       <-- wi : impulse in angular velocity - wi = [wix wiy wiz]
			 * -----------------------------------------------------------------------------
			 *
			 * The Jacobian dx_by_dstate is built with
			 *				           p    q     v     w    |
			 *				           0    3     7     10   |
			 *      				---------------------------+------
			 * dx_by_dstate = [ I_3        P_v       ] | 0  p
			 *       					[      Q_q         Q_w ] | 3  q
			 *       					[            I_3       ] | 7  v
			 *       					[                  I_3 ] | 10 w
			 * -----------------------------------------------------------------------------
			 *
			 * The Jacobian dx_by_dcontrol is built with
			 *          					 vi     wi   |
			 *                     0      3    |
			 *       					-----------------+------
			 * dx_by_dcontrol = [            ] | 0  p
			 *                  [            ] | 3  q
			 *       						[ I_3        ] | 7  v
			 *       						[        I_3 ] | 10 w
			 * this Jacobian is however constant and is computed once at Construction time.
			 *
			 * NOTE: The also constant perturbation matrix:
			 *    Q = dx_by_dcontrol * control.P * trans(dx_by_dcontrol)
			 * could be built also once after construction with initStatePerturbation().
			 * This is up to the user -- if nothing is done, Q will be computed at each iteration.
			 * -----------------------------------------------------------------------------
			 */

			// split robot state vector
			vec3 p, v, w;
			vec4 q;
			splitState(p, q, v, w);
			double dt = control.dt;


			// split control vector
			vec3 vi, wi;
			splitControl(vi, wi);


			// Non-trivial Jacobian blocks
			mat P_v(3, 3);
			vec4 qwdt;
			mat QWDT_wdt(4, 3);
			mat Q_qwdt(4, 4);
			mat Q_wdt(4, 3);
			mat Q_q(4, 4);
			identity_mat I_3(3);


			// predict each part of the state, give or build non-trivial Jacobians
			p += v * dt;
			P_v = I_3 * dt;
			vec4 q_old(q);
			quaternion::v2q(w * dt, qwdt, QWDT_wdt);
			quaternion::qProd(q_old, qwdt, q, Q_q, Q_qwdt);
			Q_wdt = ublas::prod(Q_qwdt, QWDT_wdt);
			v += vi;
			w += wi;

			// Compose state - this is the output state.
			composeState(p, q, v, w);

			// Build transition Jacobian matrix dx_by_dstate
			dxnew_by_dx.assign(identity_mat(state.size()));
			project(dxnew_by_dx, range(0, 3), range(7, 10)) = P_v;
			project(dxnew_by_dx, range(3, 7), range(3, 7)) = Q_q;
			project(dxnew_by_dx, range(3, 7), range(10, 13)) = Q_wdt * dt;


			// Build control Jacobian matrix dx_by_dcontrol
			// NOTE: dx_by_dcontrol is constant and it has been build in the constructor.
			// NOTE: These lines below just for reference:
			// dx_by_dcontrol.clear();
			// project(dx_by_dcontrol, range(7,10), range(0,3)) = I_3;
			// project(dx_by_dcontrol, range(10,13), range(3,6)) = I_3;

		}

		void Robot3DConstantVelocity::initStatePerturbation() {
			RobotAbstract::computeStatePerturbation();
		}

	}
}
