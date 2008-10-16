/* $Id:$ */

#include "qdisplay/Ellipsoid.hpp"

#include "boost/numeric/bindings/traits/ublas_matrix.hpp"
#include <boost/numeric/bindings/lapack/syev.hpp>


#include <kernel/jafarMacro.hpp>

using namespace jafar::qdisplay;

Ellipsoid::Ellipsoid( const jblas::vec2& _x, const jblas::sym_mat22& _xCov ) : Shape( Shape::ShapeEllipse, 0.0, 0.0, 1.0, 1.0 )
{
  namespace lapack = boost::numeric::bindings::lapack;
  jblas::vec2 lambda;
  ublas::matrix<double, ublas::column_major> A(ublas::project(_xCov, ublas::range(0,2), ublas::range(0,2))); 
  int ierr = lapack::syev( 'V', 'U', A, lambda, lapack::optimal_workspace() );
  JFR_POSTCOND(ierr==0,
      "Ellipsoid::Ellipsoid: error in lapack::syev() function, ierr=" << ierr);
  if (!ierr==0) {
    JFR_WARNING("Ellipsoid::Ellipsoid: error in lapack::syev() function, ierr=" << ierr);
  } else {
    setBoundingBox( _x(0), _x(1), 1, 1 );
//     setBoundingBox( _x(0), _x(1), lambda(0), lambda( 1 ) );
    JFR_DEBUG( lambda );
    setTransform( QTransform( A(0,0), A(0,1), A(1,0), A(1, 1), 0, 0 ) );
    
  }
}


