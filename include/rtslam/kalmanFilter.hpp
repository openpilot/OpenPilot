/**
 * kalmanFilter.hpp
 *
 *  Created on: 04/03/2010
 *      Author: jsola
 *
 *  \file kalmanFilter.hpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#ifndef KALMANFILTER_HPP_
#define KALMANFILTER_HPP_

//#include "rtslam/gaussian.hpp"
#include "jmath/ixaxpy.hpp"
#include "jmath/ublasExtra.hpp"

namespace jafar {
	namespace rtslam {

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
				jblas::vec x;
				jblas::sym_mat P;
				//				boost::posix_time::time_duration curTime;
				jblas::mat K;
				jblas::mat PHt_tmp;

				ExtendedKalmanFilterIndirect(size_t _size) :
					size(_size), x(size), P(size) {
					x.clear();
					P.clear();
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
				inline void predict(jblas::ind_array & iax, jblas::mat & F_v, jblas::ind_array & iav, jblas::mat & F_u,
				    jblas::sym_mat & U) {
					jafar::jmath::ublasExtra::ixaxpy_prod(P, iax, F_v, iav);
					ublas::project(P, iav, iav) += jafar::jmath::ublasExtra::prod_JPJt(U, F_u);
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
