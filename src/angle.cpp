/* $Id$ */

#include "jmath/angle.hpp"
#include "jmath/constant.hpp"

namespace jafar {
  namespace jmath {

    double degToRad(double a_) {
      return a_*constant::PI/180.0;
    }

    double radToDeg(double a_) {
      return a_*180.0/constant::PI;
    }

    double toMinusPiPi(double a_) {
      double a = a_;
      while (a <= -constant::PI) {
        a += constant::TWO_PI;
      }
      while(a > constant::PI) {
        a -= constant::TWO_PI;
      }
      return a;
    }

    double toMinusTwoPiTwoPi(double a_) {
      double a = a_;
      while (a <= -1.0*constant::TWO_PI) {
        a += constant::TWO_PI;
      }
      while(a > constant::TWO_PI) {
        a -= constant::TWO_PI;
      }
      return a;
    }

} // namespace jmath
} // namespace jafar
