/* $Id$ */

#include "jafarConfig.h"

#ifdef HAVE_BOOST_SANDBOX
#ifdef HAVE_LAPACK

#include <iostream> // gesdd uses std::cerr but does not includes iostream
#include "boost/numeric/bindings/lapack/gesdd.hpp"
#include "boost/numeric/bindings/traits/ublas_matrix.hpp"
#include "boost/numeric/bindings/traits/ublas_vector.hpp"

#include "kernel/jafarException.hpp"

#include "jmath/linearLeastSquares.hpp"

using namespace jafar::jmath;


void LinearLeastSquares::setSize(std::size_t sizeModel, std::size_t sizeDataSet)
{
  JFR_PRECOND(sizeDataSet >= sizeModel,
	      "LinearLeastSquares::setSize: invalid size");

  m_sizeModel = sizeModel;
  m_sizeDataSet = sizeDataSet;

  m_A.resize(sizeDataSet, sizeModel);
  m_b.resize(sizeDataSet);
  m_x.resize(sizeModel);
  m_xCov.resize(sizeModel, sizeModel);
}

void LinearLeastSquares::setDataSetSize(std::size_t sizeDataSet)
{
  setSize(sizeModel(), sizeDataSet);
}

void LinearLeastSquares::solve()
{
  namespace lapack = boost::numeric::bindings::lapack;
  using namespace ublas;
  using namespace jblas;
  
  vec s(sizeModel());
  mat_column_major U(sizeDataSet(), sizeModel());
  mat_column_major VT(sizeModel(), sizeModel());

  int ierr = lapack::gesdd(m_A,s,U,VT);
  JFR_POSTCOND(ierr==0,
	       "LinearLeastSquares::solve: error in lapack::gesdd() function, ierr=" << ierr);

  // fill x
  m_x.clear();
  for (std::size_t i = 0 ; i < sizeModel() ; ++i) {
    m_x.plus_assign( (inner_prod(column(U,i),m_b) / s(i)) * row(VT,i) );
  }

  // fill xCov
  for (std::size_t i = 0 ; i < sizeModel() ; ++i) {
    row(VT,i) /= s(i);
  }

  m_xCov.assign(prod(trans(VT), VT));

}


#endif // HAVE_LAPACK
#endif // HAVE_BOOST_SANDBOX
