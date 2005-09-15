/* $Id$ */\

#ifndef JMATH_CONSTANT_HPP
#define JMATH_CONSTANT_HPP

#include <cmath>

#ifdef PI
#undef PI
#warning "preprocessor symbol PI undefined"
#endif

namespace jafar {
  namespace jmath {

    /** This namespace contains usefull constants.
     *
     * \ingroup jmath
     */
    namespace constant {

      /// PI
      const double PI = M_PI;

      /// 2*PI
      const double TWO_PI = 2*M_PI;

      const double EPSILON = 1e-8;

    }

  } // namespace jmath
} // namespace jafar

#endif // JMATH_CONSTANT_HPP
