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
			Gaussian(_size), iP_(_size) {
		}

		void Innovation::compute(Expectation& exp, Measurement& meas) {
			mat INN_meas(size(), meas.size());
			mat INN_exp(size(), exp.size());
			compute(exp.x(), meas.x(), INN_meas, INN_exp); // We do not request trivial Jacobians here. Jacobians are the identity.
			P() = meas.P() + exp.P(); // Derived classes: P = Inn_meas*meas.P*Inn_meas.transpose() + Inn_exp*exp.P*Inn_exp.transpose();
		}

	}
}
