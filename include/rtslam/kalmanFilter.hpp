/**
 *  \file kalmanFilter.hpp
 *
 * Header file for the kalman filter
 *
 *  Created on: 04/03/2010
 *     \author jsola
 *
 * \ingroup rtslam
 */

#ifndef KALMANFILTER_HPP_
#define KALMANFILTER_HPP_

#include "jmath/ixaxpy.hpp"
#include "jmath/ublasExtra.hpp"
#include "rtslam/innovation.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;
		using namespace jblas;


		/**
		 * Base class for Kalman filters
		 * \ingroup rtslam
		 */
		class ExtendedKalmanFilterIndirect {
			public:
				size_t size; // state size
				size_t measurementSize;
				size_t expectationSize;
				size_t innovationSize;
			private:
				vec x_;
				sym_mat P_;
			public:
				//				boost::posix_time::time_duration curTime;
				mat K;
				mat PHt_tmp;

				ExtendedKalmanFilterIndirect(size_t _size);

				jblas::vec & x() {
					return x_;
				}
				jblas::sym_mat & P() {
					return P_;
				}
				double & x(size_t i) {
					return x_(i);
				}
				double & P(size_t i, size_t j) {
					return P_(i, j);
				}


				// TODO: define API for all these functions.
				/**
				 * Predict covariances matrix.
				 *
				 * This function predicts the future state of the covariances matrix.
				 * It uses a Jacobian \a F_v indexed by an indirect array \a iav into the state vector.
				 * The covariances matrix is also indexed by an indirect array \a iax containing the used states of the filter.
				 * It incorporates the process noise \a U via a second Jacobian \a F_u, mapping it to the state space via \a iav (the same as F_v).
				 * The formula is:
				 * - [Pvv, Pvm; Pmv, Pmm] = [F_v*Pvv*F_v'+F_u*U*F_u' ,  F_v*Pvm ; Pmv*F_v' ,  Pmm]
				 *
				 * \param iax the ind_array of all used states in the map.
				 * \param F_v the Jacobian of the process model.
				 * \param iav the ind_array of the process model states.
				 * \param F_u the Jacobian of the process model wrt the perturbation.
				 * \param U the covariances matrix of the perturbation in control-space.
				 */
				void predict(const ind_array & iax, const mat & F_v, const ind_array & iav, const mat & F_u, const sym_mat & U);

				/**
				 * Predict covariances matrix.
				 *
				 * This function predicts the future state of the covariances matrix.
				 * It uses a Jacobian \a F_v indexed by an indirect array \a iav into the state vector.
				 * The covariances matrix is also indexed by an indirect array \a iax containing the used states of the filter.
				 * It incorporates the process noise \a Q, already mapped to the state space.
				 * The formula is:
				 * - [Pvv, Pvm; Pmv, Pmm] = [F_v*Pvv*F_v'+Q ,  F_v*Pvm ; Pmv*F_v' ,  Pmm]
				 *
				 * \param iax the ind_array of all used states in the map.
				 * \param F_v the Jacobian of the process model.
				 * \param iav the ind_array of the process model states.
				 * \param Q the covariances matrix of the perturbation in state-space.
				 */
				void predict(const ind_array & iax, const mat & F_v, const ind_array & iav, const sym_mat & Q);

				/**
				 * EKF correction.
				 * This function uses the Innovation class to extract all useful chunks necessary for EKF correction.
				 * In partucular, the following info is recovered from Innovation:
				 * - INN_rsl: the Jacobian wrt the states that contributed to the innovation
				 * - ia: the indices to these states
				 * - {z, Z} = {inn.x, inn.P}, mean and conv. matrices.
				 *
				 * the EKF update is then the following:
				 * - K = -P * trans(INN_rsl) * inv(Z)
				 * - x = x + K * z
				 * - P = P - K * INN_rsl * P
				 *
				 * \param iax the indirect array of used indices in the map.
				 * \param inn the Innovation.
				 */
				void correct(const ind_array & iax, Innovation & inn, const mat & INN_rsl, const ind_array & ia_rsl);

				void computeInnovation();
				void computeK();
				void updateP();
				void stackCorrection();
				void correctAllStacked();

		};

	}
}

#endif /* KALMANFILTER_HPP_ */
