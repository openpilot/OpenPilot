/**
 * \file expectation.cpp
 *
 *  Created on: 25/03/2010
 *     \author: jsola@laas.fr
 *
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/expectation.hpp"


namespace jafar {
	namespace rtslam {
		using namespace std;

		/////////////////////////////////
		// EXPECTATION
		/////////////////////////////////

		/*
		 * Size constructor
		 */
		Expectation::Expectation(const size_t _size) :
			Gaussian(_size) {
		}

		/*
		 * sizes constructor
		 */
		Expectation::Expectation(const size_t _size, const size_t _size_nonobs, const size_t _size_state) :
			Gaussian(_size), nonObs(_size_nonobs), EXP_rsl(_size, _size_state), ia_rsl(_size_state) {
		}

		/*
		 * sizes constructor
		 */
		Expectation::Expectation(const size_t _size, const size_t _size_nonobs, const ind_array & _ia_x) :
			Gaussian(_size), nonObs(_size_nonobs), EXP_rsl(_size, _ia_x.size()), ia_rsl(_ia_x) {
		}

		bool Expectation::isVisible() {
			return visible_;
		} // landmark is visible (in Field Of View).

		double Expectation::infoGain() {
			return infoGain_;
		}	 // expected "information gain" of performing an update with this observation.

	}
}
