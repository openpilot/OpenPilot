/**
 * \file robotInertial.cpp
 *
 * \date 26/03/2010
 * \author jsola@laas.fr
 *
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "jmath/jblas.hpp"

//#include "rtslam/robotAbstract.hpp"
#include "rtslam/robotInertial.hpp"

#include "rtslam/quatTools.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;
		using namespace jblas;
		using namespace ublas;
		using namespace quaternion;


		/**
		 * Remote constructor from remote map.
		 * \param _map the remote map
		 */
		RobotInertial::RobotInertial(const map_ptr_t & _mapPtr) :
			RobotAbstract(_mapPtr, RobotInertial::size(), RobotInertial::size_control(), RobotInertial::size_perturbation()) {
			constantPerturbation = false;
			type = INERTIAL;
		}
		RobotInertial::RobotInertial(const simulation_t dummy, const map_ptr_t & _mapPtr) :
			RobotAbstract(FOR_SIMULATION, _mapPtr, RobotInertial::size(), RobotInertial::size_control(), RobotInertial::size_perturbation()) {
			constantPerturbation = true;
			type = INERTIAL;
		}


		/*
		 * This motion model is driven by IMU measurements and random perturbations, and defined by:
		 * The state vector, x = [p q v ab wb g] , of size 19.
		 *
		 * The transition equation is
		 * - x+ = move_func(x,u,n),
		 *
		 * with u = [am, wm] the IMU measurements (the control input)
		 *  and n = [vi, ti, abi, wbi] the perturbation impulse.
		 *
		 * the transition equation f() is decomposed as:
		 * #ifdef AVGSPEED
		 * - p+  = p + (v + v+)/2*dt
		 * #else
		 * - p+  = p + v*dt
		 * #endif
		 * - q+  = q**((wm - wb)*dt + ti)           <-- ** : quaternion product ; ti : theta impulse
		 * - v+  = v + (R(q)*(am - ab) + g)*dt + vi <-- am and wm: IMU measurements ; vi : v impulse
		 * - ab+ = ab + abi                         <-- abi : random walk in acc bias with abi impulse perturbation
		 * - wb+ = wb + wbi                         <-- wbi : random walk of gyro bias with wbi impulse perturbation
		 * - g+  = g                                <-- g  : gravity vector, constant but unknown
		 * -----------------------------------------------------------------------------
		 *
		 * The Jacobian XNEW_x is built with
		 *   var    |  p       q        v        ab       wb       g
		 *      pos |  0       3        7        10       13       16
		 *   -------+----------------------------------------------------
		 * #ifdef AVGSPEED
		 *   p   0  |  I  VNEW_q*dt/2  I*dt -R*dt*dt/2    0     I*dt*dt/2
		 * #else
		 *   p   0  |  I       0       I*dt      0        0        0
		 * #endif
		 *   q   3  |  0     QNEW_q     0        0      QNEW_wb    0
		 *   v   7  |  0     VNEW_q     I      -R*dt      0       I*dt
		 *   ab  10 |  0       0        0        I        0        0
		 *   wb  13 |  0       0        0        0        I        0
		 *   g   16 |  0       0        0        0        0        I
		 * -----------------------------------------------------------------------------
		 *
		 * The Jacobian XNEW_pert is built with
		 *   var    |  vi    ti   abi  wbi
		 *      pos |  0     3     6     9
		 *   -------+----------------------
		 * #ifdef AVGSPEED
		 *   p   0  |I.dt/2  0     0     0
		 * #else
		 *   p   0  |  0     0     0     0
		 * #endif
		 *   q   3  |  0  QNEW_ti  0     0
		 *   v   7  |  I     0     0     0
		 *   ab  10 |  0     0     I     0
		 *   wb  13 |  0     0     0     I
		 *   g   16 |  0     0     0     0
		 * -----------------------------------------------------------------------------
		 */
		void RobotInertial::move_func(const vec & _x, const vec & _u, const vec & _n, double _dt, vec & _xnew, mat & _XNEW_x,
		    mat & _XNEW_pert) {

			// Separate things out to make it clearer
			vec3 p, v, ab, wb, g;
			vec4 q;
			splitState(_x, p, q, v, ab, wb, g); // split state vector

			// Split control and perturbation vectors into
			// sensed acceleration and sensed angular rate
			// and noises
			vec3 am, wm, an, wn, ar, wr; // measurements and random walks
			splitControl(_u, am, wm);
			splitPert(_n, an, wn, ar, wr);

			// It is useful to start obtaining a nice rotation matrix and the product R*dt
			Rold = q2R(q);
			Rdt = Rold * _dt;

			// Invert sensor functions. Get true acc. and ang. rates
			// a = R(q)(asens - ab) + g     true acceleration
			// w = wsens - wb               true angular rate
			vec3 atrue, wtrue;
			atrue = prod(Rold, (am - ab + an)) + g; // could have done rotate(q, instead of prod(Rold, ; jac/q is Rold...
			wtrue = wm - wb + wn;

			// Get new state vector
			vec3 pnew, vnew, abnew, wbnew, gnew;
			vec4 qnew;

			// qnew = q x q(w * dt)
			// Keep qwt ( = q(w * dt)) for later use
			vec4 qwdt = v2q(wtrue * _dt);
			qnew = qProd(q, qwdt); //    orientation
			vnew = v + atrue * _dt; //    velocity
			#ifdef AVGSPEED
			pnew = p + (v+vnew)/2 * _dt; //     position
			#else
			pnew = p + v * _dt; //     position
			#endif
			abnew = ab + ar; //          acc bias
			wbnew = wb + wr; //          gyro bias
			gnew = g; //                 gravity does not change
			
			// normalize quaternion
			ublasExtra::normalizeJac(qnew, QNORM_qnew);
			ublasExtra::normalize(qnew);

			// Put it all together - this is the output state
			unsplitState(pnew, qnew, vnew, abnew, wbnew, gnew, _xnew);


			// Now on to the Jacobian...
			// Identity is a good place to start since overall structure is like this
			// var    |  p       q        v        ab       wb       g
			//    pos |  0       3        7        10       13       16
			// -------+----------------------------------------------------
			//#ifdef AVGSPEED
			// p   0  |  I  VNEW_q*dt/2  I*dt -R*dt*dt/2    0     I*dt*dt/2
			//#else
			// p   0  |  I       0       I*dt      0        0        0
			//#endif
			// q   3  |  0     QNEW_q     0        0      QNEW_wb    0
			// v   7  |  0     VNEW_q     I      -R*dt      0       I*dt
			// ab  10 |  0       0        0        I        0        0
			// wb  13 |  0       0        0        0        I        0
			// g   16 |  0       0        0        0        0        I

			_XNEW_x.assign(identity_mat(state.size()));

			// Fill in XNEW_v: VNEW_g and PNEW_v = I * dt
			identity_mat I(3);
			Idt = I * _dt;
			subrange(_XNEW_x, 0, 3, 7, 10) = Idt;
			#ifdef AVGSPEED
			subrange(_XNEW_x, 0, 3, 16, 19) = Idt*_dt/2;
			#endif
			subrange(_XNEW_x, 7, 10, 16, 19) = Idt;

			// Fill in QNEW_q
			// qnew = qold ** qwdt  ( qnew = q1 ** q2 = qProd(q1, q2) in rtslam/quatTools.hpp )
			qProd_by_dq1(qwdt, QNEW_q);
			subrange(_XNEW_x, 3, 7, 3, 7) = prod(QNORM_qnew, QNEW_q);

			// Fill in QNEW_wb
			// QNEW_wb = QNEW_qwdt * QWDT_wdt * WDT_w * W_wb
			//         = QNEW_qwdt * QWDT_w * W_wb
			//         = QNEW_qwdt * QWDT_w * (-1)
			qProd_by_dq2(q, QNEW_qwdt);
			// Here we get the derivative of qwdt wrt wtrue, so we consider dt = 1 and call for the derivative of v2q() with v = w*dt
//			v2q_by_dv(wtrue, QWDT_w);
			v2q_by_dv(wtrue*_dt, QWDT_w); QWDT_w *= _dt;
			QNEW_w = prod ( QNEW_qwdt, QWDT_w);
			subrange(_XNEW_x, 3, 7, 13, 16) = -prod(QNORM_qnew,QNEW_w);

			// Fill VNEW_q
			// VNEW_q = d(R(q)*v) / dq
			rotate_by_dq(q, v, VNEW_q);
			subrange(_XNEW_x, 7, 10, 3, 7) = VNEW_q;
			#ifdef AVGSPEED
			subrange(_XNEW_x, 0, 3, 3, 7) = VNEW_q*_dt/2;
			#endif

			// Fill in VNEW_ab
			subrange(_XNEW_x, 7, 10, 10, 13) = -Rdt;
			#ifdef AVGSPEED
			subrange(_XNEW_x, 0, 3, 10, 13) = -Rdt*_dt/2;
			#endif


			// Now on to the perturbation Jacobian XNEW_pert

			// Form of Jacobian XNEW_pert
			// It is like this:
			// var    |  vi    ti    abi    wbi
			//    pos |  0     3     6     9
			// -------+----------------------
			//#ifdef AVGSPEED
			// p   0  |I.dt/2  0     0     0
			//#else
			// p   0  |  0     0     0     0
			//#endif
			// q   3  |  0   QNEW_ti 0     0
			// v   7  |  I     0     0     0
			// ab  10 |  0     0     I     0
			// wb  13 |  0     0     0     I
			// g   16 |  0     0     0     0

			// Fill in the easy bits first
			_XNEW_pert.clear();
			#ifdef AVGSPEED
			ublas::subrange(_XNEW_pert, 0, 3, 0, 3) = Idt/2;
			#endif
			ublas::subrange(_XNEW_pert, 7, 10, 0, 3) = I;
			ublas::subrange(_XNEW_pert, 10, 13, 6, 9) = I;
			ublas::subrange(_XNEW_pert, 13, 16, 9, 12) = I;

			// Tricky bit is QNEW_ti = d(qnew)/d(ti)
			// Here, ti is the integral of the perturbation, ti = integral_{tau=0}^dt (wn(t) * dtau),
			// with: wn: the angular rate measurement noise
			//       dt: the integration period
			//       ti: the resulting angular impulse
			// The integral of the dynamic equation is:
			// q+ = q ** v2q((wm - wb)*dt + ti)
			// We have: QNEW_ti = QNEW_w / dt
			//    with: QNEW_w computed before.
			// The time dependence needs to be included in perturbation.P(), proportional to perturbation.dt:
			//   U = perturbation.P() = U_continuous_time * dt
			//	with: U_continuous_time expressed in ( rad / s / sqrt(s) )^2 = rad^2 / s^3 <-- yeah, it is confusing, but true.
			//   (Use perturbation.set_P_from_continuous() helper if necessary.)
			//
			subrange(_XNEW_pert, 3, 7, 3, 6) = prod (QNORM_qnew, QNEW_w) * (1 / _dt);
		}

		void RobotInertial::init_func(const vec & _x, const vec & _u, vec & _xnew) {
			
			// Separate things out to make it clearer
			vec3 p, v, ab, wb, g;
			vec4 q;
			splitState(_x, p, q, v, ab, wb, g); // split state vector

			// Split control vector into
			// sensed acceleration and sensed angular rate
			vec3 am, wm;
			splitControl(_u, am, wm);
			
			g = -am;
			
			unsplitState(p, q, v, ab, wb, g, _xnew);
		}
		
		
		void RobotInertial::writeLogHeader(kernel::DataLogger& log) const
		{
			std::ostringstream oss; oss << "Robot " << id();
			log.writeComment(oss.str());
			log.writeLegendTokens("time");
			
			log.writeLegendTokens("x y z");
			log.writeLegendTokens("qw qx qy qz");
			log.writeLegendTokens("yaw pitch roll");
			log.writeLegendTokens("vx vy vz");
			log.writeLegendTokens("axb ayb azb");
			log.writeLegendTokens("vyawb vpitchb vrollb");
			log.writeLegendTokens("gx gy gz");
			
			log.writeLegendTokens("sig_x sig_y sig_z");
			log.writeLegendTokens("sig_qw sig_qx sig_qy sig_qz");
			log.writeLegendTokens("sig_yaw sig_pitch sig_roll");
			log.writeLegendTokens("sig_vx sig_vy sig_vz");
			log.writeLegendTokens("sig_axb sig_ayb sig_azb");
			log.writeLegendTokens("sig_vyawb sig_vpitchb sig_vrollb");
			log.writeLegendTokens("sig_gx sig_gy sig_gz");
		}
		
		void RobotInertial::writeLogData(kernel::DataLogger& log) const
		{
			jblas::vec euler_x(3);
			jblas::sym_mat euler_P(3,3);
			quaternion::q2e(ublas::subrange(state.x(), 3, 7), ublas::project(state.P(), ublas::range(3, 7), ublas::range(3,7)), euler_x, euler_P);
			
			log.writeData(self_time);
			for(int i = 0 ; i < 7 ; ++i) log.writeData(state.x()(i));
			for(int i = 0 ; i < 3 ; ++i) log.writeData(euler_x(2-i));
			for(int i = 7 ; i < 10; ++i) log.writeData(state.x()(i));
			for(int i = 10; i < 13; ++i) log.writeData(state.x()(i));
			for(int i = 13; i < 16; ++i) log.writeData(state.x()(2-(i-13)+13));
			for(int i = 16; i < 19; ++i) log.writeData(state.x()(i));
			
			for(int i = 0 ; i < 7 ; ++i) log.writeData(sqrt(state.P()(i,i)));
			for(int i = 0 ; i < 3 ; ++i) log.writeData(sqrt(euler_P(2-i,2-i)));
			for(int i = 7 ; i < 10; ++i) log.writeData(sqrt(state.P()(i,i)));
			for(int i = 10; i < 13; ++i) log.writeData(sqrt(state.P()(i,i)));
			for(int i = 13; i < 16; ++i) log.writeData(sqrt(state.P()(2-(i-13)+13,2-(i-13)+13)));
			for(int i = 16; i < 19; ++i) log.writeData(sqrt(state.P()(i,i)));
		}

	}
}
