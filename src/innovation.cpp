/**
 * \file innovation.cpp
 *
 *  Created on: 25/03/2010
 *     \author: jsola@laas.fr
 *
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/innovation.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;


		Innovation::Innovation(const size_t _size) :
			Gaussian(_size), iP_(_size), INN_meas(_size, _size), INN_exp(_size, _size) {
		}


		Innovation::Innovation(const size_t _size, const size_t _size_meas, const size_t _size_exp) :
			Gaussian(_size), iP_(_size), INN_meas(_size, _size_meas), INN_exp(_size, _size_exp) {
		}


		Innovation::Innovation(const size_t _size, const size_t _size_meas, const ind_array & _ia_x) :
			Gaussian(_size), iP_(_size), INN_meas(_size, _size_meas), INN_rsl(_size, _ia_x.size()) {
		}

		void Innovation::compute(Expectation& exp, Measurement& meas) {
			compute(exp.x(), meas.x()); // We do not request trivial Jacobians here. Jacobians are the identity.
			P() = meas.P() + exp.P(); // Derived classes: P = Inn_meas*meas.P*Inn_meas.transpose() + Inn_exp*exp.P*Inn_exp.transpose();
			INN_rsl = prod(INN_exp, exp.EXP_rsl);
		}

	}
}
