/* $Id$ */

#ifndef JMATH_PCA_HPP
#define JMATH_PCA_HPP

#include "jafarConfig.h"

#ifdef HAVE_BOOST_SANDBOX
#ifdef HAVE_LAPACK

#include "jmath/jblas.hpp"
#include "kernel/jafarDebug.hpp"

namespace jafar {
  namespace jmath {

    /*! Principal Component analysis (PCA) Tools Class
     *
     * \ingroup jmath
     */
    class PCAtools {
    public:

      typedef enum PCA_Method {
	batch,
	EM
      };

      /// Default Constructor 
      PCAtools() : pca_performed(false) {};

      /*! Constructor with direct computation
       * @param X_ input matrix
       */
      PCAtools(const jblas::mat& X_, PCA_Method m_ = batch) {
	switch (m_) {
	case batch:
	  batchPCA(X_);
	  break;
	default:
	  JFR_RUN_TIME("jmath::PCA: forbidden case in constructor");
	}
      };

      //Accessors
      /// Mean accessor
      jblas::vec& getMean() {
	JFR_PRECOND(pca_performed, "PCA::getMean: no results available");
	return mean;
      };

      /// Eigen Vectors accessor
      jblas::mat& getEigenVectors() {
	JFR_PRECOND(pca_performed, "PCA::getEigenVectors: no results available");
	return eigenvectors;
      };

      /// Eigen Values accessor
      jblas::vec& getEigenValues() {
	JFR_PRECOND(pca_performed, "PCA::getEigenValues: no results available");
	return eigenvalues;
      }


      /*! Compute PCA using the batch algorithm
       * @param X_ input matrix 
       */
      void batchPCA(const jblas::mat& X_);

      /*! Increment the PCA basis with a new vector
       * @param I_ input vector 
       */
      void incrementPCA(const jblas::vec& I_);

      /*! Project an Input vector on the eigenspace.
       * @param I_ input vector
       * @param k_ eigenspace dimension
       * @return the image vector
       */
      jblas::vec project(const jblas::vec& I_, int k_) const;

      /*! Reconstruct full vector from its projection
       * @param P_ projection vector
       * @return reconstructed vector
       */
      jblas::vec reconstruct(const jblas::vec& I_) const;

      
    private:
      bool pca_performed;
      jblas::mat eigenvectors,coefficients;
      jblas::vec mean, eigenvalues;

    }; // class PCA
  } // namespace jmath
} // namespace jafar

#endif // HAVE_LAPACK
#endif // HAVE_BOOST_SANDBOX
#endif // JMATH_PCA_HPP
