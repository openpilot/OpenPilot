/**
 * \file innovation.cpp
 * \date 25/03/2010
 * \author jsola
 * \ingroup rtslam
 */

#include "rtslam/innovation.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;

		Innovation::Innovation(const size_t _size) :
			Gaussian(_size), iP_(_size) {
		}

		/**
		 * the inverse of the innovation covariance.
		 */
		void Innovation::invertCov() {
			jafar::jmath::ublasExtra::lu_inv(P(), iP_);
		}


		/**
		 * The Mahalanobis distance.
		 */
		double Innovation::mahalanobis() {
			invertCov();
			mahalanobis_ = ublasExtra::prod_xt_iP_x(iP_, x());
			return mahalanobis_;
		}


//		void Innovation::compute(Expectation& exp, Measurement& meas) {
//			mat INN_meas(size(), meas.size());
//			mat INN_exp(size(), exp.size());
//			compute(exp.x(), meas.x(), INN_meas, INN_exp); // We do not request trivial Jacobians here. Jacobians are the identity.
//			P() = meas.P() + exp.P(); // Derived classes: P = Inn_meas*meas.P*Inn_meas.transpose() + Inn_exp*exp.P*Inn_exp.transpose();
//		}

	}
}
