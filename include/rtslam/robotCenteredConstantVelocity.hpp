/**
 * \file robotCenteredConstantVelocity.hpp
 *
 * Header file for the robot with robot centered constant velocity motion model.
 *
 * \date 01/07/2010
 * \author jmcodol
 *
 * \ingroup rtslam
 */

#ifndef ROBOTCENTEREDCONSTANTVELOCITY_HPP_
#define ROBOTCENTEREDCONSTANTVELOCITY_HPP_

#include "rtslam/robotAbstract.hpp"

namespace jafar {
	namespace rtslam {

		class RobotCenteredConstantVelocity;
		typedef boost::shared_ptr<RobotCenteredConstantVelocity> robcenteredconstvel_ptr_t;


		/**
		 * RobotCentric Constant velocity model robot class.
		 *
		 * \author jmcodol
		 *
		 * This class implements a rigid frame in 3D moving with a constant velocity motion model.\n
		 *
		 * From [Bounding Uncertaincy in EKF-SLAM. The Robocentric Local Approach], Ruben Martinez-Cantin, Jose A. Castellanos
		 *
		 * \ingroup rtslam
		 */
		class RobotCenteredConstantVelocity: public RobotAbstract {
			public:


				/**
				 * Remote constructor from remote map.
				 * \param _map the remote map
				 */
				RobotCenteredConstantVelocity(const map_ptr_t & _mapPtr);
				/**
				 * Remote constructor from map, for simulation.
				 * \param dummy flag for simulation. Give value FOR_SIMULATION.
				 * \param _map the map
				 */
				RobotCenteredConstantVelocity(const simulation_t dummy, const map_ptr_t & _mapPtr);

				~RobotCenteredConstantVelocity(void) {
				}

				virtual std::string typeName() const {
					return "Centered-Constant-velocity";
				}

				void getFrameForReframing(vec7 & frame) ;

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

				/**
				 * The size of the robot in map.
				 *
				 * Warning, there is no respect to the article, in the sense that :
				 * in the article we have :
				 * x = [  x_B^Rk-1  ] = [ R1 ] We are, in our case, using : R = [ R2 ] = [  x_Rk^Rk-1 ]
				 *     [  x_F^Rk-1  ]   [ L  ]                                  [ R1 ]   [  x_B^Rk-1  ]
				 *     [  x_Rk^Rk-1 ]   [ R2 ]                              L = [ L  ] = [  x_F^Rk-1  ]
				 * the robot contains :
				 * - R2 = x_Rk_Rkm1 : the pose/speed/... of the robot at time k, with respect to the frame of the robot at time k-1
				 * - R1 = x_B_Rkm1 : the pose of the Base Frame B, with respect to the frame of the robot at time k-1.
				 *
				 * X = [ x_Rk_Rkm1 ;
				 *       x_B_Rkm1  ] ;
				 *
				 * (see eq. 7) in article
				 */
				static size_t size() {
					int size_x_Rk_Rkm1 = 13 ; // P,Q,V,W
					int size_x_B_Rkm1  = 7  ; // P,Q
					return size_x_Rk_Rkm1 + size_x_B_Rkm1  ;
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

				/**
				 * we have :
				 * robot state = r = [ R2 ] = [  x_Rk^Rk-1 ] = [  pose ]
				 *                   [ R1 ]   [  x_B^Rk-1  ]   [ -base ]
				 *
				 */
				void setVelocityStd(double velLinStd, double velAngStd){
					for (size_t i = pose.size(); i < pose.size() + 3; i++){
						state.P(i,i) = velLinStd*velLinStd;
					}
					for (size_t i = pose.size() + 3; i < pose.size() + 6; i++){
						state.P(i,i) = velAngStd*velAngStd;
					}
				}

			protected:
//				/**
//				 * Split state vector.
//				 *
//				 * Extracts \a p, \a q, \a v and \a w from the state vector, \a x = [\a p, \a q, \a v, \a w].
//				 * \param x the state vector
//				 * \param p the position
//				 * \param q the quaternion
//				 * \param v the linear velocity
//				 * \param w the angular velocity
//				 */
//				template<class Vx, class Vp, class Vq, class Vv, class Vw>
//				inline void splitState(const Vx x, Vp & p, Vq & q, Vv & v, Vw & w) {
//					p = ublas::subrange(x, 0, 3);
//					q = ublas::subrange(x, 3, 7);
//					v = ublas::subrange(x, 7, 10);
//					w = ublas::subrange(x, 10, 13);
//				}

				/**
				 * Split state vector.
				 *
				 * Extracts \a p, \a q, \a v \a w \a pBase and \a qBase from the state vector, \a x = [\a p, \a q, \a v, \a w, \a pBase, \a qBase].
				 * \param x the state vector
				 * \param p the position
				 * \param q the quaternion
				 * \param v the linear velocity
				 * \param w the angular velocity
				 * \param pBase the position of the base frame in the current frame
				 * \param qBase the quaternion of the base frame in the current frame
				 */
				template<class Vx, class Vp, class Vq, class Vv, class Vw>
				inline void splitState(const Vx x, Vp & p, Vq & q, Vv & v, Vw & w, Vp & pBase, Vq & qBase) {
					p     = ublas::subrange(x,  0,  3);
					q     = ublas::subrange(x,  3,  7);
					v     = ublas::subrange(x,  7, 10);
					w     = ublas::subrange(x, 10, 13);
					pBase = ublas::subrange(x, 13, 16);
					qBase = ublas::subrange(x, 16, 20);
				}


//				/**
//				 * Compose state vector.
//				 *
//				 * Composes the state vector with \a p, \a q, \a v and \a w, \a x = [\a p, \a q, \a v, \a w].
//				 * \param p the position
//				 * \param q the quaternion
//				 * \param v the linear velocity
//				 * \param w the angular velocity
//				 * \param x the state vector
//				 */
//				template<class Vp, class Vq, class Vv, class Vw, class Vx>
//				inline void unsplitState(const Vp & p, const Vq & q, const Vv & v, const Vw & w, Vx & x) {
//					ublas::subrange(x, 0, 3) = p;
//					ublas::subrange(x, 3, 7) = q;
//					ublas::subrange(x, 7, 10) = v;
//					ublas::subrange(x, 10, 13) = w;
//				}

				/**
				 * Compose state vector.
				 *
				 * Composes the state vector with \a p, \a q, \a v \a w, \a pBase and \a qBase, \a x = [\a p, \a q, \a v, \a w, \a pBase, \a qBase].
				 * \param p the position
				 * \param q the quaternion
				 * \param v the linear velocity
				 * \param w the angular velocity
				 * \param x the state vector
			   * \param pBase the position of the base frame in the current frame
				 * \param qBase the quaternion of the base frame in the current frame
				 */
				template<class Vp, class Vq, class Vv, class Vw, class Vx>
				inline void unsplitState(const Vp & p, const Vq & q, const Vv & v, const Vw & w, const Vp & pBase, const Vq & qBase, Vx & x) {
					ublas::subrange(x, 0, 3)   = p    ;
					ublas::subrange(x, 3, 7)   = q    ;
					ublas::subrange(x, 7, 10)  = v    ;
					ublas::subrange(x, 10, 13) = w    ;
					ublas::subrange(x, 13, 16) = pBase;
					ublas::subrange(x, 16, 20) = qBase;
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
				vec7 lastJump; // F contains [p,q]' of the last reframe prediction.
				mat33 PNEW_v; ///<      Temporary Jacobian matrix
				mat43 QNEW_wdt; ///<   Temporary Jacobian matrix
//				mat43 PBASENEW_p; ///<    Temporary Jacobian matrix
//				mat44 PBASENEW_q; ///<    Temporary Jacobian matrix
//				mat43 QBASENEW_p; ///<    Temporary Jacobian matrix
//				mat44 QBASENEW_q; ///<    Temporary Jacobian matrix
//				mat44 QNEW_q; ///<      Temporary Jacobian matrix

		};
	}
}

#endif /* ROBOTCENTEREDCONSTANTVELOCITY_HPP_ */
