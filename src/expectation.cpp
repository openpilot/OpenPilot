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
		Expectation::Expectation(const size_t _size, const size_t _size_nonobs) :
			Gaussian(_size) {
		}

		bool Expectation::isVisible() {
			return visible_;
		} // landmark is visible (in Field Of View).

		double Expectation::infoGain() {
			return infoGain_;
		} // expected "information gain" of performing an update with this observation.

	}
}
