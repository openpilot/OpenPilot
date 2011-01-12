/**
 * \file robotCenteredConstantVelocity.cpp
 * \date 02/07/2010
 * \author jmcodol
 * \ingroup rtslam
 */

#include "jmath/jblas.hpp"
#include "jmath/ublasExtra.hpp"
#include "boost/numeric/ublas/operation.hpp"
#include "boost/numeric/ublas/matrix_proxy.hpp"
#include "boost/numeric/ublas/vector_proxy.hpp"
#include "rtslam/quatTools.hpp"

#include "rtslam/robotAbstract.hpp"
#include "rtslam/robotCenteredConstantVelocity.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;
		using namespace jmath;

		/**
		 * Remote constructor from remote map.
		 * \param _map the remote map
		 */
		RobotCenteredConstantVelocity::RobotCenteredConstantVelocity(const map_ptr_t & _mapPtr) :
			RobotAbstract(_mapPtr, RobotCenteredConstantVelocity::size(),
			              RobotCenteredConstantVelocity::size_control(),
			              RobotCenteredConstantVelocity::size_perturbation()) {
			// Build constant perturbation Jacobian
			constantPerturbation = true;
			computePertJacobian();
			type = CENTERED_CONSTANT_VELOCITY;
		}

		RobotCenteredConstantVelocity::RobotCenteredConstantVelocity(const simulation_t dummy,
		    const map_ptr_t & _mapPtr) :
			RobotAbstract(FOR_SIMULATION, _mapPtr, RobotCenteredConstantVelocity::size(),
			              RobotCenteredConstantVelocity::size_control(),
			              RobotCenteredConstantVelocity::size_perturbation()) {
			// Build constant perturbation Jacobian
			constantPerturbation = true;
			computePertJacobian();
			type = CENTERED_CONSTANT_VELOCITY;
		}


		void RobotCenteredConstantVelocity::getFrameForReframing(vec7 & frame) {
			frame = ublas::subrange(state.x(), 0, 7) ;
		}

		void RobotCenteredConstantVelocity::move_func(
				const vec & _x, const vec & _u,
		    const vec & _n, const double _dt,
		    vec & _xnew, mat & _XNEW_x,
		    mat & _XNEW_u) {

			using namespace jblas;
			using namespace ublas;

			/*
       *
			 * The state vector, x = [p q v w pBase qBase] = [x y z, qw qx qy qz, vx vy vz, wx wy wz, xb yb zb, qwb qxb qyb qzb], of size 20.
			 * The transition equation x+ = move(x,i), with i = [vi wi] the control impulse, decomposed as:
       *
			 * This motion model is defined by 2 steps :
			 * -----------------------------------------
			 *  1) We perform a frame transform of all the map :
			 *   -a) The Landmarks
			 *         From frame F0 to frame [p,q]
			 *   -b) The Base frame
			 *         From frame F0 to frame [p,q]
			 *   -c) The Current robot pose become (we don't have to code this part because we will update the pose in step 2)
			 *         p=0
			 *         q=[1,0,0,0]'
			 *         v=v
			 *         w=w
			 *
			 *  2) We update the current pose with a constant velocity model :
			 *   * p = v*dt
			 *   * q = w2q(w*dt) <-- w2q : 3 angles to quaternion
			 *   * v = v + vi    <-- vi  : impulse in linear velocity  - vi = [vix viy viz]
			 *   * w = w + wi    <-- wi  : impulse in angular velocity - wi = [wix wiy wiz]
			 *
			 * -----------------------------------------------------------------------------
			 *
			 * We have some jacobians :
			 * The Jacobian XNEW_x is built with
			 *
			 *              _______________________ROBOT STATE______________________________
			 *             /                                                                \
			 * 						    p             q       v     w         pBase          qBase             lold        |
			 *						    0             3       7     10        13             16                20          |
			 *      	-------------------------------------------------------------------------------------------+------
       *             ____________________________________________________________________
			 *  XNEW_x = [ |                      PNEW_v                                      |                ] |  0  p     \|
			 *           [ |                           QNEW_w                                 |                ] |  3  q      |
			 *           [ |                      I_3                                         |                ] |  7  v      | Robot State
			 *           [ |                            I_3                                   |                ] |  10 w      |
			 *           [ | [-----PBASENEW_f----]              PBASENEW_pbase                |                ] |  13 pBase  |
			 *           [ |            QBASENEW_q                             QBASENEW_qbase |                ] |  16 qBase /|
			 *           [ |__________________________________________________________________|                ] |
			 *           [  LNEW_p     LNEW_q                                                     LNEW_lold    ] |  20 lnew
			 *
			 * -----------------------------------------------------------------------------
			 *
			 * The Jacobian XNEW_pert is built with
			 *          			 vi     wi   |
			 *                 0      3    |
			 *       			-----------------+------
			 *                ____________
			 * XNEW_pert = 	[|            |] | 0  p     \|
			 * 					   	[|            |] | 3  q      |
			 * 							[| I_3        |] | 7  v      | Robot State
			 * 							[|        I_3 |] | 10 w      |
			 * 							[|            |] | 7  pBase  |
			 * 							[|____________|] | 10 qBase /|
			 * 							[              ] | 7  lnew
			 * this Jacobian is however constant and is computed once at Construction time.
			 *
			 * NOTE: The also constant perturbation matrix:
			 *    Q = XNEW_pert * perturbation.P * trans(XNEW_pert)
			 * could be built also once after construction with computeStatePerturbation().
			 * This is up to the user -- if nothing is done, Q will be computed at each iteration.
			 * -----------------------------------------------------------------------------
			 */

			lastJump = ublas::subrange(_x, 0, 7) ; // will be used in the reframe process of : the robot, the landmarks.


			// TODO the internals jacobians (of fix size)
			//  can be defined in the class definition,
			//  or the RobotAbstract class.

			// used variables :
			// 1- robot at time t
			vec3 p, v, w, pbase;
			vec4 q, qbase;
			// 2- robot at time t+1
			vec3 pnew, vnew, wnew, pbasenew;
			vec4 qnew, qbasenew;
			// 3- jacobians from t to t+1
			mat   PBASENEW_f(3, 7);
			mat33 PBASENEW_pbase ;
			mat44 QBASENEW_q, QBASENEW_qbase;
			identity_mat I_3(3);

			// split robot state vector (F is the reference frame change between t and t+1)
			splitState(_x, p, q, v, w, pbase, qbase);
			lastJump = ublas::subrange(_x, 0, 7) ;

			// split perturbation vector
			vec3 vi, wi;
			splitControl(_n, vi, wi);

			// predict each part of the state, give or build non-trivial Jacobians
			// 1- PQVW at t+1
			pnew      = v   * _dt; // FIXME reframe the new position and quaternion (position R_t relative to frame R_t-1)
			PNEW_v    = I_3 * _dt;
			quaternion::v2q(w * _dt, qnew, QNEW_wdt); // FiXME _w or _wdt
			vnew      = v + vi;
			wnew      = w + wi;
			// 2- PQ-of-Base at t+1 (reframe the base frame)
			quaternion::eucToFrame(lastJump, pbase, pbasenew, PBASENEW_f, PBASENEW_pbase) ; // pbase
			quaternion::qProd(qbase, q, qbasenew, QBASENEW_qbase, QBASENEW_q)      ; // qbase

			// Re-compose state - this is the output state.
			unsplitState(pnew, qnew, vnew, wnew, pbasenew, qbasenew, _xnew); // Robot state

			// Build transition Jacobian matrix XNEW_x
			_XNEW_x.clear() ;
			project(_XNEW_x, range(0 , 3 ), range(7 , 10)) = PNEW_v        ;
			project(_XNEW_x, range(3 , 7 ), range(10, 13)) = QNEW_wdt      ; // FIXME jacobian wrt w or wdt
			project(_XNEW_x, range(7 , 10), range(7 , 10)) = I_3           ;
			project(_XNEW_x, range(10, 13), range(10, 13)) = I_3           ;
			project(_XNEW_x, range(13, 16), range(0 , 7 )) = PBASENEW_f    ;
			project(_XNEW_x, range(13, 16), range(13, 16)) = PBASENEW_pbase;
			project(_XNEW_x, range(16, 20), range(3 , 7 )) = QBASENEW_q    ;
			project(_XNEW_x, range(16, 20), range(16, 20)) = QBASENEW_qbase;

			// NOW the landmarks are defined in the frame F = [{0,0,0, 1,0,0,0} - lastJump],
			// so we will reframe all the landmarks.


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
		 * The Jacobian XNEW_pert is built with
		 *          			 vi     wi   |
		 *                 0      3    |
		 *       			-----------------+------
		 * XNEW_pert = 	[            ] | 0  p     \|
		 * 					   	[            ] | 3  q      |
		 * 							[ I_3        ] | 7  v      | Robot State
		 * 							[        I_3 ] | 10 w      |
		 * 							[            ] | 7  pBase  |
		 * 							[            ] | 10 qBase /|
		 *
		 * NOTE: These lines below just for reference:
		 */
		void RobotCenteredConstantVelocity::computePertJacobian() {
			identity_mat I(3);
			XNEW_pert.clear();
			subrange(XNEW_pert, 7, 10, 0, 3) = I;
			subrange(XNEW_pert, 10, 13, 3, 6) = I;
		}

	}
}
