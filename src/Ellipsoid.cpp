/* $Id:$ */

#include "qdisplay/Ellipsoid.hpp"

#include "boost/numeric/bindings/traits/ublas_matrix.hpp"
#include <boost/numeric/bindings/lapack/syev.hpp>


#include <kernel/jafarMacro.hpp>

using namespace jafar::qdisplay;

Ellipsoid::Ellipsoid( double x, double y, double cov_x, double cov_y, double cov_xy, double scale ) :
	Shape( Shape::ShapeEllipse, 0.0, 0.0, 1.0, 1.0 )
{
	jblas::vec2 _x;
	_x(0) = x, _x(1) = y;
	jblas::sym_mat22 _xCov;
	_xCov(0,0) = cov_x, _xCov(1,1) = cov_y, _xCov(0,1) = cov_xy;
	set(_x, _xCov, scale);
}

Ellipsoid::Ellipsoid( const jblas::vec2& _x, const jblas::sym_mat22& _xCov, double _scale ) : 
	Shape( Shape::ShapeEllipse, 0.0, 0.0, 1.0, 1.0 )
{
	set(_x, _xCov, _scale);
}

void Ellipsoid::set( const jblas::vec2& _x, const jblas::sym_mat22& _xCov, double _scale )
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
    //JFR_DEBUG(_xCov << " " << lambda);
    if( lambda( 0 ) < 0 ) lambda( 0 ) = 0;
    if( lambda( 1 ) < 0 ) lambda( 1 ) = 0;
    setBoundingBox( _x(0), _x(1), _scale * 2*sqrt(lambda(0)), _scale * 2*sqrt(lambda( 1 ) ) );
    setTransform( QTransform( A(0,0), A(0,1), A(1,0), A(1, 1), 0, 0 ) );
  }
}


