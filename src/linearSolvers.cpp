/* $Id linearSolver.cpp  */
#include "jmath/linearSolvers.hpp"

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

namespace jafar {
  namespace jmath {
    namespace LinearSolvers {

      using namespace jafar;
      using namespace jblas;
      using namespace std;
      namespace lapack = boost::numeric::bindings::lapack;
      namespace ublas = boost::numeric::ublas;

      int solve_QR(jblas::mat_column_major& A, const jblas::mat& B, jblas::mat& X){
        JFR_PRECOND( ((A.size1() == B.size1()) && 
                     (B.size1() == A.size2()) && 
                     (A.size2() == X.size1())),
                     "LinearSolver: invalid size. A is nxn, B and X are nxq");
        JFR_PRECOND( X.size2() == B.size2(),
                     "LinearSolver: invalid size. X is nxq and B is nxq");
        size_t lhs = A.size1();
        int error = 0;
        ublas::triangular_matrix<double, ublas::upper>R (lhs, lhs);
        jblas::vec tau(lhs);
        /*QR decomposition of A*/
        error = lapack::geqrf(A, tau);
        if (error != 0){
          throw(jmath::LapackException(error, 
                                       "solve_QR: error in lapack::geqrf() routine",
                                       __FILE__,
                                       __LINE__));
          return 0;
        }
        /*store R (R is returned in the upper triangular of A)*/
        for(unsigned int i=0; i < lhs; ++i) {
          for (unsigned int j=i; j < lhs; ++j){
            R(i,j) = A(i,j);
          }
        }

        /*compute Q (Q is returned in A)*/
        error = lapack::orgqr(A, tau);
        if (error != 0){
          throw(jmath::LapackException(error, 
                                       "solve_QR: error in lapack::orgqr() routine",
                                       __FILE__,
                                       __LINE__));
          return 0;
        }
        /* solve R X = Q^t B */
        X = ublas::prod(ublas::trans(A), B);
        ublas::blas_3::tsm(X, 1.0, ublas::trans(R), ublas::upper_tag());
        return 1;
      }

      int solve_QR_noQ(jblas::mat_column_major A, jblas::mat B, jblas::mat& X){
          JFR_PRECOND( ((A.size1() == B.size1()) && 
                        (B.size1() == A.size2()) && 
                        (A.size2() == X.size1())),
                     "LinearSolver: invalid size. A is nxn, X and B are nxq");
        JFR_PRECOND( X.size2() == B.size2(),
                     "LinearSolver: invalid size. X is nxq and B is nxq");
        size_t lhs = A.size1();
        int error = 0;
        ublas::triangular_matrix<double, ublas::upper>R (lhs, lhs);
        jblas::vec tau(lhs);

        /* QR decomposition of A */
        error = lapack::geqrf(A, tau);
        if (error != 0){
          throw(jmath::LapackException(error, 
                                       "solve_QR_noQ: error in lapack::geqrf() routine",
                                       __FILE__,
                                       __LINE__));
          return 0;
        }
        /* store R (R is returned in the upper triangular of A) */
        for(unsigned int i=0; i < lhs; ++i)
          for (unsigned int j=i; j < lhs;++j)
            R(i,j) = A(i,j);

        /* solve R^t y = A^t b */
        X = ublas::prod(ublas::trans(A), B);
        X = ublas::blas_3::tsm(X, 1.0, ublas::trans(R), ublas::upper_tag());
        /* solve Rx = y (y was saved in A^t b) */
        X = ublas::blas_3::tsm(X, 1.0, R, ublas::upper_tag());
        return 1;
      }
  
      int solve_Cholesky(jblas::mat_column_major A, jblas::mat_column_major B, jblas::mat& X){
        JFR_PRECOND( ((A.size1() == A.size2()) && (A.size1() == B.size1()) && (A.size2() == X.size1())),
                     "LinearSolver: invalid size. A is nxn, X is nxq and B is nxq");
        JFR_PRECOND( X.size2() == B.size2(),
                     "LinearSolver: invalid size. X is nxq and B is nxq");
        int error = 0;
        error = lapack::potrf('U', A);
        if (error != 0){
          throw(jmath::LapackException(error, 
                                       "solve_Cholesky: error in lapack::potrf() routine",
                                       __FILE__,
                                       __LINE__));
          return 0;
        }
        error = lapack::potrs('U', A, B);
        if (error != 0){
          throw(jmath::LapackException(error, 
                                       "solve_Cholesky: error in lapack::potrf() routine",
                                       __FILE__,
                                       __LINE__));
          return 0;
        }
        X.assign(B);
        return 1;
      }

      int solve_Cholesky(jblas::mat_column_major A, jblas::vec b, jblas::vec& x){
        JFR_PRECOND( ((A.size1() == A.size2()) && (A.size1() == b.size()) && (A.size2() == x.size())),
                     "LinearSolver: invalid size. A is nxn, x is nx1 and b is nx1");
        jblas::mat_column_major B(b.size(),1); 
        jblas::mat X(x.size(),1);
        column(B,0) = b;
        int result = solve_Cholesky(A, B, X);
        if (result == 1)
          x = column(X,0);
        return result;
      }

      int solve_LU(jblas::mat_column_major A, jblas::vec b, jblas::vec& x){
        JFR_PRECOND( ((A.size1() == A.size2()) && (A.size1() == b.size()) && (A.size2() == x.size())),
                    "LinearSolver: invalid size. A is "<<A.size1()<<"x"<<A.size2() << " and b is "<<b.size()<<"x1");
        size_t size = b.size();
        int error = 0;
        jblas::veci ipiv(size);
        /* LU decomposition of A */
        error = lapack::getrf(A, ipiv);
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
        error = lapack::getrs(A, ipiv, B);
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

      int solve_SVD(const jblas::mat& A, const jblas::vec& b, jblas::vec& x){
        JFR_PRECOND( A.size1() == A.size2() and A.size1() == b.size(),
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
        for (unsigned int i = 0 ; i < size ; ++i) {
          x.plus_assign( (inner_prod(column(U,i), b) / s(i)) * row(VT,i) );
        }
        return 1;
      }

      int solve_BK(const jblas::mat& A, const jblas::vec& b, jblas::vec& x){
        JFR_PRECOND(((A.size1() == A.size2()) && (A.size2() == b.size())),
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
  }
}
#endif
#endif
