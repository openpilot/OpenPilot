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

#include "jmath/jblas.hpp"
#include "jmath/ublasExtra.hpp"
#include "boost/numeric/ublas/operation.hpp"
#include "boost/numeric/ublas/matrix_proxy.hpp"
#include "boost/numeric/ublas/vector_proxy.hpp"
#include "rtslam/quatTools.hpp"

#include "rtslam/robotAbstract.hpp"
#include "rtslam/robotConstantVelocity.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;
		using namespace jmath;


		/**
		 * Remote constructor from remote map.
		 * \param _map the remote map
		 */
		RobotConstantVelocity::RobotConstantVelocity(MapAbstract & _map) :
			RobotAbstract(_map, RobotConstantVelocity::size(), RobotConstantVelocity::size_control()) {
			// Build constant perturbation Jacobian
			constantPerturbation = true;
			computeControlJacobian();
			type("Constant-Velocity");
		}

		void RobotConstantVelocity::move_func(const vec & _x, const vec & _u, const double _dt, vec & _xnew, mat & _XNEW_x, mat & _XNEW_u) {

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
			 * The Jacobian XNEW_x is built with
			 * 						 p     q      v      w      |
			 *						 0     3      7      10     |
			 *      	--------------------------------+------
			 * XNEW_x = [ I_3         PNEW_v        ] | 0  p
			 *       		[      QNEW_q        QNEW_w ] | 3  q
			 *       		[              I_3          ] | 7  v
			 *       		[                      I_3  ] | 10 w
			 * -----------------------------------------------------------------------------
			 *
			 * The Jacobian XNEW_control is built with
			 *          				 vi     wi   |
			 *                   0      3    |
			 *       				-----------------+------
			 * XNEW_control = [            ] | 0  p
			 * 								[            ] | 3  q
			 * 								[ I_3        ] | 7  v
			 * 								[        I_3 ] | 10 w
			 * this Jacobian is however constant and is computed once at Construction time.
			 *
			 * NOTE: The also constant perturbation matrix:
			 *    Q = XNEW_control * control.P * trans(XNEW_control)
			 * could be built also once after construction with computeStatePerturbation().
			 * This is up to the user -- if nothing is done, Q will be computed at each iteration.
			 * -----------------------------------------------------------------------------
			 */

			// split robot state vector
			vec3 p, v, w;
			vec4 q;
			splitState(_x, p, q, v, w);
			double dt = control.dt;


			// split control vector
			vec3 vi, wi;
			splitControl(_u, vi, wi);


			// Non-trivial Jacobian blocks
			identity_mat I_3(3);

			vec3 pnew, vnew, wnew;
			vec4 qnew;

			// predict each part of the state, give or build non-trivial Jacobians
			pnew = p + v * dt;
			PNEW_v = I_3 * dt;
			vec4 qwdt;
			quaternion::v2q(w * dt, qwdt, QWDT_wdt);
			quaternion::qProd(q, qwdt, qnew, QNEW_q, QNEW_qwdt);
			QNEW_wdt = prod(QNEW_qwdt, QWDT_wdt);
			vnew = v + vi;
			wnew = w + wi;


			// Compose state - this is the output state.
			unsplitState(pnew, qnew, vnew, wnew, _xnew);


			// Build transition Jacobian matrix XNEW_x
			_XNEW_x.assign(identity_mat(state.size()));
			project(_XNEW_x, range(0, 3), range(7, 10)) = PNEW_v;
			project(_XNEW_x, range(3, 7), range(3, 7)) = QNEW_q;
			project(_XNEW_x, range(3, 7), range(10, 13)) = QNEW_wdt * dt;


			/*
			 * Build control Jacobian matrix XNEW_control.
			 * NOTE: XNEW_control is constant and it has been build in the constructor.
			 *
			 * The control Jacobian is
			 *
			 * var    |  vi  wi
			 *    pos |  0   3
			 * -------+--------
			 *  p  0  |  0   0
			 *  q  3  |  0   0
			 *  v  7  |  I   0
			 *  w  10 |  0   I
			 *
			 * NOTE: These lines below just for reference:
			 *
			 * XNEW_control.clear();
			 * project(XNEW_control, range(7,10), range(0,3)) = I_3;
			 * project(XNEW_control, range(10,13), range(3,6)) = I_3;
			 */

		}

		void RobotConstantVelocity::computeControlJacobian() {
			identity_mat I(3);
			XNEW_control.clear();
			subrange(XNEW_control, 7, 10, 0, 3) = I;
			subrange(XNEW_control, 10, 13, 3, 6) = I;
		}


	}
}
