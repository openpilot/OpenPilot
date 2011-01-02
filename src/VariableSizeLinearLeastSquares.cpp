/* $Id$ */

#include "jmath/VariableSizeLinearLeastSquares.hpp"

#include "kernel/jafarMacro.hpp"

#ifdef HAVE_BOOST_SANDBOX
#ifdef HAVE_LAPACK

#include "boost/numeric/bindings/lapack/driver/gesv.hpp"
#include "boost/numeric/bindings/ublas/matrix.hpp"
#include "boost/numeric/bindings/ublas/vector.hpp"


namespace lapack = boost::numeric::bindings::lapack;
using namespace jafar::jmath;

VariableSizeLinearLeastSquares::VariableSizeLinearLeastSquares(int _modelSize)
  : m_modelSize( _modelSize ), m_A( m_modelSize), m_b(m_modelSize, m_modelSize), m_x(m_modelSize), m_b_val(0), m_countValues(0)
{
  m_A.assign( jblas::zero_mat( m_modelSize ) );
  m_b.assign( jblas::zero_mat( m_modelSize ) );
}

void VariableSizeLinearLeastSquares::solve()
{
  ublas::matrix<double, ublas::column_major> Abis(m_A);
  ublas::matrix<double, ublas::column_major> bbis(m_b);
	ublas::vector<int> ipiv(m_modelSize);
  lapack::gesv(Abis,ipiv,bbis);
  m_x = column(bbis,0);
}

void VariableSizeLinearLeastSquares::addMeasure( const jblas::vec& dataPoint, double b_val)
{
  for( int i = 0; i < m_modelSize; ++i)
  {
    for( int j = i; j < m_modelSize; ++j)
    {
      m_A(i,j) += dataPoint(i) * dataPoint(j);
    }
    m_b(i,0) -= dataPoint(i) * b_val;
  }
  m_b_val += b_val * b_val;
  ++m_countValues;
}

double VariableSizeLinearLeastSquares::computeMinimum() const
{
  double err = 0;
  
  for( int i = 0; i < m_modelSize; ++i)
  {
    for( int j = 0; j < m_modelSize; ++j)
    {
      err += m_A(i,j) * m_x(i) * m_x(j);
    }
  }
  return err - 2.0 * ublas::inner_prod( m_x, column(m_b,0) ) + m_b_val;
}

void VariableSizeLinearLeastSquares::merge( const VariableSizeLinearLeastSquares& _rhs)
{
  JFR_ASSERT( m_modelSize == _rhs.m_modelSize, "Can't merge solvers of different model size.");
  m_A += _rhs.m_A;
  m_b += _rhs.m_b;
  m_b_val += _rhs.m_b_val;
  m_countValues += _rhs.m_countValues;
}

#endif
#endif
