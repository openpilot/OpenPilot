/* $Id$ */

#ifndef _JMATH_VARIABLESIZELINEARLEASTSQUARES_HPP_
#define _JMATH_VARIABLESIZELINEARLEASTSQUARES_HPP_

#include "jafarConfig.h"

#ifdef HAVE_BOOST_SANDBOX
#ifdef HAVE_LAPACK

#include "jmath/jblas.hpp"

namespace jafar {
  namespace jmath {
    /** Linear Least Squares solver. Find \f$x\f$ which minimizes:
     * \f[
     * \left\| A.x + b \right\|^2
     * \f]
     * 
     * The difference with \ref LinearLeastSquares is that the size of
     * the data sets is not staticly set, which means you can reduce the
     * size of the data sets, or add new points and do a new computation
     * without reinitializing the solver.
     */
    class VariableSizeLinearLeastSquares {
      public:
        explicit VariableSizeLinearLeastSquares(int _modelSize);
        const jblas::vec& x() const {return m_x;}
        void solve();
        void addMeasure( const jblas::vec& dataPoint, double b_val);
        jblas::sym_mat A() { return m_A; }
        double computeMinimum() const;
        int countMeasures() const { return m_countValues; }
        /**
         * Merge the values of the \ref VariableSizeLinearLeastSquares
         * given in argument.
         * Only two \ref VariableSizeLinearLeastSquares of the same
         * model size can be merged.
         */
        void merge( const VariableSizeLinearLeastSquares& );
      private:
        /// Size of the model
        int m_modelSize;
        
        /// design matrix
        jblas::sym_mat m_A;
//         jblas::sym_mat_column_major m_A;
  
        /// rhs vector
        ublas::matrix<double, ublas::column_major> m_b;
  
        /// least squares solution
        jblas::vec m_x;
        
        double m_b_val;
        int m_countValues;
    };
  }
}

#endif
#endif

#endif
