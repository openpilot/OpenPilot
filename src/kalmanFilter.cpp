/**
 * \file kalmanFilter.cpp
 *
 * Extended Kalman filter with sparse access governed by boost::indirect_array
 *
 *   \author: jsola@laas.fr
 *
 *  Created on: 25/03/2010
 *
 *
 * \ingroup rtslam
 */

#include "rtslam/kalmanFilter.hpp"
#include "rtslam/observationAbstract.hpp"
#include "jmath/jblas.hpp"
#include "jmath/ublasExtra.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;
		using namespace jmath;
		using namespace jblas;
		using namespace ublas;

		ExtendedKalmanFilterIndirect::ExtendedKalmanFilterIndirect(size_t _size) :
			size(_size), x_(size), P_(size) {
			x_.clear();
			P_.clear();
		}

		void ExtendedKalmanFilterIndirect::predict(const ind_array & iax, const mat & F_v, const ind_array & iav,
		    const mat & F_u, const sym_mat & U) {
			jafar::jmath::ublasExtra::ixaxpy_prod(P_, iax, F_v, iav);
			ublas::project(P_, iav, iav) += jafar::jmath::ublasExtra::prod_JPJt(U, F_u);
		}
		void ExtendedKalmanFilterIndirect::predict(const ind_array & iax, const mat & F_v, const ind_array & iav,
		    const sym_mat & Q) {
			jafar::jmath::ublasExtra::ixaxpy_prod(P_, iax, F_v, iav, iav, Q);
		}

		void ExtendedKalmanFilterIndirect::correct(const ind_array & iax, Innovation & inn) {
			PHt_tmp = ublas::prod(ublas::project(P_, iax, inn.ia_inn_x), trans(inn.INN_x));
			inn.invertCov();
			K = prod(PHt_tmp, inn.iP_);
			P_ += prod<sym_mat>(K, trans(PHt_tmp));
		}

	}
}
