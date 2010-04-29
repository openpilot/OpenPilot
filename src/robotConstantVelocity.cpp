/**
 * robotConstantVelocity.cpp
 *
 * \date 07/03/2010
 * \author jsola@laas.fr
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
		RobotConstantVelocity::RobotConstantVelocity(const map_ptr_t & _mapPtr) :
			RobotAbstract(_mapPtr, RobotConstantVelocity::size(), RobotConstantVelocity::size_control(), RobotConstantVelocity::size_perturbation()) {
			// Build constant perturbation Jacobian
			constantPerturbation = true;
			computePertJacobian();
			type("Constant-Velocity");
		}

		RobotConstantVelocity::RobotConstantVelocity(const simulation_t dummy, const map_ptr_t & _mapPtr) :
			RobotAbstract(FOR_SIMULATION, _mapPtr, RobotConstantVelocity::size(), RobotConstantVelocity::size_control(), RobotConstantVelocity::size_perturbation()) {
			// Build constant perturbation Jacobian
			constantPerturbation = true;
			computePertJacobian();
			type("Constant-Velocity");
		}


		void RobotConstantVelocity::move_func(const vec & _x, const vec & _u, const vec & _n, const double _dt, vec & _xnew, mat & _XNEW_x, mat & _XNEW_u) {

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
			 * The Jacobian XNEW_pert is built with
			 *          			 vi     wi   |
			 *                 0      3    |
			 *       			-----------------+------
			 * XNEW_pert = 	[            ] | 0  p
			 * 					   	[            ] | 3  q
			 * 							[ I_3        ] | 7  v
			 * 							[        I_3 ] | 10 w
			 * this Jacobian is however constant and is computed once at Construction time.
			 *
			 * NOTE: The also constant perturbation matrix:
			 *    Q = XNEW_pert * perturbation.P * trans(XNEW_pert)
			 * could be built also once after construction with computeStatePerturbation().
			 * This is up to the user -- if nothing is done, Q will be computed at each iteration.
			 * -----------------------------------------------------------------------------
			 */

			// split robot state vector
			vec3 p, v, w;
			vec4 q;
			splitState(_x, p, q, v, w);


			// split perturbation vector
			vec3 vi, wi;
			splitControl(_n, vi, wi);


			// Non-trivial Jacobian blocks
			identity_mat I_3(3);

			vec3 pnew, vnew, wnew;
			vec4 qnew;

			// predict each part of the state, give or build non-trivial Jacobians
			pnew = p + v * _dt;
			PNEW_v = I_3 * _dt;
			vec4 qwdt;
			quaternion::v2q(w * _dt, qwdt, QWDT_wdt);
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
			project(_XNEW_x, range(3, 7), range(10, 13)) = QNEW_wdt * _dt;


			/*
			 * We are normally supposed here to build the perturbation Jacobian matrix XNEW_pert.
			 * NOTE: XNEW_pert is constant and it has been build in the constructor.
			 *
			 * \sa See computePertJacobian() for more info.
			 */

		}

		/*
		 * Build perturbation Jacobian matrix XNEW_pert.
		 *
		 * The perturbation Jacobian is
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
		 */
		void RobotConstantVelocity::computePertJacobian() {
			identity_mat I(3);
			XNEW_pert.clear();
			subrange(XNEW_pert, 7, 10, 0, 3) = I;
			subrange(XNEW_pert, 10, 13, 3, 6) = I;
		}

	}
}
