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
 * \author: jsola@laas.fr
 *
 * File defining the abstract robot class
 *
 * \ingroup rtslam
 */

#ifndef __RobotAbstract_H__
#define __RobotAbstract_H__

/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */

#include <jmath/jblas.hpp>
#include "kernel/jafarDebug.hpp"

#include "rtslam/rtSlam.hpp"
#include "rtslam/gaussian.hpp"
#include "rtslam/mapObject.hpp"
// include parents
#include "rtslam/mapAbstract.hpp"
#include "rtslam/mapObject.hpp"
#include "rtslam/parents.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;


		//  Forward declarations of children
		class SensorAbstract;


		/** Base class for all Gaussian perturbation vectors defined in the module rtslam.
		 * \author jsola@laas.fr
		 *
		 * The Perturbation class is mainly a Gaussian. It represents discrete-time perturbation vectors.
		 *
		 * In case the perturbation and perturbation values want to be specified in continuous-time,
		 * this class incorporates private members for storing the continuous values,
		 * and also methods for the conversion to discrete-time.
		 *
		 * @ingroup rtslam
		 */
		class Perturbation: public Gaussian {
			private:
				vec x_ct; ///< continuous-time perturbation vector
				sym_mat P_ct; ///< continuous-time covariances matrix

			public:
				Perturbation(const size_t _size) :
					Gaussian(_size), x_ct(0), P_ct(0) {
				}
				Perturbation(const vec & p, const sym_mat & P) :
					Gaussian(p, P), x_ct(0), P_ct(0) {
				}
				Perturbation(const Gaussian & p) :
					Gaussian(p), x_ct(0), P_ct(0) {
				}
				template<class SymMat>
				void set_P_continuous(SymMat & _P_ct) {
					JFR_ASSERT(_P_ct.size1() == size(), "Matrix sizes mismatch.");
					P_ct.resize(size(), size());
					P_ct = _P_ct;
				}
				template<class V>
				void set_x_continuous(V & _x_ct) {
					JFR_ASSERT(_x_ct.size() == size(), "Vector sizes mismatch.");
					x_ct.resize(size());
					x_ct = _x_ct;
				}
				/**
				 * Discrete perturbation from continuous specification.
				 * - The white, Gaussian random values integrate with the square root of dt. Their variance integrates linearly with dt:
				 *		- P = _P_ct * _dt
				 *
				 * This function takes covariances from the internal variables of the class (which is often constant).
				 * \param _dt the time interval to integrate.
				 */
				void set_P_from_continuous(double _dt) {
					JFR_ASSERT(P_ct.size1() == size(), "Continuous-time covariance not yet initialized.");
					P(P_ct * _dt); // perturbation is random => variance is linear with time
				}
				/**
				 * Discrete perturbation from continuous specification.
				 * - The white, Gaussian random values integrate with the square root of dt. Their variance integrates linearly with dt:
				 *		- P = _P_ct * _dt
				 *
				 * \param _P_ct continuous-time perturbation covariances matrix.
				 * \param _dt the time interval to integrate.
				 */
				void set_P_from_continuous(sym_mat & _P_ct, double _dt) {
					JFR_ASSERT(_P_ct.size1() == size(), "Matrix sizes mismatch.");
					set_P_continuous(_P_ct);
					P(P_ct * _dt); // perturbation is random => variance is linear with time
				}
				/**
				 * Discrete perturbation from continuous specifications.
				 * - The deterministic values integrate with time normally, linearly with dt:
				 * 		- x = _x_ct * _dt
				 * - The white, Gaussian random values integrate with the square root of dt. Their variance integrates linearly with dt:
				 *		- P = _P_ct * _dt
				 *
				 * This function takes mean and covariances from the internal variables of the class (which are often constant).
				 * \param _dt the time interval to integrate.
				 */
				void set_from_continuous(double _dt) {
					JFR_ASSERT(x_ct.size() == size(), "Continuous-time values not yet initialized.");
					x(x_ct * sqrt(_dt)); // perturbation is random => mean is linear with square root of time
					P(P_ct * _dt); // perturbation is random => variance is linear with time
				}
				/**
				 * Discrete perturbation from continuous specifications.
				 * - The variance integrates linearly with dt:
				 *		- P = Pct.P * _dt
				 * - Therefore, the mean values integrate with time linearly with the square root of dt:
				 * 		- x = Pct.x * sqrt(_dt)
				 *
				 * \param Pct a continuous-time Gaussian process noise.
				 * \param _dt the time interval to integrate.
				 */
				void set_from_continuous(Gaussian & Pct, double _dt) {
					JFR_ASSERT(Pct.size() == size(), "Sizes mismatch");
					set_x_continuous(Pct.x());
					set_P_continuous(Pct.P());
					set_from_continuous(_dt);
				}
		};


		/** Base class for all robots defined in the module
		 * rtslam.
		 *
		 * \ingroup rtslam
		 */
		class RobotAbstract: public MapObject, public ChildOf<MapAbstract> , public boost::enable_shared_from_this<
		    RobotAbstract>, public ParentOf<SensorAbstract> {

				friend ostream& operator <<(ostream & s, jafar::rtslam::RobotAbstract & rob);

			public:


				// define the function linkToParentMap().
				ENABLE_LINK_TO_PARENT(MapAbstract,Map,RobotAbstract)
				;
				// define the functions mapPtr() and map().
				ENABLE_ACCESS_TO_PARENT(MapAbstract,map)
				;
				// define the type SensorList, and the function sensorList().
				ENABLE_ACCESS_TO_CHILDREN(SensorAbstract,Sensor,sensor);



				/**
				 * Remote constructor from remote map and size of state and control vectors.
				 * \param _mapPtr a pointer to the map.
				 * \param _size_state the size of the robot state vector
				 * \param _size_control the size of the control vector
				 * \param _size_pert the size of the perturbation vector
				 */
				RobotAbstract(const map_ptr_t & _mapPtr, const size_t _size_state, const size_t _size_control,
				              const size_t _size_pert);

				// Mandatory virtual destructor.
				virtual ~RobotAbstract() {
				}

				Gaussian pose; ///<             Robot Gaussian pose
				vec control; ///<               Control vector
				double dt_or_dx; ///<           Sampling time or any other relevant increment (e.g. odometry is not time-driven but distance-driven)
				Perturbation perturbation; ///< Perturbation Gaussian vector
				/**
				 * Constant perturbation flag.
				 * Flag for indicating that the state perturbation Q is constant and should not be computed at each iteration.
				 *
				 * In case this flag wants to be set to \c true , the user must consider computing the constant \a Q immediately after construction time.
				 * This can be done in three ways:
				 * - define a function member \b setup(..args..) in the derived class to compute the Jacobian XNEW_pert,
				 *   and enter the appropriate perturbation.P() value.
				 * 	 Then call \b computeStatePerturbation(), which will compute \a Q from \a P() and \a XNEW_pert.
				 * - define a function member \b setup(..args..) in the derived class to enter the matrix Q directly.
				 *
				 * In any of the above cases, call your \b setup() function immediately after the constructor.
				 * - Define a constructor that accepts a number of parameters relevant to your perturbation levels,
				 *   and perform all the above operations to obtain Q inside the constructor body.
				 */
				bool constantPerturbation;

				jblas::mat XNEW_x; ///<         Jacobian wrt state
				jblas::mat XNEW_pert; ///<      Jacobian wrt perturbation
				jblas::sym_mat Q; ///<          Process noise covariances matrix in state space, Q = XNEW_pert * perturbation.P * trans(XNEW_pert);

				static size_t size_control() {
					return 0;
				}
				static size_t size_perturbation() {
					return 0;
				}
				void set_control(const vec & c) {
					JFR_ASSERT(c.size() == size_control(), "RobotAbstract::set_control(vec&, double): Sizes mismatch");
					control = c;
				}
				void set_perturbation(const Perturbation & _pert) {
					perturbation = _pert;
				}


				/**
				 * Move one step ahead, affect SLAM filter.
				 * This function updates the full state and covariances matrix of the robot plus the cross-variances with all other map objects.
				 */
				virtual void move();

				/**
				 * Move one step ahead, affect SLAM filter.
				 * This function updates the full state and covariances matrix of the robot plus the cross-variances with all other map objects.
				 */
				template<class V>
				inline void move(V & _u) {
					JFR_ASSERT(_u.size() == control.size(), "robotAbstract.hpp: move: wrong control size.");
					control = _u;
					move();
				}


				/**
				 * Compute robot process noise \a Q in state space.
				 * This function is called by move() at each iteration if constantPerturbation is \b false.
				 * It performs the operation:
				 * - Q = XNEW_pert * control.P() * XNEW_pert',
				 *
				 * where XNEW_pert and control.P() must have been already computed.
				 */
				void computeStatePerturbation();

				/**
				 * Explore all sensors.
				 * This function iterates all the sensors in the robot and calls the main sensor operations.
				 */
				void exploreSensors();

			protected:


				/**
				 * Move one step ahead.
				 *
				 * Implement this function in every derived class.
				 *
				 * This function predicts the robot state one step of length \a _dt ahead in time,
				 * according to the current state _x, the control input \a _u and the time interval \a _dt.
				 * It computes the new state and the convenient Jacobian matrices.
				 *
				 * In clase the flag \b constantPerturbation is set to \c true, the matrix _XNEW_pert is not computed.
				 *
				 * \sa Documentation of the constantPerturbation flag.
				 *
				 * \param _x the current state vector
				 * \param _u the control vector
				 * \param _dt the time interval
				 * \param _xnew the new state
				 * \param _XNEW_x the Jacobian of \a _xnew wrt \a _x
				 * \param _XNEW_pert the Jacobian of \a _xnew wrt \a _n
				 */
				virtual void move_func(const vec & _x, const vec & _u, const vec& _n, const double _dt, vec & _xnew,
				                       mat & _XNEW_x, mat & _XNEW_pert) = 0;


				//				/**
				//				 * Move one step ahead, use object members as data.
				//				 */
				//				inline void move_func() {
				//					vec x = state.x();
				//					vec n = perturbation.x();
				//					cout << "x = " << x << endl;
				//					move_func(x, control, n, dt_or_dx, x, XNEW_x, XNEW_pert);
				//					state.x() = x;
				//				}
				/**
				 * Move one step ahead, use object members as data.
				 */
				//				virtual void move_func() = 0;

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
