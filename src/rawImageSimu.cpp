/**
 * rawImageSimu.cpp
 *
 * \date 1/04/2010
 *      Author: jmcodol
 *
 *  \file rawImageSimu.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/rawImageSimu.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;

		//////////////////////////
		// RAW IMAGE SIMULATED
		//////////////////////////

		/**
		 * Add a Feature on the image simulated.
		 */
		 void RawImageSimu::addFeature(const FeatureAbstract) {
			 return ;
		 }
			/*
			 * Operator << for class rawAbstract.
			 * It shows some informations
			 */
			std::ostream& operator <<(std::ostream & s, jafar::rtslam::RawImageSimu & rawIS) {
				s << " I am a raw-data image simu" << endl;
				return s;
			}

	} // namespace rtslam
} // namespace jafar
