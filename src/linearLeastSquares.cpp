/* $Id$ */

#include "jafarConfig.h"

#ifdef HAVE_BOOST_SANDBOX
#ifdef HAVE_LAPACK

#include "assert.h"

#include <cmath>

#include <iostream> // gesdd uses std::cerr but does not includes iostream
#include "boost/numeric/bindings/lapack/gesdd.hpp"
#include "boost/numeric/bindings/traits/ublas_matrix.hpp"
#include "boost/numeric/bindings/traits/ublas_vector.hpp"

#include "kernel/jafarException.hpp"

#include "jmath/jmathException.hpp"
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

void LinearLeastSquares::setData(std::size_t index, jblas::vec const& A_row, double b_val)
{
  JFR_PRECOND(index < sizeDataSet(),
	      "LinearLeastSquares::setData: invalid index: " << index);
  JFR_PRECOND(A_row.size() == sizeModel(),
	      "LinearLeastSquares::setData: invalid A_row size: " << A_row.size());
  row(m_A, index).assign(A_row);
  m_b(index) = b_val;
}

void LinearLeastSquares::setData(std::size_t index, jblas::vec const& A_row, double b_val, double weight)
{
  JFR_TRACE_BEGIN;
  double sqrt_weight = sqrt(weight);
  setData(index, sqrt_weight*A_row, sqrt_weight*b_val);
  JFR_TRACE_END("LinearLeastSquares::setData");
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
  if (ierr!=0) {
    throw(jmath::LapackException(ierr, 
				 "LinearLeastSquares::solve: error in lapack::gesdd() routine",
				 __FILE__,
				 __LINE__));
  }

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
