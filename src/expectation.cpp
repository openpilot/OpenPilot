/**
 * \file expectation.cpp
 *
 * \date 25/03/2010
 * \author jsola@laas.fr
 *
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "jmath/ublasExtra.hpp"
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
			Gaussian(_size), nonObs(_size_nonobs) {
		}

	}
}
