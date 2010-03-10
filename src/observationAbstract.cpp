/**
 * observationAbstract.cpp
 *
 *  Created on: 10/03/2010
 *      Author: jsola
 *
 *  \file observationAbstract.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/observationAbstract.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;

		Expectation::Expectation(const size_t _size) :
			Gaussian(_size) {
		}

		/*
		 * sizes constructor
		 */
		Expectation::Expectation(const size_t _size, const size_t _size_nonobs) :
			Gaussian(_size), nonObs(_size_nonobs) {
		}

		/*
		 * Size constructor
		 */
		Innovation::Innovation(const size_t _size) :
			Gaussian(_size), iP_(_size) {
		}

		/*
		 * Sizes construction.
		 */
		Innovation::Innovation(const size_t _size, const size_t _size_meas, const size_t _size_exp) :
			Gaussian(_size), iP_(_size), INN_meas(_size, _size_meas), INN_exp(_size, _size_exp) {
		}

	} // namespace rtslam
} // namespace jafar
