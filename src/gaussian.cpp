/**
 * \file gaussian.cpp
 * \date 14/03/2010
 * \author jsola
 * \ingroup rtslam
 */

#include "rtslam/gaussian.hpp"
#include "rtslam/rtslamException.hpp"
#include "jmath/jblas.hpp"
#include "jmath/ublasExtra.hpp"

namespace jafar {
	namespace rtslam {

		using namespace jblas;
		using namespace jmath;

		/*
		 * Class Gaussian
		 */
		double Gaussian::probabilityDensity(const jblas::vec& v) const {
			JFR_PRECOND(v.size() == size_,
					"GaussianVector::value: size of v must match size of gaussian");

			mat w_P(P_);
			mat P_inv(size(), size());
			ublasExtra::inv(w_P, P_inv);

			vec y = v - x_;
			double num = exp(-0.5 * inner_prod(y, prod(P_inv, y)));

			double den = pow(2 * M_PI, (double) (size() / 2)) * sqrt(ublasExtra::lu_det(w_P));
			return num / den;
		}

	}
}
