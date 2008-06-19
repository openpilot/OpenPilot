/* $Id: linearSolver.hpp  $ */

#ifndef JMATH_LINEAR_LEAST_SQUARES_HPP
#define JMATH_LINEAR_LEAST_SQUARES_HPP

#include "jafarConfig.h"

#ifdef HAVE_BOOST_SANDBOX
#ifdef HAVE_LAPACK

#include "jmath/jblas.hpp"

namespace jafar {
  namespace jmath {

    /**
     * Linera system Solver. Uses different methodes to solve Ax = b 
     * Makes intensive use of LAPACK, uBLAS and BOOST
     * A is mxm, b is mx1. A, b and x elements are double
     * \ingroup jmath
     */
    class LinearSolver {

    private:

      /// design matrix
      jblas::mat_column_major m_A;

      /// rhs vector
      jblas::vec m_b;

      /// solution
      jblas::vec m_x;

      ///model size should be the same for A, b and x
      std::size_t size;
    public:
      
      LinearSolver(const jblas::mat_column_major& A, const jblas::vec& b);

      jblas::mat_column_major const& A() const {return m_A;}
      jblas::mat_column_major& A() {return m_A;}

      jblas::vec const& b() const {return m_b;}
      jblas::vec& b() {return m_b;}

      jblas::vec const& x() const {return m_x;}

      /** the solving methodes
       */  
      int solve_QR();
      int solve_QR_noQ();
      int solve_Cholesky();
      int solve_LU();
      int solve_SVD();
      int solve_BK();

    }; // class LinearSolver

  } // namespace jmath
} // namespace jafar


#endif // HAVE_LAPACK
#endif // HAVE_BOOST_SANDBOX

#endif // JMATH_LINEAR_LEAST_SQUARES_HPP
