/**
 * \file robotOdometry.hpp
 *
 * Header file for robot with odometry motion model.
 *
 * \date 01/01/2012
 * \author dmarquez
 * \author agonzale
 *
 * \ingroup rtslam
 */

#ifndef ROBOTODOMETRY_HPP_
#define ROBOTODOMETRY_HPP_

#include "rtslam/robotAbstract.hpp"

namespace jafar {
	namespace rtslam {

		class RobotOdometry;
		typedef boost::shared_ptr<RobotOdometry> robodo_ptr_t;

		/**
		 * Odometry motion model robot class.
		 *
		 * This class implements a rigid frame in 3D moving with an odometry motion model.
		 * This model performs one step on the pose F of a vehicle, given odometry increments U=[dx,dv] given by the robot frame.
		 *
		 *
		 * This model is the following:\n
		 * -p += dx <- position
		 * -q += v2q(dv) <- quaternion
		 * dx : position increment <- given by odometry sensors
		 * dv : orientation increment <- given by odometry sensors
		 *
		 *
		 * This model is embedded into the system variables as follows:
		 * - the prediction function is x+ = f(x,u), implemented with method move().
		 * - the state vector is x = state.x = [p q]
		 * - the state covariance is in the map, P = state.P = (map.filter).P(state.ia, state.ia)
		 * - the control vector is u = control.x = [dx dv]
		 * - the control covariances matrix is U = control.P
		 * - the Jacobians of f(x,u) provided by move() are XNEW_x and XNEW_control.
		 * - the perturbation covariance is obtained with the method computeStatePerturbation(), as follows:
		 * 		- Q = XNEW_control * control.P * XNEW_control'\n
		 *
		 * \sa Explore the comments in file robotOdometry.cpp for full algebraic details.
		 *
		 * \ingroup rtslam
		 */
		class RobotOdometry: public RobotAbstract {
			public:

				/**
				 * Remote constructor from remote map.
				 * \param _map the remote map
				 */
				RobotOdometry(const map_ptr_t _mapPtr);

				/**
				 * Remote constructor from remote map, for simulation.
				 * \param dummy flag for simulation. Give value FOR_SIMULATION.
				 * \param _map the remote map
				 */
				RobotOdometry(const simulation_t dummy, const map_ptr_t _mapPtr);

				~RobotOdometry(void) {
				}
				virtual std::string typeName() const {
					return "Odometry";
				}

				/**
				 * Move one step ahead.
				 *
				 * This function predicts the robot state one step of length \a dt ahead in time,
				 * according to the control input \a control.x and the time interval \a control.dt.
				 *
				 * \param _x the current state vector
				 * \param _u the odometry input vector
				 * \param _dt the sampling time
				 * \param _xnew the new state vector
				 * \param _XNEW_x the Jacobian of xnew wrt x
				 * \param _XNEW_pert the Jacobian of xnew wrt u
				 */
				void move_func(const vec & _x, const vec & _u, const vec & _n,
				    const double _dt, vec & _xnew, mat & _XNEW_x, mat & _XNEW_u);
				
				void init_func(const vec & _x, const vec & _u, vec & _xnew);

				static size_t size() {
					return 7;
				}

				static size_t size_control() {
					return 6;
				}

				static size_t size_perturbation() {
					return 6;
				}
				virtual size_t mySize() {
					return size();
				}
				virtual size_t mySize_control() {
					return size_control();
				}
				virtual size_t mySize_perturbation() {
					return size_perturbation();
				}
				

			protected:
				/**
				 * Split state vector.
				 *
				 * Extracts \a p and \a q from the state vector, \a x = [\a p, \a q].
				 * \param x the state vector
				 * \param p the position
				 * \param q the quaternion
				 */
				template<class Vx, class Vp, class Vq>
				inline void splitState(const Vx x, Vp & p, Vq & q) {
					p = ublas::subrange(x, 0, 3);
					q = ublas::subrange(x, 3, 7);
				}

				/**
				 * Compose state vector.
				 *
				 * Composes the state vector with \a p and \a q, \a x = [\a p, \a q].
				 * \param p the position
				 * \param q the quaternion
				 * \param x the state vector
				 */
				template<class Vp, class Vq, class Vx>
				inline void unsplitState(const Vp & p, const Vq & q, Vx & x) {
					ublas::subrange(x, 0, 3) = p;
					ublas::subrange(x, 3, 7) = q;
				}

				/**
				 * Split control vector.
				 *
				 * Extracts odometry datas \a dx and \a dv from the odometry vector.
				 * \param dx the position increment.
				 * \param dv the orientation increment.
				 */
				template<class Vu, class V>
				inline void splitControl(Vu & u, V & dx, V & dv) {
					dx = project(u, ublas::range(0, 3));
					dv = project(u, ublas::range(3, 6));
				}

			private:
				// temporary matrices to speed up Jacobian computation
				mat33 PNEW_dx;	///<	Temporary Jacobian matrix
				mat43 QDV_dv;	///<	Temporary Jacobian matrix
				mat44 QNEW_qdv; ///<	Temporary Jacobian matrix
				mat43 QNEW_dv;	///<	Temporary Jacobian matrix
				mat44 QNEW_q;	///<	Temporary Jacobian matrix
		};
	}
}

#endif /* ROBOTODOMETRY_HPP_ */
