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

				ExtendedKalmanFilterIndirect(size_t _size) :
					size(_size), x_(size), P_(size) {
					x_.clear();
					P_.clear();
				}

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
				 * \param iax the ind_array of used states.
				 * \param F_v the Jacobian of the process model.
				 * \param iav the ind_array of the process model states.
				 * \param F_u the Jacobian of the process model wrt the perturbation.
				 * \param U the covariances matrix of the perturbation.
				 */
				inline void predict(const ind_array & iax, const  mat & F_v, const ind_array & iav, const mat & F_u, const sym_mat & U) {
					jafar::jmath::ublasExtra::ixaxpy_prod(P_, iax, F_v, iav);
					ublas::project(P_, iav, iav) += jafar::jmath::ublasExtra::prod_JPJt(U, F_u);
				}
				inline void predict(const ind_array & iax, const mat & F_v, const ind_array & iav, const sym_mat & Q) {
					jafar::jmath::ublasExtra::ixaxpy_prod(P_, iax, F_v, iav);
					ublas::project(P_, iav, iav) += Q;
				}
				inline void correct();
				inline void computeInnovation();
				inline void computeK();
				inline void updateP();
				inline void stackCorrection();
				inline void correctAllStacked();

		};

	}
}

#endif /* KALMANFILTER_HPP_ */
