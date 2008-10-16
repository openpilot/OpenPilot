/* $Id:$ */

#ifndef _QDISPLAY_ELLIPSOID_HPP_
#define _QDISPLAY_ELLIPSOID_HPP_

#include <jmath/jblas.hpp>
#include <qdisplay/Shape.hpp>

namespace jafar {
  namespace qdisplay {
    class Ellipsoid : public Shape {
      Ellipsoid( const jblas::vec2& _x, const jblas::sym_mat22& _xCov );
    };
  }
}

#endif
