
/**
 * featureAbstract.cpp
 *
 * \date 1/04/2010
 * \author jmcodol
 *
 *  \file featureAbstract.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/featurePoint.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;

		void FeatureImagePoint::setup(double _u, double _v, double _quality){
			measurement.x(0) = _u;
			measurement.x(1) = _v;
			measurement.matchScore = _quality;
		}

	}
}
