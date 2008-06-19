/* $Id linearSolver.cpp  */
#include "jmath/linearSolver.hpp"

#include "jafarConfig.h"

#ifdef HAVE_BOOST_SANDBOX
#ifdef HAVE_LAPACK

#include "assert.h"
#include <cmath>
#include <iostream> // gesdd uses std::cerr but does not includes iostream

#include "boost/numeric/bindings/lapack/geqrf.hpp"
#include "boost/numeric/bindings/lapack/gesdd.hpp"
#include "boost/numeric/bindings/lapack/ormqr.hpp"
#include "boost/numeric/bindings/lapack/orgqr.hpp"
#include "boost/numeric/bindings/lapack/gesv.hpp"
#include "boost/numeric/bindings/lapack/posv.hpp"
#include "boost/numeric/bindings/lapack/sysv.hpp"

#include "boost/numeric/bindings/traits/ublas_matrix.hpp"
#include "boost/numeric/bindings/traits/ublas_vector.hpp"
#include <boost/numeric/ublas/triangular.hpp>
#include <boost/numeric/ublas/blas.hpp>
#include "kernel/jafarException.hpp"
#include "jmath/jmathException.hpp"
#include "jmath/linearLeastSquares.hpp"

using namespace std;
using namespace jafar::jmath;
using namespace jblas;
namespace lapack = boost::numeric::bindings::lapack;
namespace ublas = boost::numeric::ublas;
LinearSolver::LinearSolver(const mat_column_major& A, const vec& b){
//   if (A.size1() < A.size2()) {
//     for(int i=0; i <size; ++i){
//       for(int j=0; j <size; ++j){
//         m_A(i,j) = A(j,i);
//       }
//     }
//   } else {
//     m_A = A;
//   }
  JFR_PRECOND(A.size1() == A.size2() == b.size(),
	      "LinearSolver: invalid size. A is mxm and b is mx1");
  size = b.size();
  m_A.resize(size, size, false);
  m_b.resize(size, false);
  m_x.resize(size, false);
  m_A = A;
  m_b = b;
}
/*
 * This function returns the solution of Ax = b
 *
 * The function is based on QR decomposition with explicit computation of Q:
 * If A=Q R with Q orthogonal and R upper triangular, the linear system becomes
 * Q R x = b or R x = Q^T b.
 *
 * The function returns 0 in case of error, 1 if successfull
 */
int LinearSolver::solve_QR(){
  int error = 0;
  jblas::mat at(size, size);
  ublas::triangular_matrix<double, ublas::upper>R (size, size);
  jblas::vec tau(size);
  /*QR decomposition of A*/
  error = lapack::geqrf(m_A, tau);
  if (error != 0){
     throw(jmath::LapackException(error, 
				 "LinearSolver::solve_QR: error in lapack::geqrf() routine",
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
				 "LinearSolver::solve_QR: error in lapack::orgqr() routine",
				 __FILE__,
				 __LINE__));
     return 0;
  }
  /* solve R x = Q^t b */
  at = ublas::trans(m_A);
  boost::numeric::ublas::blas_3::tsm(at, m_b, R, ublas::upper_tag());
  std::cout << at << std::endl;
  return 1;
}

/*
 * This function returns the solution of Ax = b
 *
 * The function is based on QR decomposition without computation of Q:
 * If A=Q R with Q orthogonal and R upper triangular, the linear system becomes
 * (A^T A) x = A^T b or (R^T Q^T Q R) x = A^T b or (R^T R) x = A^T b.
 * This amounts to solving R^T y = A^T b for y and then R x = y for x
 * Note that Q does not need to be explicitly computed
 *
 * The function returns 0 in case of error, 1 if successfull
 */
int LinearSolver::solve_QR_noQ(){
  int error = 0;
  jblas::mat at(size, size); 
  ublas::triangular_matrix<double, ublas::upper> R(size, size);
  jblas::vec tau(size);

  /* QR decomposition of A */
  error = lapack::geqrf(m_A, tau);
  if (error != 0){
     throw(jmath::LapackException(error, 
				 "LinearSolver::solve_QR_noQ: error in lapack::geqrf() routine",
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
  at = ublas::trans(m_A);
  ublas::blas_3::tsm(at, m_b, ublas::trans(R), ublas::upper_tag());
  /* solve Rx = y (y was saved in A^t) */
  ublas::blas_3::tsm(at, jblas::unit_vec(size), R, ublas::upper_tag());
  std::cout << at << std::endl;
  return 1;
}

/*
 * This function returns the solution of Ax=b
 *
 * The function assumes that A is symmetric & positive definite and employs
 * the Cholesky decomposition:
 * If A=U^T U with U upper triangular, the system to be solved becomes
 * (U^T U) x = b
 * This amount to solving U^T y = b for y and then U x = y for x
 *
 * The function returns 0 in case of error, 1 if successfull
 */
int LinearSolver::solve_Cholesky(){
  int error = 0;
  jblas::mat at(size, size); 
  error = lapack::potrf('U', m_A);
  if (error != 0){
     throw(jmath::LapackException(error, 
				 "LinearSolver::solve_Cholesky: error in lapack::potrf() routine",
				 __FILE__,
				 __LINE__));
     return 0;
  }
  /* solve the linear system U^t y = b */
  at = ublas::trans(m_A);
  ublas::blas_3::tsm(m_b, jblas::unit_vec(size), at, ublas::upper_tag());
  /* solve the linear system U x = b */
  ublas::blas_3::tsm(m_b, jblas::unit_vec(size), m_A, ublas::upper_tag());
  return 1;
}

/*
 * This function returns the solution of Ax = b
 *
 * The function employs LU decomposition:
 * If A=L U with L lower and U upper triangular, then the original system
 * amounts to solving
 * L y = b, U x = y
 *
 * The function returns 0 in case of error, 1 if successfull
 */
int LinearSolver::solve_LU(){
  int error = 0;
  jblas::veci ipiv(size);

  /* LU decomposition of A */
  error = lapack::getrf(m_A, ipiv);
  if (error != 0){
     throw(jmath::LapackException(error, 
				 "LinearSolver::solve_LU: error in lapack::getrf() routine",
				 __FILE__,
				 __LINE__));
     return 0;
  }

  /* solve with computed LU */
  jblas::mat_column_major B(size,1);		// getrs accepts two matrices
  column(B,0) = m_b;	// assign b to first column of B (need matrix_proxy.hpp)
  error = lapack::getrs(m_A, ipiv, B);
  if (error != 0){
     throw(jmath::LapackException(error, 
				 "LinearSolver::solve_LU: error in lapack::getrs() routine",
				 __FILE__,
				 __LINE__));
     return 0;
  }

  /* fill x */
  m_x.clear();
  m_x = column(B, 0);
  return 1;
}

/*
 * This function returns the solution of Ax = b
 *
 * The function is based on SVD decomposition:
 * If A=U D V^T with U, V orthogonal and D diagonal, the linear system becomes
 * (U D V^T) x = b or x=V D^{-1} U^T b
 * Note that V D^{-1} U^T is the pseudoinverse A^+
 *
 * The function returns 0 in case of error, 1 if successfull
 */
int LinearSolver::solve_SVD(){
  int error = 0;
  jblas::mat_column_major U(size, size), VT(size, size);
  jblas::vec s(size);
  /* SVD decomposition of A */
  error = lapack::gesdd(m_A,s,U,VT);
  if (error!=0) {
    throw(jmath::LapackException(error,
                                 "LinearSolver::solve_LU: error in lapack::gesdd() routine",
                                 __FILE__,
                                 __LINE__));
  }
  /* fill x */
  m_x.clear();
  for (uint i = 0 ; i < size ; ++i) {
    m_x.plus_assign( (inner_prod(column(U,i),m_b) / s(i)) * row(VT,i) );
  }
  return 1;
}

/*
 * This function returns the solution of Ax = b for a real symmetric matrix A
 *
 * The function is based on Bunch-Kaufman factorization:
 * A is factored as U*D*U^T where U is upper triangular and
 * D symmetric and block diagonal
 *
 * The function returns 0 in case of error, 1 if successfull
 */
int LinearSolver::solve_BK(){
  int error = 0;
  jblas::veci ipiv(size);
  /* factorize A */
  error = lapack::sytrf('U', m_A, ipiv);
  if (error!=0) {
    throw(jmath::LapackException(error,
                                 "LinearSolver::solve_BK: error in lapack::sytrf() routine",
                                 __FILE__,
                                 __LINE__));
  }
  /* solve the system with the computed factorization */
  jblas::mat_column_major B(size,1);		// sytrs accepts two matrices
  column(B,0) = m_b;	// assign b to first column of B (need matrix_proxy.hpp)
  error = lapack::sytrs('U', m_A, B, ipiv);
  if (error!=0) {
    throw(jmath::LapackException(error,
                                 "LinearSolver::solve_BK: error in lapack::sytrs() routine",
                                 __FILE__,
                                 __LINE__));
  }
  
  /* fill x */
  m_x.clear();
  m_x = column(B, 0);
  return 1;
}

// int LinearSolver::invert_Cholesky(mat A, mat& A_inv, const bool& isColMaj){
//   int size = b.size();
//   int error = 0;
//   /* force a major column version of A */
//   if (!isColMaj){
//     A =  ublas::trans(A);
//   }
//   error = lapack::potrf("L", A);
//   if (error != 0){
//      throw(jmath::LapackException(error, 
// 				 "LinearSolver::invert_Cholesky: error in lapack::potrf() routine",
// 				 __FILE__,
// 				 __LINE__));
//      return 0;
//   }
//   error = lapack::potri("L", A);
//   if (error != 0){
//      throw(jmath::LapackException(error, 
// 				 "LinearSolver::invert_Cholesky: error in lapack::potri() routine",
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

#endif
#endif
