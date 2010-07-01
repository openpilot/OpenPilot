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
#include "kernel/IdFactory.hpp"

#include "rtslam/rtSlam.hpp"
#include "rtslam/gaussian.hpp"
#include "rtslam/mapObject.hpp"
#include "rtslam/perturbation.hpp"
// include parents
#include "rtslam/mapAbstract.hpp"
#include "rtslam/mapObject.hpp"
#include "rtslam/parents.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;


		//  Forward declarations of children
		class SensorAbstract;

		/** Base class for all robots defined in the module
		 * rtslam.
		 *
		 * \ingroup rtslam
		 */
		class RobotAbstract: public MapObject, public ChildOf<MapAbstract> , public boost::enable_shared_from_this<
		    RobotAbstract>, public ParentOf<SensorAbstract> {

				friend ostream& operator <<(ostream & s, jafar::rtslam::RobotAbstract & rob);

			public:

				enum type_enum {
					ODOMETRY, CONSTANT_VELOCITY, INERTIAL, CENTERED_CONSTANT_VELOCITY
				};
				type_enum type;


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
				/**
				 * Remote constructor from remote map and size of state and control vectors.
				 * \param dummy flag indicating simulation object. Give MapObject::FOR_SIMULATION.
				 * \param _mapPtr a pointer to the map.
				 * \param _size_state the size of the robot state vector
				 * \param _size_control the size of the control vector
				 * \param _size_pert the size of the perturbation vector
				 */
				RobotAbstract(const simulation_t dummy, const map_ptr_t & _mapPtr, const size_t _size_state, const size_t _size_control,
				              const size_t _size_pert);

				// Mandatory virtual destructor.
				virtual ~RobotAbstract() {
				}

				virtual std::string categoryName() {
					return "ROBOT";
				}


				static IdFactory robotIds;

				void setId(){id(robotIds.getId());}

				Gaussian pose; ///<             Robot Gaussian pose
				vec control; ///<               Control vector
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
				double self_time; ///< 					Current estimation time
				double dt_or_dx; ///<           Sampling time or any other relevant increment (e.g. odometry is not time-driven but distance-driven)

				jblas::mat XNEW_x; ///<         Jacobian wrt state
				jblas::mat XNEW_pert; ///<      Jacobian wrt perturbation
				jblas::sym_mat Q; ///<          Process noise covariances matrix in state space, Q = XNEW_pert * perturbation.P * trans(XNEW_pert);

				virtual size_t mySize() = 0;
				virtual size_t mySize_control() = 0;
				virtual size_t mySize_perturbation() = 0;
				static size_t size_control() {
					return 0;
				}
				static size_t size_perturbation() {
					return 0;
				}
				void set_control(const vec & c) {
					JFR_ASSERT(c.size() == mySize_control(), "RobotAbstract::set_control(vec&, double): Sizes mismatch");
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
				//template<class V>
				inline void move(const vec & _u) {
					JFR_ASSERT(_u.size() == control.size(), "robotAbstract.hpp: move: wrong control size.");
					control = _u;
					move();
				}

				void move(double time){
					if (self_time < 1.) dt_or_dx = 0;
					else dt_or_dx = time - self_time;
					perturbation.set_from_continuous(dt_or_dx);
					move();
					self_time = time;
				}

				void move(const vec & u, double time){
					if (self_time < 1.) dt_or_dx = 0;
					else dt_or_dx = time - self_time;
					perturbation.set_from_continuous(dt_or_dx);
					move(u);
					self_time = time;
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
