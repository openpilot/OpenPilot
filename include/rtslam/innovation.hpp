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
		 * It implements the trivial innovation model inn = meas - exp.
		 * It also returns the Jacobian matrices.
		 * Derive this class if you need other non-trivial innovation
		 * models (useful for line landmarks).
		 *
		 *\ingroup rtslam
		 */
		class Innovation: public Gaussian {
			public:
				/// The inverse of the innovation covariances matrix.
				jblas::sym_mat iP_;
				/// The Mahalanobis distance from the measurement to the expectation.
				double mahalanobis_;
				/// The Jacobian of the innovation wrt the measurement.
				jblas::mat INN_meas;
				/// The Jacobian of the innovation wrt the expectation.
				jblas::mat INN_exp;
				/// The Jacobian of the innovation wrt the state vector.
				jblas::mat INN_x;
				/// the indirect array of states that contributed to the innovation
				jblas::ind_array ia;

			public:


				/**
				 * Size construction.
				 * \param _size the innovation size
				 */
				Innovation(const size_t _size);

				/**
				 * Sizes construction.
				 * \param _size the innovation size
				 * \param _size_meas the measurement size
				 * \param _size_exp the expectation size
				 */
				Innovation(const size_t _size, const size_t _size_meas, const size_t _size_exp);

				/**
				 * Sizes and indirect_array constructor.
				 * The indirect array points to the states in the map that contributed to the expectation.
				 */
				Innovation(const size_t _size, const size_t _size_meas, const ind_array & _ia_x);



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
				 * \param exp_mean the expectation
				 * \param meas_mean the measurement
				 */
				void compute(Expectation& exp, Measurement& meas) {
					compute(exp.x(), meas.x()); // We do not request trivial Jacobians here. Jacobians are the identity.
					P() = meas.P() + exp.P(); // Derived classes: P = Inn_meas*meas.P*Inn_meas.transpose() + Inn_exp*exp.P*Inn_exp.transpose();
					INN_x = prod(INN_exp,exp.EXP_x);
				}
		};



	}
}

#endif /* INNOVATION_HPP_ */
