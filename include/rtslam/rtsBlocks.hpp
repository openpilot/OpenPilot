/*
 * rtsBlocks.h
 *
 *     Project: rtslam
 *  Created on: Jan 29, 2010
 *      Author: jsola
 */

#ifndef RTSBLOCKS_H_
#define RTSBLOCKS_H_

#include "jmath/jblas.hpp"
#include "jmath/ublasExtra.hpp"

// Shortcut for namespace ublas
namespace ublas = boost::numeric::ublas;

using namespace jblas;

namespace jafar {

	namespace rtslam {

		/** Base class for all Gaussians defined in the module rtslam.
		 *
		 * @ingroup rtslam
		 */
		class Gaussian {
			protected:
				vec x;
				vec std;
				//				sym_mat P;
				mat P;
				void update_std() {
					std.resize(P.size1());
					for (std::size_t i = 0; i < std.size(); i++) {
						std(i) = sqrt(P(i, i));
					}
				}
				void update_P() {
					P.resize(std.size());
					for (std::size_t i = 0; i < std.size(); i++) {
						P(i, i) = std(i) * std(i);
					}
				}

			public:

				// Constructor
				Gaussian(const vec x_, const sym_mat P_) {
					set(x_, P_);
				}

				// Constructor
				Gaussian(const vec x_, const vec std_) {
					set(x_, std_);
				}

				// Accessors
				void set_x(const vec x_) {
					x(x_);
				}
				void set_std(const vec std_) {
					std(std_);
					update_P();
				}
				void set_P(const sym_mat P_) {
					P(P_);
					update_std();
				}
				void set(const vec x_, const sym_mat P_) {
					set_x(x_);
					set_P(P_);
				}
				void set(const vec x_, const vec std_) {
					set_x(x_);
					set_std(std_);
				}
				const vec x() {
					return x;
				}
				const vec std() {
					return std;
				}
				const sym_mat P() {
					return P;
				}

				/**
				 * The symmetric product at the exponent of the Gaussian density.
				 * This is the value E so that the density is d = norm_factor * exp(-0.5*E).
				 * \param x_  the evaluation point.
				 * \return the exponent part E = (trans(x_-x)*inv(P)*(x_-x)
				 */
				double doubled_abs_exponent(const vec x_) {
					sym_mat Pinv(P.size1());
					lu_inv(P, Pinv);
					vec tmp = ublas::prod(Pinv, (x_ - x));
					return ublas::inner_prod((x_ - x), tmp);
				}

				/**
				 * The Gaussian density at a given point.
				 * @param x_ the evaluation point.
				 * \return the density d = 1/sqrt(2 pi^n det(P))*exp(-0.5*(trans(x_-x)*inv(P)*(x_-x)))
				 */
				double density(const vec x_);

				/**
				 * Mahalanobis distance to a second independent Gaussian.
				 * \param g the second Gaussian.
				 * \return the mahalanobis distance.
				 */
				double mahalanobis(const Gaussian g);

		};

		/** Base class for all indirect Gaussians defined in the module rtslam.
		 *
		 * @ingroup rtslam
		 */
		class GaussianIndirect : public Gaussian{
				ind_array r;
				vec_indirect x;
				sym_mat_indirect P;
			public:
				// Constructor
				GaussianIndirect(const vec& x_, const sym_mat& P_) {
					set(x_, P_);
				}
				// Accessors
				void set_r(const indirect_array& r_) {
					r(r_);
				}
//				void set_r(const ublas::vector<int> r_) {
//					r(r_);
//				}
				void set_x(const vec x_) {
					x(x_);
				}
				void set_P(const sym_mat P_) {
					P(P_);
				}
				void set(const vec x_, sym_mat P_) {
					set_x(x_);
					set_P(P_);
				}
				const vec get_x() {
					return x;
				}
				const sym_mat get_P() {
					return P_;
				}
		};

		/** Base class for all Gaussian state vectors defined in the module rtslam.
		 *
		 * @ingroup rtslam
		 */
		class State: public GaussianIndirect {
//			protected:
//				indirect_array r;
//				vec_indirect x;
//				sym_mat_indirect P;
//			public:
//				void set_r(const indirect_array r_);
//				void set_x(const vec x_);
//				void set_P(const sym_mat P_);
//				const indirect_array get_r();
//				const vec get_x();
//				const sym_mat get_P();
		};

		/** Base class for all Gaussian poses defined in the module rtslam.
		 *
		 * @ingroup rtslam
		 */
		class Pose: public State {
		};

		/** Base class for all Gaussian control vectors defined in the module rtslam.
		 *
		 * @ingroup rtslam
		 */
		class Control: public Gaussian {
				double dt;
			public:
				void set_dt(double dt_) {
					dt(dt_);
				}
				void set(const vec x_, const sym_mat P_, double td_) {
					set(x_, P_);
					set_dt( dt_);
				}
				void set(const vec x_, const vec std_, double dt_) {
					set_x(x_);
					set_std( xStd_);
					set_dt(dt_);
				}
				const double get_dt() {
					return dt;
				}
		};

		/** Base class for all landmark appearances defined in the module
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class rtsAppearanceAbstract {
				/**
				 * Mandatory virtual destructor
				 */
				virtual ~rtsAppearanceAbstract(void);
		};

		/** Base class for all Gaussian expectations defined in the module rtslam.
		 *
		 * @ingroup rtslam
		 */
		class Expectation: public Gaussian {
			public:
				rtsAppearanceAbstract appearance;
		};

		/** Base class for all Gaussian measurements defined in the module rtslam.
		 *
		 * @ingroup rtslam
		 */
		class Measurement: public Gaussian {
			public:
				rtsAppearanceAbstract appearance;
		};

		/** Base class for all Gaussian innovations defined in the module rtslam.
		 *
		 * It implements the trivial innovation model inn = meas - exp. It also returns the Jacobian matrices.
		 *
		 * Derive this class for other non-trivial innovation models.
		 *
		 * @ingroup rtslam
		 */
		class Innovation: public Gaussian {
			protected:
				///  the expectation size.
				std::size_t size_exp;
				/// the measurement size.
				std::size_t size_meas;
				/// the innovation size.
				std::size_t size_inn;
				/// The inverse fo the innovation covariances matrix.
				sym_mat iP;
				/// The Mahalanobis distance from the measurement to the expectation.
				double mahalanobis;
				/// The Jacobian of the innovation wrt the measurement.
				mat Inn_meas;
				/// The Jacobian of the innovation wrt the expectation.
				mat Inn_exp;
			public:

				/**
				 * the inverse of the innovation covariance.
				 */
				void invCov(void) {
					ublas::lu_inv(P, iP); // TODO: check documentation for inverting (small) matrices.
				}

				/**
				 * The Mahalanobis distance.
				 */
				void Mahalanobis(void) {
					mahalanobis = prod(x, prod(iP, x)); // TODO  see documentation for transpose and vec*mat*vec^T products.
				}

				/**
				 * The trivial innovation function  inn = meas - exp .
				 */
				void func(vec exp_mean /// The expected mean value.
				    , vec meas_mean /// The measured mean value.
				) {
					x = meas_mean - exp_mean;
				}

				/**
				 * The trivial innovation function inn = meas - exp. It updates the Jacobian matrices.
				 */
				void func_with_Jacobians(Expectation exp_mean /// The expected mean value.
				    , Measurement meas_mean /// The measured mean value.
				) {
					func(exp_mean, meas_mean);
					Inn_meas = identity_mat(size_exp);
					Inn_exp = -1.0 * identity_mat(size_exp);
				}

				/**
				 * Compute full innovation
				 */
				void compute(Expectation exp /// The expected Gaussian.
				    , Measurement meas /// The measured Gaussian.
				) {
					func(exp, meas); // We do not request trivial Jacobians here. Jacobians are the identity.
					P = meas.P + exp.P; // Derived classes: P = Inn_meas*meas.P*Inn_meas.transpose() + Inn_exp*exp.P*Inn_exp.transpose();
				}
		};

		/** Base class for all landmark descriptors defined in the module
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class rtsDescriptorAbstract {
				/**
				 * Mandatory virtual destructor
				 */
				virtual ~rtsDescriptorAbstract(void);
		};

	}
}

#endif /* RTSBLOCKS_H_ */
