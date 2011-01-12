/**
 * \file measurement.cpp
 * \date 25/03/2010
 * \author jsola
 * \ingroup rtslam
 */


#include "rtslam/measurement.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;

		/////////////////////////
			// MEASUREMENT
			/////////////////////////

			/*
			 * Size constructor
			 */
			Measurement::Measurement(size_t _size) :
				Gaussian(_size), matchScore(0) {
			}


	}
}
