/**
 * \file expectation.hpp
 *
 *  Created on: 25/03/2010
 *     \author: jsola@laas.fr
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

		using namespace std;
		using namespace jmath;
		using namespace jblas;


		/** Base class for all Gaussian expectations defined in the module rtslam.
		 *
		 * @ingroup rtslam
		 */
		class Expectation: public Gaussian {

			public:


				/**
				 * size constructor
				 */
				Expectation(const size_t _size);

				/**
				 * sizes constructor
				 */
				Expectation(const size_t _size, const size_t _size_nonobs, const size_t _size_state);

				/**
				 * Sizes and indirect_array constructor.
				 * The indirect array points to the states in the map that contributed to the expectation.
				 */
				Expectation(const size_t _size, const size_t _size_nonobs, const ind_array & _ia_x);

				jblas::vec nonObs; ///< expected value of the non-observable part.

				jblas::mat EXP_x; ///< Jacobian wrt the state.
				jblas::ind_array ia; ///< ind. array of indices to the map

				void computeVisibility();
				void estimateInfoGain();
				inline bool isVisible(); // landmark is visible (in Field Of View).
				inline double infoGain(); // expected "information gain" of performing an update with this observation.

			private:
				bool visible_;
				bool infoGain_;
		};

	}
}

#endif /* EXPECTATION_HPP_ */
