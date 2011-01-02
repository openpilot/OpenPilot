/* $Id$ */

#include "jafarConfig.h"

#ifdef HAVE_BOOST_SANDBOX
#ifdef HAVE_LAPACK

#include "assert.h"
#include <cmath>
#include <iostream> 
#include "boost/numeric/bindings/lapack/driver/gesdd.hpp"
#include "boost/numeric/bindings/ublas/matrix.hpp"
#include "boost/numeric/bindings/ublas/vector.hpp"

#include "kernel/jafarException.hpp"
#include "jmath/jmathException.hpp"
#include "jmath/vlls.hpp"

using namespace jafar::jmath;

VariableSizeLinearLeastSquares2::VariableSizeLinearLeastSquares2(size_t _modelSize)
  : m_modelSize( _modelSize ), m_dataSetSize(0), m_countValues(0) {
	m_x.resize(m_modelSize);
	m_xCov.resize(m_modelSize, m_modelSize);
}

VariableSizeLinearLeastSquares2::VariableSizeLinearLeastSquares2(size_t _modelSize, 
																																 size_t _dataSetSize)
  : m_modelSize( _modelSize ), m_dataSetSize(_dataSetSize), m_countValues(0) {
	m_A.resize(m_dataSetSize, m_modelSize);
	m_b.resize(m_dataSetSize);
	m_x.resize(m_modelSize);
	m_xCov.resize(m_modelSize, m_modelSize);
}

void VariableSizeLinearLeastSquares2::addMeasure(jblas::vec const& A_row, double b_val)
{
  JFR_PRECOND(A_row.size() == m_modelSize,
							"VariableSizeLinearLeastSquares::addMeasure: invalid A_row size: " << A_row.size());
	if(m_dataSetSize == 0) {
		m_A.resize(m_countValues+1, m_modelSize);
		m_b.resize(m_countValues+1);
	} else {
		if(m_countValues > m_dataSetSize)
			JFR_ERROR(JmathException, JmathException::SYSTEM_FULL, "system is full!");
	}
  row(m_A, m_countValues).assign(A_row);
  m_b(m_countValues) = b_val;
  ++m_countValues;
}

void VariableSizeLinearLeastSquares2::merge( const VariableSizeLinearLeastSquares2& _rhs)
{
  JFR_ASSERT( m_modelSize == _rhs.m_modelSize, "Can't merge solvers of different model size.");
  m_A += _rhs.m_A;
  m_b += _rhs.m_b;
  m_countValues += _rhs.m_countValues;
}

void VariableSizeLinearLeastSquares2::solve()
{
  namespace lapack = boost::numeric::bindings::lapack;
  using namespace ublas;
  using namespace jblas;
  
  vec s(m_modelSize);
  mat_column_major U(m_countValues, m_modelSize);
  mat_column_major VT(m_countValues, m_modelSize);

  int ierr = lapack::gesdd('A', m_A,s,U,VT);
  if (ierr!=0) {
    throw(jmath::LapackException(ierr, 
				 "LinearLeastSquares::solve: error in lapack::gesdd() routine",
				 __FILE__,
				 __LINE__));
  }

  // fill x
  m_x.clear();
  for (std::size_t i = 0 ; i < m_modelSize ; ++i) {
    m_x.plus_assign( (inner_prod(column(U,i),m_b) / s(i)) * row(VT,i) );
  }

  // fill xCov
  for (std::size_t i = 0 ; i < m_modelSize ; ++i) {
    row(VT,i) /= s(i);
  }

  m_xCov.assign(prod(trans(VT), VT));

}

#endif // HAVE_LAPACK
#endif // HAVE_BOOST_SANDBOX
