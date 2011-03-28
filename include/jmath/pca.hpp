/* $Id$ */

#ifndef JMATH_PCA_T_HPP
#define JMATH_PCA_T_HPP

#include "jafarConfig.h"

#ifdef HAVE_BOOST_SANDBOX
#ifdef HAVE_LAPACK

#include "jmath/jblas.hpp"
#include "kernel/jafarDebug.hpp"

namespace jafar {
  namespace jmath {

    /*! Principal Component analysis (PCA) Tools Class.\n
     *  Principal components are extracted by singular values decomposition on the covariance matrix
     *  of the centered input data. Available data after pca computation are the mean of the input data,
     *  the eigenvalues (in descending order) and corresponding eigenvectors.\n
     *  Other methods allow projection in the eigenspace, reconstruction from eigenspace and 
     *  update of the eigenspace with a new data (according Matej Artec, Matjaz Jogan and Ales Leonardis
     *  : "Incremental PCA for On-line Visual Learning and Recognition").
     *
     *  \ingroup jmath
     */
    template<typename NUMTYPE>
    class PCA_T {
      public:

      /*! Updating method flag
       */
      enum UFlag {
	/*! keep the new basis vector if possible */
        increase, 
	/*! preserve subspace dimension */
        preserve, 
	/*! preserve subspace dimension if overall reconstruction error 
          is under a specified threshold */
        ore 
      };


      /*! Default Constructor
       * @param basisOnly_ flag to compute only the PCA basis
       */
      PCA_T(bool basisOnly_=false) : basis_only(basisOnly_) {};

      /*! Constructor with direct computation
       * @param X_ input m*n matrix (ie n vectors of R(m))
       * @param dim_ subspace dimension in [1,min(m,n)] (default: min(m;n))
       * @param basisOnly_ flag to compute only the PCA basis
       */
      PCA_T(const ublas::matrix<NUMTYPE>& X_, int dim_=0, bool basisOnly_ = false) {
        basis_only = basisOnly_;
        batchPCA(X_,dim_);
      };

      /*! Copy Constructor
       * @param pca_ PCA object
       */
      PCA_T(PCA_T const & pca_) {
        mean = pca_.mean;
        eigenvalues = pca_.eigenvalues;
        eigenvectors = pca_.eigenvectors;
        coefficients = pca_.coefficients;
      }

      /*! assignment operator
       * @param pca_ PCA object
       */
      PCA_T& operator= (PCA_T const & pca_) {
        mean = pca_.mean;
        eigenvalues = pca_.eigenvalues;
        eigenvectors = pca_.eigenvectors;
        coefficients = pca_.coefficients;
        return *this;
      };

      //Accessors
      /// Mean accessor
      ublas::vector<NUMTYPE>& getMean() {
        JFR_PRECOND(mean.size() != 0, "PCA::getMean: no results available");
        return mean;
      };

      /// Eigen Vectors accessor
      ublas::matrix<NUMTYPE>& getEigenVectors() {
        JFR_PRECOND(eigenvectors.size2() != 0, "PCA::getEigenVectors: no results available");
        return eigenvectors;
      };

      /// Eigen Values accessor
      ublas::vector<NUMTYPE>& getEigenValues() {
        JFR_PRECOND(eigenvalues.size() != 0, "PCA::getEigenValues: no results available");
        return eigenvalues;
      }

      /// Coefficients accessor
      ublas::matrix<NUMTYPE>& getCoefficients() {
        JFR_PRECOND(coefficients.size2() != 0, "PCA::getEigenValues: no results available");
        return coefficients;
      };


      /*! Compute PCA using the batch algorithm
       * @param X_ input m*n matrix (ie n vectors of R(m))
       * @param dim_ subspace dimension in [1,min(m,n)] (default: min(m;n))
       */
      void batchPCA(const ublas::matrix<NUMTYPE>& X_, int dim_=0);


      /*! update the PCA with a new vector
       * @param I_ input vector 
       * @param f_ update flag 
       * @param thd_ threshold used if UFlag = ore 
       */
      void updatePCA(const ublas::vector<NUMTYPE>& I_, UFlag f_=preserve, NUMTYPE thd_=0.25);

      /*! Project an Input vector on the eigenspace.
       * @param I_ input vector
       * @return the image vector
       */
      ublas::vector<NUMTYPE> project(const ublas::vector<NUMTYPE>& I_) const;

      /*! Reconstruct full vector from its projection
       * @param P_ projection vector
       * @return reconstructed vector
       */
      ublas::vector<NUMTYPE> reconstruct(const ublas::vector<NUMTYPE>& I_) const;

      private:

#ifdef USE_JMATH_SERIALIZATION
      friend class boost::serialization::access;
      template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
        ar & BOOST_SERIALIZATION_NVP(basis_only);
        ar & BOOST_SERIALIZATION_NVP(eigenvectors);
        ar & BOOST_SERIALIZATION_NVP(coefficients);
        ar & BOOST_SERIALIZATION_NVP(mean);
        ar & BOOST_SERIALIZATION_NVP(eigenvalues);
      };
#endif
      bool basis_only;
      ublas::matrix<NUMTYPE> eigenvectors,coefficients;
      ublas::vector<NUMTYPE> mean, eigenvalues;

    }; // class PCA_T
  } // namespace jmath
} // namespace jafar

#include <jmath/pca.hxx>

namespace jafar{
  namespace jmath {
    typedef PCA_T<double> PCA;
  }
}
#endif // HAVE_LAPACK
#endif // HAVE_BOOST_SANDBOX
#endif // JMATH_PCA_T_HPP

