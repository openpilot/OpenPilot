/* $Id:$ */

#ifndef _QDISPLAY_ELLIPSOID_HPP_
#define _QDISPLAY_ELLIPSOID_HPP_

#include <jmath/jblas.hpp>
#include <qdisplay/Shape.hpp>

namespace jafar {
  namespace qdisplay {
    /**
     * @ingroup qdisplay
     * This class allow to display the uncertainty ellipsoid around a point.
     */
    class Ellipsoid : public Shape {
      public:
        /**
         * @param _x center of the ellipsoid
         * @param _xCov covariance matrix
         * @param _scale scale of the ellipsoid
         */
        Ellipsoid( const jblas::vec2& _x, const jblas::sym_mat22& _xCov, double _scale = 3.0 );
        Ellipsoid( double x, double y, double cov_x, double cov_y, double cov_xy, double _scale = 3.0 );
        void set( const jblas::vec2& _x, const jblas::sym_mat22& _xCov, double _scale = 3.0 );
    };
  }
}

#endif
