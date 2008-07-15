/* $Id linearSolver.cpp  */
#include "jmath/linearSolver.hpp"

#include "jafarConfig.h"

#ifdef HAVE_BOOST_SANDBOX
#ifdef HAVE_LAPACK

#include "assert.h"
#include <cmath>
#include <iostream> // gesdd uses std::cerr but does not includes iostream

#include <boost/numeric/bindings/lapack/geqrf.hpp>
#include <boost/numeric/bindings/lapack/gesdd.hpp>
#include <boost/numeric/bindings/lapack/ormqr.hpp>
#include <boost/numeric/bindings/lapack/orgqr.hpp>
#include <boost/numeric/bindings/lapack/gesv.hpp>
#include <boost/numeric/bindings/lapack/posv.hpp>
#include <boost/numeric/bindings/lapack/sysv.hpp>

#include "boost/numeric/bindings/traits/ublas_matrix.hpp"
#include <boost/numeric/bindings/traits/ublas_vector.hpp>
#include <boost/numeric/ublas/triangular.hpp>
#include <boost/numeric/ublas/blas.hpp>
#include "kernel/jafarException.hpp"
#include "jmath/jmathException.hpp"
#include "jmath/linearLeastSquares.hpp"

namespace LinearSolver {

using namespace jafar;
using namespace jblas;
using namespace std;
namespace lapack = boost::numeric::bindings::lapack;
namespace ublas = boost::numeric::ublas;

int solve_QR(jblas::mat A, jblas::vec& b, jblas::vec& x){
  JFR_PRECOND(A.size1() == A.size2() == b.size(),
	      "LinearSolver: invalid size. A is mxm and b is mx1");
  size_t size = b.size();
  int error = 0;
  ublas::matrix<double, ublas::column_major> m_A(size, size);
  m_A.assign(A);
  jblas::mat at(size, size);
  ublas::triangular_matrix<double, ublas::upper>R (size, size);
  jblas::vec tau(size);
  /*QR decomposition of A*/
  error = lapack::geqrf(m_A, tau);
  if (error != 0){
     throw(jmath::LapackException(error, 
				 "solve_QR: error in lapack::geqrf() routine",
				 __FILE__,
				 __LINE__));
     return 0;
  }
  /*store R (R is returned in the upper triangular of A)*/
  for(uint i=0; i < size; ++i) {
    for (uint j=i; j < size; ++j){
      R(i,j) = m_A(i,j);
    }
  }

  /*compute Q (Q is returned in A)*/
  error = lapack::orgqr(m_A, tau);
  if (error != 0){
     throw(jmath::LapackException(error, 
				 "solve_QR: error in lapack::orgqr() routine",
				 __FILE__,
				 __LINE__));
     return 0;
  }
  /* solve R x = Q^t b */
  jblas::vec atb(b.size());
  atb = ublas::prod(ublas::trans(m_A), b);
  ublas::blas_3::tsm(atb, 1.0, ublas::trans(R), ublas::upper_tag());
  return 1;
}

int solve_QR_noQ(jblas::mat A, jblas::vec& b, jblas::vec& x){
  JFR_PRECOND(A.size1() == A.size2() == b.size(),
	      "LinearSolver: invalid size. A is mxm and b is mx1");
  size_t size = b.size();
  int error = 0;
  ublas::matrix<double, ublas::column_major> m_A(size, size);
  m_A.assign(A);
  jblas::mat at(size, size); 
  ublas::triangular_matrix<double, ublas::upper> R(size, size);
  jblas::vec tau(size);

  /* QR decomposition of A */
  error = lapack::geqrf(m_A, tau);
  if (error != 0){
     throw(jmath::LapackException(error, 
				 "solve_QR_noQ: error in lapack::geqrf() routine",
				 __FILE__,
				 __LINE__));
     return 0;
  }
  /* store R (R is returned in the upper triangular of A) */
  for(uint i=0; i < size; ++i) {
    for (uint j=i; j < size;++j){
      R(i,j) = m_A(i,j);
    }
  }
  /* solve R^t y = A^t b */
  jblas::vec atb(b.size());
  atb = ublas::prod(ublas::trans(m_A), b);
  ublas::blas_3::tsm(atb, 1.0, ublas::trans(R), ublas::upper_tag());
  /* solve Rx = y (y was saved in A^t) */
  ublas::blas_3::tsm(atb, 1.0, R, ublas::upper_tag());
  return 1;
}

int solve_Cholesky(jblas::mat A, jblas::vec& b, jblas::vec& x){
  JFR_PRECOND(A.size1() == A.size2() == b.size(),
	      "LinearSolver: invalid size. A is mxm and b is mx1");
  size_t size = b.size();
  int error = 0;
  jblas::mat_column_major m_A(size, size);
  m_A.assign(A);
  jblas::mat at(size, size); 
  error = lapack::potrf('U', m_A);
  if (error != 0){
     throw(jmath::LapackException(error, 
				 "solve_Cholesky: error in lapack::potrf() routine",
				 __FILE__,
				 __LINE__));
     return 0;
  }
  /* solve the linear system U^t y = b */
  at = ublas::trans(m_A);
  ublas::blas_3::tsm(b, 1.0, at, ublas::upper_tag());
  /* solve the linear system U x = b */
  ublas::blas_3::tsm(b, 1.0, A, ublas::upper_tag());
  return 1;
}

int solve_LU(jblas::mat A, jblas::vec& b, jblas::vec& x){
  JFR_PRECOND(A.size1() == A.size2() == b.size(),
	      "LinearSolver: invalid size. A is mxm and b is mx1");
  size_t size = b.size();
  int error = 0;
  jblas::veci ipiv(size);
  jblas::mat_column_major m_A(size, size);
  m_A.assign(A);

  /* LU decomposition of A */
  error = lapack::getrf(m_A, ipiv);
  if (error != 0){
     throw(jmath::LapackException(error, 
				 "solve_LU: error in lapack::getrf() routine",
				 __FILE__,
				 __LINE__));
     return 0;
  }

  /* solve with computed LU */
  jblas::mat_column_major B(size,1);		// getrs accepts two matrices
  column(B,0) = b;	// assign b to first column of B (need matrix_proxy.hpp)
  error = lapack::getrs(m_A, ipiv, B);
  if (error != 0){
     throw(jmath::LapackException(error, 
				 "solve_LU: error in lapack::getrs() routine",
				 __FILE__,
				 __LINE__));
     return 0;
  }

  /* fill x */
  x.clear();
  x.assign(column(B, 0));
  return 1;
}

int solve_SVD(jblas::mat A, jblas::vec& b, jblas::vec& x){
  JFR_PRECOND(A.size1() == A.size2() == b.size(),
	      "LinearSolver: invalid size. A is mxm and b is mx1");
  size_t size = b.size();
  int error = 0;
  jblas::mat_column_major m_A(size, size);
  m_A.assign(A);
  jblas::mat_column_major U(size, size), VT(size, size);
  jblas::vec s(size);
  /* SVD decomposition of A */
  error = lapack::gesdd(m_A,s,U,VT);
  if (error!=0) {
    throw(jmath::LapackException(error,
                                 "solve_LU: error in lapack::gesdd() routine",
                                 __FILE__,
                                 __LINE__));
  }
  /* fill x */
  x.clear();
  for (uint i = 0 ; i < size ; ++i) {
    x.plus_assign( (inner_prod(column(U,i), b) / s(i)) * row(VT,i) );
  }
  return 1;
}

int solve_BK(jblas::mat A, jblas::vec& b, jblas::vec& x){
  JFR_PRECOND(A.size1() == A.size2() == b.size(),
	      "LinearSolver: invalid size. A is mxm and b is mx1");
  size_t size = b.size();
  int error = 0;
  jblas::veci ipiv(size);
  /* factorize A */
  jblas::mat_column_major m_A(size, size);
  m_A.assign(A);
  error = lapack::sytrf('U', m_A, ipiv);
  if (error!=0) {
    throw(jmath::LapackException(error,
                                 "solve_BK: error in lapack::sytrf() routine",
                                 __FILE__,
                                 __LINE__));
  }
  /* solve the system with the computed factorization */
  jblas::mat_column_major B(size,1);		// sytrs accepts two matrices
  column(B,0) = b;	// assign b to first column of B (need matrix_proxy.hpp)
  error = lapack::sytrs('U', m_A, ipiv, B);
  if (error!=0) {
    throw(jmath::LapackException(error,
                                 "solve_BK: error in lapack::sytrs() routine",
                                 __FILE__,
                                 __LINE__));
  }
  /* fill x */
  x.clear();
  x = column(B, 0);
  return 1;
}

// int invert_Cholesky(mat A, mat& A_inv){
//   int size = b.size();
//   int error = 0;
//   /* force a major column version of A */
//   if (!isColMaj){
//     A =  ublas::trans(A);
//   }
//   error = lapack::potrf("L", A);
//   if (error != 0){
//      throw(jmath::LapackException(error, 
// 				 "invert_Cholesky: error in lapack::potrf() routine",
// 				 __FILE__,
// 				 __LINE__));
//      return 0;
//   }
//   error = lapack::potri("L", A);
//   if (error != 0){
//      throw(jmath::LapackException(error, 
// 				 "invert_Cholesky: error in lapack::potri() routine",
// 				 __FILE__,
// 				 __LINE__));
//      return 0;
//   }
//   /* store A_inv (the lower trianglar of A^-1 (symmetric) is returned in the lower triangular of A) */
//   for(int i=0; i < size; ++i) {
//     for (int j=0; j <= i;++j){
//       A_inv(j,i) = A_inv(i,j) = A(i,j);
//     }
//   }
//   return 1;
// }
}
#endif
#endif
