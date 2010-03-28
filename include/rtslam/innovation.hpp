/**
 * \file innovation.hpp
 *
 *  Created on: 25/03/2010
 *     \author: jsola@laas.fr
 *
 *
 *  This file defines the class Innovation.
 *
 * \ingroup rtslam
 */

#include "rtslam/expectation.hpp"
#include "rtslam/measurement.hpp"
#include "rtslam/gaussian.hpp"
#include "jmath/ublasExtra.hpp"

#ifndef INNOVATION_HPP_
#define INNOVATION_HPP_

namespace jafar {
	namespace rtslam {


		/** Base class for all Gaussian innovations defined in the module rtslam.
		 * \author jsola@laas.fr
		 *
		 * It implements the trivial innovation model:
		 * - inn = meas - exp.
		 *
		 * which is, after all,
		 * - z = y - h(x)
		 *
		 * so usual in Kalman filtering.
		 *
		 * It also returns the Jacobian matrices:
		 * - INN_exp = dz/dh = -I
		 * - INN_meas = dz/dy = I
		 *
		 * Derive this class and overload the methods if you need other non-trivial innovation
		 * models (useful for line landmarks).
		 *
		 *\ingroup rtslam
		 */
		class Innovation: public Gaussian {
			public:

				jblas::sym_mat iP_; ///<        The inverse of the innovation covariances matrix.
				double mahalanobis_; ///<       The Mahalanobis distance from the measurement to the expectation.
				jblas::mat INN_meas; ///<       The Jacobian of the innovation wrt the measurement.
				jblas::mat INN_exp; ///<        The Jacobian of the innovation wrt the expectation.
				jblas::mat INN_x; ///<          The Jacobian of the innovation wrt the state vector.
				jblas::ind_array ia_inn_x; ///< The indirect array of states that contributed to the innovation


				/**
				 * Size construction.
				 * Use this constructor for usual innovations with equal expectation, measurement and innovation sizes.
				 * \param _size the innovation size
				 */
				Innovation(const size_t _size);

				/**
				 * Sizes construction.
				 * Use this constructor for innovations with differing expectation, measurement and innovation sizes.
				 * \param _size the innovation size
				 * \param _size_meas the measurement size
				 * \param _size_exp the expectation size
				 */
				Innovation(const size_t _size, const size_t _size_meas, const size_t _size_exp);

				/**
				 * Sizes and indirect_array constructor.
				 * The indirect array points to the states in the map that contributed to the expectation.
				 * \param _size the innovation size
				 * \param _size_meas the measurement size
				 * \param _ia_x the indirect array pointing to the states that contributed to the innovation.
				 */
				Innovation(const size_t _size, const size_t _size_meas, const ind_array & _ia_x);

				virtual ~Innovation(){} ///< mandatory virtual destructor

				/**
				 * the inverse of the innovation covariance.
				 */
				void invertCov() {
					jafar::jmath::ublasExtra::lu_inv(P(), iP_);
				}


				/**
				 * The Mahalanobis distance.
				 */
				double mahalanobis() {
					invertCov();
					mahalanobis_ = ublas::inner_prod(x(), (jblas::vec) ublas::prod(iP_, x()));
					return mahalanobis_;
				}


				/**
				 * The trivial innovation function  inn = meas - exp.
				 * Derive the class and overload this method to use other, non-trivial innovation functions.
				 * \param exp_mean the expectation mean
				 * \param meas_mean the measurement mean
				 */
				template<class V1, class V2>
				void compute(V1& exp_mean, V2& meas_mean) {
					x() = meas_mean - exp_mean;
				}


				/**
				 * The trivial innovation function inn = meas - exp.
				 * It updates the Jacobian matrices.
				 * Derive the class and overload this method to use other, non-trivial innovation functions.
				 * \param exp_mean the expectation mean
				 * \param meas_mean the measurement mean
				 */
				template<class V1, class V2>
				void compute_with_Jacobians(V1& exp_mean, V2& meas_mean) {
					func(exp_mean, meas_mean);
					INN_meas = jblas::identity_mat(exp_mean.size());
					INN_exp = -1.0 * jblas::identity_mat(exp_mean.size());
				}


				/**
				 * Compute full innovation, with covariances matrix.
				 * Derive the class and overload this method to use other, non-trivial innovation functions.
				 * \param exp the expectation object
				 * \param meas the measurement object
				 */
				void compute(Expectation& exp, Measurement& meas);

		};

	}
}

#endif /* INNOVATION_HPP_ */
