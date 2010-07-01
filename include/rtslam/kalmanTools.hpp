/*
 * kalmanTools.hpp
 *
 *     Project: jafar
 *  Created on: Jul 1, 2010
 *      Author: jsola
 */

#ifndef KALMANTOOLS_HPP_
#define KALMANTOOLS_HPP_

#include "kernel/jafarDebug.hpp"

#include "jmath/jblas.hpp"
#include "jmath/indirectArray.hpp"
#include "rtslam/innovation.hpp"

namespace jafar {
	namespace rtslam {
		namespace kalman {
			using namespace jblas;

			template<class SM, class MJ, class MK>
			void computeKalmanGain(const SM & P, const ind_array & ia_x, Innovation & inn, const MJ & INN_x1, const ind_array & ia_x1, MK & K){
					JFR_ASSERT(P.size1() > ia_x.size(), "indirect indexing too large for matrix P");
					JFR_ASSERT(INN_x1.size1() == inn.size(), "sizes mismatch: INN_x1 and inn");
					JFR_ASSERT(INN_x1.size2() == ia_x1.size(), "sizes mismatch: INN_x1 and ia_x1");
					JFR_ASSERT(K.size1() == ia_x.size(), "sizes mismatch: K and ia_x");
					JFR_ASSERT(K.size2() == inn.size(), "sizes mismatch: K and inn");

				inn.invertCov();
				mat K = - prod(prod(project(P, ia_x, ia_x1), trans(INN_rsl)), inn.iP_);
			}

		}
	}
}


#endif /* KALMANTOOLS_HPP_ */
