/* $Id$ */
#include <cmath>

#include "jmath/angle.hpp"

namespace jafar {
  namespace jmath {

    double degToRad(double a_) {
      return a_*M_PI/180.0;
    }

    double radToDeg(double a_) {
      return a_*180.0/M_PI;
    }

    double toMinusPiPi(double a_) {
      double a = a_;
      while (a <= -M_PI) {
        a += 2*M_PI;
      }
      while(a > M_PI) {
        a -= 2*M_PI;
      }
      return a;
    }

    double toMinus2Pi2Pi(double a_) {
      double a = a_;
      while (a <= -2.0*M_PI) {
        a += 2*M_PI;
      }
      while(a > 2*M_PI) {
        a -= 2*M_PI;
      }
      return a;
    }

} // namespace jmath
} // namespace jafar
