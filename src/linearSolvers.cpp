/* $Id linearSolver.cpp  */
#include "jmath/linearSolvers.hpp"

#include "jafarConfig.h"

#ifdef HAVE_BOOST_SANDBOX
#ifdef HAVE_LAPACK

#include "assert.h"
#include <cmath>
#include <iostream> // gesdd uses std::cerr but does not includes iostream

#include <boost/numeric/bindings/lapack/computational/geqrf.hpp>
#include <boost/numeric/bindings/lapack/driver/gesdd.hpp>
#include <boost/numeric/bindings/lapack/computational/ormqr.hpp>
#include <boost/numeric/bindings/lapack/computational/orgqr.hpp>
#include <boost/numeric/bindings/lapack/computational/trtrs.hpp>
#include <boost/numeric/bindings/lapack/computational/potrf.hpp>
#include <boost/numeric/bindings/lapack/computational/potrs.hpp>
#include <boost/numeric/bindings/lapack/computational/getrf.hpp>
#include <boost/numeric/bindings/lapack/computational/getrs.hpp>
#include <boost/numeric/bindings/lapack/computational/sytrf.hpp>
#include <boost/numeric/bindings/lapack/computational/sytrs.hpp>
#include <boost/numeric/bindings/lapack/driver/gesv.hpp>
#include <boost/numeric/bindings/lapack/driver/posv.hpp>
#include <boost/numeric/bindings/lapack/driver/sysv.hpp>
#include <boost/numeric/bindings/blas/level3/gemm.hpp>
#include <boost/numeric/bindings/ublas/matrix.hpp>
#include <boost/numeric/bindings/ublas/vector.hpp>
#include <boost/numeric/ublas/triangular.hpp>
#include <boost/numeric/bindings/trans.hpp>
#include <boost/numeric/bindings/upper.hpp>

#include "kernel/jafarException.hpp"
#include "jmath/jmathException.hpp"
//#include "jmath/linearLeastSquares.hpp"

namespace jafar {
  namespace jmath {
    namespace LinearSolvers {

      using namespace jafar;
      using namespace jblas;
      using namespace std;
      namespace lapack = boost::numeric::bindings::lapack;
      namespace ublas = boost::numeric::ublas;
			namespace blas = boost::numeric::bindings::blas;
      namespace bindings = boost::numeric::bindings;

      int solve_QR(mat_column_major& A, const mat_column_major& B, mat_column_major& X){
        JFR_PRECOND( ((A.size1() == B.size1()) && 
                     (B.size1() == A.size2()) && 
                     (A.size2() == X.size1())),
                     "LinearSolver: invalid size. A is nxn, B and X are nxq");
        JFR_PRECOND( X.size2() == B.size2(),
                     "LinearSolver: invalid size. X is nxq and B is nxq");
        size_t lhs = A.size1();
        int error = 0;
        ublas::matrix<double, ublas::column_major>R (lhs, lhs);
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
				/* X = Q^t B*/
				blas::gemm(1.0, bindings::trans(A), B, 0.0, X);
				error = lapack::trtrs(bindings::upper(R), X);
        if (error != 0){
          throw(jmath::LapackException(error, 
                                       "solve_QR: error in lapack::trtrs() routine",
                                       __FILE__,
                                       __LINE__));
          return 0;
        }
        return 1;
      }

      int solve_QR_noQ(mat_column_major A, mat B, mat& X){
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
        /* solve R^t y = A^t b */
        X = ublas::prod(ublas::trans(A), B);
        error = lapack::trtrs(bindings::trans(bindings::upper(A)), X);
        if (error != 0){
          throw(jmath::LapackException(error, 
                                       "solve_QR_noQ: error in lapack::trtrs() routine",
                                       __FILE__,
                                       __LINE__));
          return 0;
        }
        /* solve Rx = y (y was saved in A^t b) */
        error = lapack::trtrs(bindings::upper(A), X);
        if (error != 0){
          throw(jmath::LapackException(error, 
                                       "solve_QR_noQ: error in lapack::trtrs() routine",
                                       __FILE__,
                                       __LINE__));
          return 0;
        }
        return 1;
      }
  
      int solve_Cholesky(mat_column_major A, mat_column_major B, mat& X){
        JFR_PRECOND( ((A.size1() == A.size2()) && (A.size1() == B.size1()) && (A.size2() == X.size1())),
                     "LinearSolver: invalid size. A is nxn, X is nxq and B is nxq");
        JFR_PRECOND( X.size2() == B.size2(),
                     "LinearSolver: invalid size. X is nxq and B is nxq");
        int error = 0;
        error = lapack::potrf(bindings::upper(A));
        if (error != 0){
          throw(jmath::LapackException(error, 
                                       "solve_Cholesky: error in lapack::potrf() routine",
                                       __FILE__,
                                       __LINE__));
          return 0;
        }
        error = lapack::potrs(bindings::upper(A), B);
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

      int solve_Cholesky(mat_column_major A, jblas::vec b, jblas::vec& x){
        JFR_PRECOND( ((A.size1() == A.size2()) && 
											(A.size1() == b.size()) && 
											(A.size2() == x.size())),
                     "LinearSolver: invalid size. A is nxn, x is nx1 and b is nx1");
        mat_column_major B(b.size(),1); 
        mat X(x.size(),1);
        column(B,0) = b;
        int result = solve_Cholesky(A, B, X);
        if (result == 1)
          x = column(X,0);
        return result;
      }

      int solve_LU(mat_column_major A, jblas::vec b, jblas::vec& x){
        JFR_PRECOND( ((A.size1() == A.size2()) && 
											(A.size1() == b.size()) && 
											(A.size2() == x.size())),
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
        mat_column_major B(size,1);		// getrs accepts two matrices
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

      int solve_SVD(const mat& A, const jblas::vec& b, jblas::vec& x){
        JFR_PRECOND( A.size1() == A.size2() and A.size1() == b.size(),
                    "LinearSolver: invalid size. A is mxm and b is mx1");
        size_t size = b.size();
        int error = 0;
        mat_column_major m_A(size, size);
        m_A.assign(A);
        mat_column_major U(size, size), VT(size, size);
        jblas::vec s(size);
        /* SVD decomposition of A */
        error = lapack::gesdd('A',bindings::upper(m_A),s,U,VT);
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

      int solve_BK(const mat& A, const jblas::vec& b, jblas::vec& x){
        JFR_PRECOND(((A.size1() == A.size2()) && (A.size2() == b.size())),
                    "LinearSolver: invalid size. A is mxm and b is mx1");
        size_t size = b.size();
        int error = 0;
        jblas::veci ipiv(size);
        /* factorize A */
        mat_column_major m_A(size, size);
        m_A.assign(A);
				ublas::symmetric_adaptor<ublas::matrix<double, ublas::column_major>, ublas::upper> s_A(m_A);

        error = lapack::sytrf(bindings::upper(m_A), ipiv);
        if (error!=0) {
          throw(jmath::LapackException(error,
                                       "solve_BK: error in lapack::sytrf() routine",
                                       __FILE__,
                                       __LINE__));
        }
        /* solve the system with the computed factorization */
        mat_column_major B(size,1);		// sytrs accepts two matrices
        column(B,0) = b;	// assign b to first column of B (need matrix_proxy.hpp)
        error = lapack::sytrs(bindings::upper(m_A), ipiv, B);
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
