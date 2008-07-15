/* $Id: linearSolver.hpp  $ */

#ifndef LINEAR_SOLVERS_HPP
#define LINEAR_SOLVERS_HPP

#include "jafarConfig.h"

#ifdef HAVE_BOOST_SANDBOX
#ifdef HAVE_LAPACK

#include "jmath/jblas.hpp"
#include "boost/numeric/ublas/matrix_proxy.hpp"
#include "boost/numeric/ublas/vector_proxy.hpp"

namespace jafar {
  namespace jmath {

    /**
     * Linear system Solvers. Proposes different methodes to solve Ax = b 
     * Makes intensive use of LAPACK, uBLAS and BOOST
     * A is mxm, b is mx1. A, b and x elements are double
     * \ingroup jmath
     * All this functions throw an exception on lapack errors
     */
    namespace LinearSolvers {
      
      /**
       * This function returns the solution of Ax = b
       *
       * The function is based on QR decomposition with explicit computation of Q:
       * If A=Q R with Q orthogonal and R upper triangular, the linear system becomes
       * Q R x = b or R x = Q^T b.
       *
       * The function returns 0 in case of error, 1 if successfull
       */
      int solve_QR(const jblas::mat& A, jblas::vec& b, jblas::vec& x);

      /**
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
      int solve_QR_noQ(const jblas::mat& A, jblas::vec& b, jblas::vec& x);

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
      int solve_Cholesky(const jblas::mat& A, jblas::vec& b, jblas::vec& x);

      /**
       * This function returns the solution of Ax = b
       *
       * The function employs LU decomposition:
       * If A=L U with L lower and U upper triangular, then the original system
       * amounts to solving
       * L y = b, U x = y
       *
       * The function returns 0 in case of error, 1 if successfull
       */
      int solve_LU(const jblas::mat& A, jblas::vec& b, jblas::vec& x);

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
      int solve_SVD(const jblas::mat& A, jblas::vec& b, jblas::vec& x);

      /*
       * This function returns the solution of Ax = b for a real symmetric matrix A
       *
       * The function is based on Bunch-Kaufman factorization:
       * A is factored as U*D*U^T where U is upper triangular and
       * D symmetric and block diagonal
       *
       * The function returns 0 in case of error, 1 if successfull
       */
      int solve_BK(const jblas::mat& A, jblas::vec& b, jblas::vec& x);
    } // namespace LinearSolvers
  } // namespace jmath
} // namespace jafar


#endif // HAVE_LAPACK
#endif // HAVE_BOOST_SANDBOX

#endif // JMATH_LINEAR_LEAST_SQUARES_HPP
