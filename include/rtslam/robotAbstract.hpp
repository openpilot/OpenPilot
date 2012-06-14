/**
 * \file robotAbstract.hpp
 * \author jsola
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

#include "kernel/jafarDebug.hpp"
#include "kernel/IdFactory.hpp"
#include "kernel/dataLog.hpp"
#include "kernel/timingTools.hpp"
#include "jmath/jblas.hpp"

#include "rtslam/rtSlam.hpp"
#include "rtslam/gaussian.hpp"
#include "rtslam/mapObject.hpp"
#include "rtslam/perturbation.hpp"
// include parents
#include "rtslam/mapAbstract.hpp"
#include "rtslam/mapObject.hpp"
#include "rtslam/parents.hpp"

#include "rtslam/hardwareEstimatorAbstract.hpp"

namespace jafar {
	namespace rtslam {


		//  Forward declarations of children
		class SensorAbstract;

		/** Base class for all robots defined in the module
		 * rtslam.
		 *
		 * \ingroup rtslam
		 */
		class RobotAbstract: public MapObject, public ChildOf<MapAbstract> , public boost::enable_shared_from_this<
		    RobotAbstract>, public ParentOf<SensorAbstract>, public kernel::DataLoggable {

				friend ostream& operator <<(ostream & s, RobotAbstract const & rob);

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

				hardware::hardware_estimator_ptr_t hardwareEstimatorPtr;

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

				void setPoseDegStd(double x, double y, double z, double rollDeg,
				    double pitchDeg, double yawDeg, double xStd, double yStd, double zStd,
				    double rollDegStd, double pitchDegStd, double yawDegStd);
		
				void setPoseStd(double x, double y, double z, double roll,
				    double pitch, double yaw, double xStd, double yStd, double zStd,
				    double rollStd, double pitchStd, double yawStd);

				
				virtual std::string categoryName() const {
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
				kernel::Timestamp self_time; ///< 					Current estimation time
				double dt_or_dx; ///<           Sampling time or any other relevant increment (e.g. odometry is not time-driven but distance-driven)

				jblas::mat XNEW_x; ///<         Jacobian wrt state
				jblas::mat XNEW_pert; ///<      Jacobian wrt perturbation
				jblas::sym_mat Q; ///<          Process noise covariances matrix in state space, Q = XNEW_pert * perturbation.P * trans(XNEW_pert);
				
				jblas::vec origin_sensors; ///< origin to get the initial state position at 0 / absolute sensors
				jblas::vec origin_export; ///< origin of the exported position in absolute coordinates
				void setOrigin(jblas::vec3 pos) { origin_export = pos; }
				jblas::vec robot_pose; ///< the pose of the true robot in the slam robot frame, for exported position

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

				void setHardwareEstimator(hardware::hardware_estimator_ptr_t hardwareEstimatorPtr_)
				{
					hardwareEstimatorPtr = hardwareEstimatorPtr_;
				}


				/**
				 * Move one step ahead, affect SLAM filter.
				 * This function updates the full state and covariances matrix of the robot plus the cross-variances with all other map objects.
				 */
				void move() {
					vec x = state.x();
					vec n = perturbation.x();
					vec xnew(x.size());

					move_func(x, control, n, dt_or_dx, xnew, XNEW_x, XNEW_pert);
					state.x() = xnew;

					if (mapPtr()->filterPtr){

						if (!constantPerturbation)
							computeStatePerturbation();

						mapPtr()->filterPtr->predict(mapPtr()->ia_used_states(), XNEW_x, state.ia(), Q); // P = F*P*F' + Q
					}
				}


				/**
				 * Initialize the state, affect SLAM filter.
				 */
				inline void init(const vec & _u, const vec & _U) {
					JFR_ASSERT(_u.size() >= control.size(), "robotAbstract.hpp: init: wrong control size.");
					control = ublas::subrange(_u, 0, control.size());
					vec x = state.x();
					vec xnew(x.size());

					init_func(x, control, _U, xnew);
					state.x() = xnew;
				}
				
				inline void init(const vec & _u) {
					JFR_ASSERT(_u.size() >= control.size(), "robotAbstract.hpp: init: wrong control size.");
					control = ublas::subrange(_u, 0, control.size());
					vec x = state.x();
					vec xnew(x.size());

					init_func(x, control, xnew);
					state.x() = xnew;
				}
				/**
				 * Move one step ahead, affect SLAM filter.
				 * This function updates the full state and covariances matrix of the robot plus the cross-variances with all other map objects.
				 */
				//template<class V>
				inline void move(const vec & _u) {
					JFR_ASSERT(_u.size() >= control.size(), "robotAbstract.hpp: move: wrong control size.");
					control = ublas::subrange(_u, 0, control.size());
					move();
				}
				
				virtual void move(double time);
				void move_fake(double time);

				/**
				 * Compute robot process noise \a Q in state space.
				 * This function is called by move() at each iteration if constantPerturbation is \b false.
				 * It performs the operation:
				 * - Q = XNEW_pert * perturbation.P() * XNEW_pert',
				 *
				 * where XNEW_pert and perturbation.P() must have been already computed.
				 */
				void computeStatePerturbation();


				virtual void writeLogHeader(kernel::DataLogger& log) const;
				virtual void writeLogData(kernel::DataLogger& log) const;

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
				 * \param _n the perturbation vector
				 * \param _dt the time interval
				 * \param _xnew the new state
				 * \param _XNEW_x the Jacobian of \a _xnew wrt \a _x
				 * \param _XNEW_pert the Jacobian of \a _xnew wrt \a _n
				 */
				virtual void move_func(const vec & _x, const vec & _u, const vec& _n, const double _dt, vec & _xnew,
				                       mat & _XNEW_x, mat & _XNEW_pert) = 0;
				/**
				 * Initialize the robot state with the average previous values of control
				 *
				 * You may optionnaly implement this function in derived classes.
				 *
				 * This function can initialize differently the robot state, given the
				 * average control input on a past period of time, if possible.
				*/
				virtual void init_func(const vec & _x, const vec & _u, const vec & _U, vec & _xnew) {}
				virtual void init_func(const vec & _x, const vec & _u, vec & _xnew) {}


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
