/* $Id$ */

#ifndef JMATH_LINEAR_LEAST_SQUARES_HPP
#define JMATH_LINEAR_LEAST_SQUARES_HPP

#include "jafarConfig.h"

#ifdef HAVE_BOOST_SANDBOX
#ifdef HAVE_LAPACK

#include "jmath/jblas.hpp"

namespace jafar {
  namespace jmath {

    /** (Weighted) Linear Least Squares solver. Find \f$x\f$ which minimizes:
     * \f[
     * \left\| A.x - b \right\|^2
     * \f]
     *
     * Each data can also be weighted (usualy by the inverse of its variance).
     *
     * The solution is computed using a Singular Value Decomposition,
     * the lapack::gesdd is used to do the real work.
     * 
     * \ingroup jmath
     */
    class LinearLeastSquares {

    private:

      std::size_t m_sizeModel;
      std::size_t m_sizeDataSet;

      /// design matrix
      jblas::mat_column_major m_A;

      /// rhs vector
      jblas::vec m_b;

      /// least squares solution
      jblas::vec m_x;

      /// least squares solution covariance
      jblas::sym_mat m_xCov;

    public:
      
      LinearLeastSquares() {};

      std::size_t sizeModel() const {return m_sizeModel;}
      std::size_t sizeDataSet() const {return m_sizeDataSet;}

      jblas::mat_column_major const& A() const {return m_A;}
      jblas::mat_column_major& A() {return m_A;}

      jblas::vec const& b() const {return m_b;}
      jblas::vec& b() {return m_b;}

      jblas::vec const& x() const {return m_x;}
      jblas::sym_mat const& xCov() const {return m_xCov;}

			///set size of model and data set and either preserve or not existing data
      void setSize(std::size_t sizeModel, std::size_t sizeDataSet, bool preserve=false);
			///set size of data set and either preserve or not existing data
      void setDataSetSize(std::size_t sizeDataSet, bool preserve=false);

      /** Safe function to add a data point. For more efficiency, one
       * can directly modify the matrix A and the vector b.
       */
      void setData(std::size_t index, jblas::vec const& A_row, double b_val);

      /** Safe function to add a weighted data point. For more
       * efficiency, one can directly modify the matrix A and the vector b.
       */
      void setData(std::size_t index, jblas::vec const& A_row, double b_val, double weight);

      void solve();

    }; // class LinearLeastSquares

  } // namespace jmath
} // namespace jafar


#endif // HAVE_LAPACK
#endif // HAVE_BOOST_SANDBOX

#endif // JMATH_LINEAR_LEAST_SQUARES_HPP
