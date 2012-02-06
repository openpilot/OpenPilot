/**
 * \file robotConstantVelocity.hpp
 *
 * Header file for the robot with constant velocity motion model.
 *
 * \date 14/02/2010
 * \author jsola
 *
 * \ingroup rtslam
 */

#ifndef ROBOTCONSTANTVELOCITY_HPP_
#define ROBOTCONSTANTVELOCITY_HPP_

#include "rtslam/robotAbstract.hpp"

namespace jafar {
	namespace rtslam {

		class RobotConstantVelocity;
		typedef boost::shared_ptr<RobotConstantVelocity> robconstvel_ptr_t;


		/**
		 * Constant velocity model robot class.
		 *
		 * \author jsola
		 *
		 * This class implements a rigid frame in 3D moving with a constant velocity motion model. This model is the following:\n
		 * - p += v*dt <-- position\n
		 * - q = q**(w*dt) <-- orientation quaternion\n
		 * - v += vi <-- linear velocity\n
		 * - w += wi <-- angular velocity
		 *
		 * where \a vi and \a wi are linear and angular velocity random impulses,
		 * and ** is the quaternion product.
		 *
		 * This model is embedded into the system variables as follows:
		 * - the prediction function is x+ = f(x,u), implemented with method move().
		 * - the state vector is x = state.x = [p q v w]
		 * - the state covariance is in the map, P = state.P = (map.filter).P(state.ia, state.ia)
		 * - the control vector is u = control.x = [vi wi]
		 * - the control covariances matrix is U = control.P
		 * - the Jacobians of f(x,u) provided by move() are XNEW_x and XNEW_control.
		 * - the perturbation covariance is obtained with the method computeStatePerturbation(), as follows:
		 * 		- Q = XNEW_control * control.P * XNEW_control'\n
		 * .
		 *
		 * \sa Explore the comments in file robotConstantVelocity.cpp for full algebraic details.
		 *
		 * \ingroup rtslam
		 */
		class RobotConstantVelocity: public RobotAbstract {
			public:


				/**
				 * Remote constructor from remote map.
				 * \param _map the remote map
				 */
				RobotConstantVelocity(const map_ptr_t & _mapPtr);
				/**
				 * Remote constructor from map, for simulation.
				 * \param dummy flag for simulation. Give value FOR_SIMULATION.
				 * \param _map the map
				 */
				RobotConstantVelocity(const simulation_t dummy, const map_ptr_t & _mapPtr);

				~RobotConstantVelocity(void) {
				}

				virtual std::string typeName() const {
					return "Constant-velocity";
				}


				/**
				 * Move one step ahead.
				 *
				 * This function predicts the robot state one step of length \a dt ahead in time,
				 * according to the control input \a control.x and the time interval \a control.dt.
				 *
				 * \param _x the current state vecto
				 * \param _p the perturbation vector
				 * \param _dt the sampling time
				 * \param _xnew the new state vector
				 * \param _XNEW_x the Jacobian of xnew wrt x
				 * \param _XNEW_pert the Jacobian of xnew wrt p
				 */
				void move_func(const vec & _x, const vec & _u, const vec & _n, const double _dt, vec & _xnew, mat & _XNEW_x,
				    mat & _XNEW_pert);

				void computePertJacobian();

				static size_t size() {
					return 13;
				}
				static size_t size_control() {
					return 0;
				}
				static size_t size_perturbation() {
					return 6;
				}

				virtual size_t mySize() {return size();}
				virtual size_t mySize_control() {return size_control();}
				virtual size_t mySize_perturbation() {return size_perturbation();}

				void setVelocityStd(double velLinStd, double velAngStd){
					for (size_t i = pose.size(); i < pose.size() + 3; i++){
						state.P(i,i) = velLinStd*velLinStd;
					}
					for (size_t i = pose.size() + 3; i < pose.size() + 6; i++){
						state.P(i,i) = velAngStd*velAngStd;
					}
				}

				virtual void writeLogHeader(kernel::DataLogger& log) const;
				virtual void writeLogData(kernel::DataLogger& log) const;
				
			protected:
				/**
				 * Split state vector.
				 *
				 * Extracts \a p, \a q, \a v and \a w from the state vector, \a x = [\a p, \a q, \a v, \a w].
				 * \param x the state vector
				 * \param p the position
				 * \param q the quaternion
				 * \param v the linear velocity
				 * \param w the angular velocity
				 */
				template<class Vx, class Vp, class Vq, class Vv, class Vw>
				inline void splitState(const Vx x, Vp & p, Vq & q, Vv & v, Vw & w) {
					p = ublas::subrange(x, 0, 3);
					q = ublas::subrange(x, 3, 7);
					v = ublas::subrange(x, 7, 10);
					w = ublas::subrange(x, 10, 13);
				}


				/**
				 * Compose state vector.
				 *
				 * Composes the state vector with \a p, \a q, \a v and \a w, \a x = [\a p, \a q, \a v, \a w].
				 * \param p the position
				 * \param q the quaternion
				 * \param v the linear velocity
				 * \param w the angular velocity
				 * \param x the state vector
				 */
				template<class Vp, class Vq, class Vv, class Vw, class Vx>
				inline void unsplitState(const Vp & p, const Vq & q, const Vv & v, const Vw & w, Vx & x) {
					ublas::subrange(x, 0, 3) = p;
					ublas::subrange(x, 3, 7) = q;
					ublas::subrange(x, 7, 10) = v;
					ublas::subrange(x, 10, 13) = w;
				}


				/**
				 * Split control vector.
				 *
				 * Extracts impulses \a vi and \a wi from the control vector.
				 * \param vi the linear impulse.
				 * \param wi the angular impulse.
				 */
				template<class Vu, class V>
				inline void splitControl(const Vu & u, V & vi, V & wi) {
					vi = project(u, ublas::range(0, 3));
					wi = project(u, ublas::range(3, 6));
				}

			private:
				// temporary matrices to speed up Jacobian computation
				mat33 PNEW_v; ///<      Temporary Jacobian matrix
				mat43 QWDT_wdt; ///< 	Temporary Jacobian matrix
				mat44 QNEW_qwdt; ///<   Temporary Jacobian matrix
				mat43 QNEW_wdt; ///<    Temporary Jacobian matrix
				mat44 QNEW_q; ///<      Temporary Jacobian matrix
				mat44 QNORM_qnew; ///<	Temporary Jacobian matrix

		};
	}
}

#endif /* ROBOTCONSTANTVELOCITY_HPP_ */
