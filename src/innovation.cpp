/**
 * \file innovation.cpp
 *
 *  Created on: 25/03/2010
 *     \author: jsola@laas.fr
 *
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/innovation.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;


		///////////////////////////
		// INNOVATION
		///////////////////////////

		/*
		 * Size constructor
		 */
		Innovation::Innovation(const size_t _size) :
			Gaussian(_size), iP_(_size), INN_meas(_size, _size), INN_exp(_size, _size) {
		}


		/*
		 * Sizes construction.
		 */
		Innovation::Innovation(const size_t _size, const size_t _size_meas, const size_t _size_exp) :
			Gaussian(_size), iP_(_size), INN_meas(_size, _size_meas), INN_exp(_size, _size_exp) {
		}


		/**
		 * Sizes and indirect_array constructor.
		 * The indirect array points to the states in the map that contributed to the innovation.
		 */
		Innovation::Innovation(const size_t _size, const size_t _size_meas, const ind_array & _ia_x) :
			Gaussian(_size), iP_(_size), INN_meas(_size, _size_meas), INN_x(_size, _ia_x.size()) {
		}

	}
}
