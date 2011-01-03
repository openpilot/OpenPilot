/**
 * \file expectation.hpp
 *
 * \date 25/03/2010
 * \author jsola@laas.fr
 *
 *
 *  This file defines the class Expectation.
 *
 * \ingroup rtslam
 */

#include "jmath/jblas.hpp"

#include "rtslam/gaussian.hpp"

#ifndef EXPECTATION_HPP_
#define EXPECTATION_HPP_

namespace jafar {
	namespace rtslam {

		using namespace jmath;
		using namespace jblas;


		/** Base class for all Gaussian expectations defined in the module rtslam.
		 *
		 * The expectation is defined as the projection of the state into the measurement space:
		 * - this->x() = exp = h(x)
		 *
		 * where the function h(x) belongs to some observation model.
		 *
		 * Its covariances matrix is computed via Gaussian uncertainty propagation using the Jacobians of h():
		 * - this->P() = EXP = EXP_rsl * P * EXP_rsl'
		 *
		 * with EXP_rsl = dexp/dx = dh(x)/dx, computed also by the observation model.
		 *
		 * @ingroup rtslam
		 */
		class Expectation: public Gaussian {

			public:


				/**
				 * size constructor
				 */
				Expectation(const size_t _size, const size_t _size_nonobs = 0);

				vec nonObs; ///<         Expected value of the non-observable part.
				bool visible;
				double infoGain;

				friend std::ostream& operator <<(std::ostream & s, Expectation const & e_) {
					s << (Gaussian&)e_ << std::endl;
					s << "  .nonObs : " << e_.nonObs << " | .infoGain : " << e_.infoGain;
					return s;
				}
		};

	}
}

#endif /* EXPECTATION_HPP_ */
