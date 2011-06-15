
/**
 * featureSegment.cpp
 *
 * \date 24/02/2011
 * \author bhautboi
 *
 *  \file featureSegment.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#ifdef HAVE_MODULE_DSEG

#include "rtslam/featureSegment.hpp"

namespace jafar {
   namespace rtslam {
      using namespace std;

      void FeatureImageSegment::setup(double _u1, double _v1, double _u2, double _v2, double _quality){
         measurement.x(0) = _u1;
         measurement.x(1) = _v1;
         measurement.x(2) = _u2;
         measurement.x(3) = _v2;
         measurement.matchScore = _quality;
      }

      void FeatureSegment::setup(double _u1, double _v1, double _u2, double _v2, double _quality){
         measurement.x(0) = _u1;
         measurement.x(1) = _v1;
         measurement.x(2) = _u2;
         measurement.x(3) = _v2;
         measurement.matchScore = _quality;
      }

   }
}

#endif /* HAVE_MODULE_DSEG */
