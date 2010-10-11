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
#include "jmath/ublasExtra.hpp"

#include "rtslam/gaussian.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;
		using namespace jblas;
		using namespace jmath::ublasExtra;

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
					Gaussian(_size), x_ct(_size), P_ct(_size) {
					x_ct.clear();
					P_ct.clear();
				}
				Perturbation(const vec & p, const sym_mat & P) :
					Gaussian(p, P), x_ct(p.size()), P_ct(p.size()) {
					x_ct.clear();
					P_ct.clear();
				}
				Perturbation(const vec & p, const vec & _std) :
					Gaussian(p.size()), x_ct(p.size()), P_ct(p.size()) {
					x_ct.clear();
					P_ct.clear();
					this->x(p);
					this->std(_std);
				}

				/**
				 * Continuous-time specified constructor.
				 * \param p the perturbation mean in continuous time.
				 * \param P the perturbation covariance in continuous time
				 * \param dt the sampling time
				 */
				Perturbation(const vec & p, const sym_mat & P, double dt) ;
				/**
				 * Continuous-time specified constructor.
				 * \param p the perturbation mean in continuous time.
				 * \param P the perturbation std. deviation in continuous time
				 * \param dt the sampling time
				 */
				Perturbation(const vec & p, const vec & _std, double dt) ;

				Perturbation(const Gaussian & p) :
					Gaussian(p), x_ct(p.size()), P_ct(p.size()) {
					x_ct.clear();
					P_ct.clear();
				}
				
				void clear()
				{
					Gaussian::clear();
					x_ct.clear();
					P_ct.clear();
				}
				
				template<class SymMat>
				void set_P_continuous(SymMat & _P_ct) {
					JFR_ASSERT(_P_ct.size1() == size(), "Matrix sizes mismatch.");
					P_ct = _P_ct;
				}
				template<class V>
				void set_x_continuous(V & _x_ct) {
					JFR_ASSERT(_x_ct.size() == size(), "Vector sizes mismatch.");
					x_ct = _x_ct;
				}
				/**
				 * Set continuous-time covariance from standard deviation specification
				 * \param _std a vector with the standard deviations.
				 */
				void set_std_continuous(const vec & _std) {
					JFR_ASSERT(size() == _std.size(), "Sizes mismatch");
					P_ct.clear();
					size_t i;
					for (i = 0 ; i< _std.size() ; i++)
						P_ct(i,i) = _std(i)*_std(i);
//cout << "setting P_ct: " << P_ct << endl;
				}
				/**
				 * Set continuous-time perturbation
				 */
				void set_continuous(const vec & _x, const vec & _std){
					set_x_continuous(_x);
					set_std_continuous(_std);
				}
				/**
				 * Set continuous-time perturbation
				 */
				void set_continuous(const vec & _x, const sym_mat & _P){
					set_x_continuous(_x);
					set_P_continuous(_P);
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
//JFR_DEBUG("setting P: " << P() << " using P_ct: " << P_ct << " and dt " << _dt);
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
