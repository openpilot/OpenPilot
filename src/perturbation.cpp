/*
 * \file perturbation.cpp
 * \date 19/06/2010
 * \author jsola
 * \ingroup rtslam
 */

#include "rtslam/perturbation.hpp"

namespace jafar {
	namespace rtslam {
		using namespace jblas;

		Perturbation::Perturbation(const vec & p, const sym_mat & P, double dt) :
			Gaussian(p.size()) , x_ct(p.size()), P_ct(p.size()) {
			set_x_continuous(p);
			set_P_continuous(P);
			set_from_continuous(dt);
		}
		Perturbation::Perturbation(const vec & _p, const vec & _std, double _dt) :
			Gaussian(_p.size()) , x_ct(_p.size()), P_ct(_p.size()) {
			set_x_continuous(_p);
			set_std_continuous(_std);
			set_from_continuous(_dt);
		}

	}
}
