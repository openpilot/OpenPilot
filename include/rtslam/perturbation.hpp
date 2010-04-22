/**
 * \file perturbation.hpp
 * 
 * ## Add brief description here ##
 *
 * \author jsola@laas.fr
 * \date 22/04/2010
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */



#ifndef PERTURBATION_HPP_
#define PERTURBATION_HPP_

#include "jmath/jblas.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;

		/** Base class for all Gaussian perturbation vectors defined in the module rtslam.
		 * \author jsola@laas.fr
		 *
		 * The Perturbation class is mainly a Gaussian. It represents discrete-time perturbation vectors.
		 *
		 * In case the perturbation and perturbation values want to be specified in continuous-time,
		 * this class incorporates private members for storing the continuous values,
		 * and also methods for the conversion to discrete-time.
		 *
		 * @ingroup rtslam
		 */
		class Perturbation: public Gaussian {
			private:
				vec x_ct; ///< continuous-time perturbation vector
				sym_mat P_ct; ///< continuous-time covariances matrix

			public:
				Perturbation(const size_t _size) :
					Gaussian(_size), x_ct(0), P_ct(0) {
				}
				Perturbation(const vec & p, const sym_mat & P) :
					Gaussian(p, P), x_ct(0), P_ct(0) {
				}
				Perturbation(const Gaussian & p) :
					Gaussian(p), x_ct(0), P_ct(0) {
				}
				template<class SymMat>
				void set_P_continuous(SymMat & _P_ct) {
					JFR_ASSERT(_P_ct.size1() == size(), "Matrix sizes mismatch.");
					P_ct.resize(size(), size());
					P_ct = _P_ct;
				}
				template<class V>
				void set_x_continuous(V & _x_ct) {
					JFR_ASSERT(_x_ct.size() == size(), "Vector sizes mismatch.");
					x_ct.resize(size());
					x_ct = _x_ct;
				}
				/**
				 * Discrete perturbation from continuous specification.
				 * - The white, Gaussian random values integrate with the square root of dt. Their variance integrates linearly with dt:
				 *		- P = _P_ct * _dt
				 *
				 * This function takes covariances from the internal variables of the class (which is often constant).
				 * \param _dt the time interval to integrate.
				 */
				void set_P_from_continuous(double _dt) {
					JFR_ASSERT(P_ct.size1() == size(), "Continuous-time covariance not yet initialized.");
					P(P_ct * _dt); // perturbation is random => variance is linear with time
				}
				/**
				 * Discrete perturbation from continuous specification.
				 * - The white, Gaussian random values integrate with the square root of dt. Their variance integrates linearly with dt:
				 *		- P = _P_ct * _dt
				 *
				 * \param _P_ct continuous-time perturbation covariances matrix.
				 * \param _dt the time interval to integrate.
				 */
				void set_P_from_continuous(sym_mat & _P_ct, double _dt) {
					JFR_ASSERT(_P_ct.size1() == size(), "Matrix sizes mismatch.");
					set_P_continuous(_P_ct);
					P(P_ct * _dt); // perturbation is random => variance is linear with time
				}
				/**
				 * Discrete perturbation from continuous specifications.
				 * - The deterministic values integrate with time normally, linearly with dt:
				 * 		- x = _x_ct * _dt
				 * - The white, Gaussian random values integrate with the square root of dt. Their variance integrates linearly with dt:
				 *		- P = _P_ct * _dt
				 *
				 * This function takes mean and covariances from the internal variables of the class (which are often constant).
				 * \param _dt the time interval to integrate.
				 */
				void set_from_continuous(double _dt) {
					JFR_ASSERT(x_ct.size() == size(), "Continuous-time values not yet initialized.");
					x(x_ct * sqrt(_dt)); // perturbation is random => mean is linear with square root of time
					P(P_ct * _dt); // perturbation is random => variance is linear with time
				}
				/**
				 * Discrete perturbation from continuous specifications.
				 * - The variance integrates linearly with dt:
				 *		- P = Pct.P * _dt
				 * - Therefore, the mean values integrate with time linearly with the square root of dt:
				 * 		- x = Pct.x * sqrt(_dt)
				 *
				 * \param Pct a continuous-time Gaussian process noise.
				 * \param _dt the time interval to integrate.
				 */
				void set_from_continuous(Gaussian & Pct, double _dt) {
					JFR_ASSERT(Pct.size() == size(), "Sizes mismatch");
					set_x_continuous(Pct.x());
					set_P_continuous(Pct.P());
					set_from_continuous(_dt);
				}
		};


	}
}

#endif /* PERTURBATION_HPP_ */
