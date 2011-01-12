/**
 * \file featurePoint.cpp
 * \date 01/04/2010
 * \author jmcodol
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
