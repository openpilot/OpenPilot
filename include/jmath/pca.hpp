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
		class PCA {
		public:

			/*! Updating method flag
			*/
			typedef enum UFlag {
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
					PCA(bool basisOnly_=false) : basis_only(basisOnly_) {};

			/*! Constructor with direct computation
				* @param X_ input m*n matrix (ie n vectors of R(m))
					* @param dim_ subspace dimension in [1,min(m,n)] (default: min(m;n))
					* @param basisOnly_ flag to compute only the PCA basis
			*/
				PCA(const jblas::mat& X_, int dim_=0, bool basisOnly_ = false) {
					basis_only = basisOnly_;
					batchPCA(X_,dim_);
				};

			/*! Copy Constructor
				* @param pca_ PCA object
			*/
				PCA(PCA const & pca_) {
					mean = pca_.mean;
					eigenvalues = pca_.eigenvalues;
					eigenvectors = pca_.eigenvectors;
					coefficients = pca_.coefficients;
				}

			/*! assignment operator
				* @param pca_ PCA object
			*/
				PCA& operator= (PCA const & pca_) {
					mean = pca_.mean;
					eigenvalues = pca_.eigenvalues;
					eigenvectors = pca_.eigenvectors;
					coefficients = pca_.coefficients;
					return *this;
				};

			//Accessors
			/// Mean accessor
				jblas::vec& getMean() {
					JFR_PRECOND(mean.size() != 0, "PCA::getMean: no results available");
					return mean;
				};

			/// Eigen Vectors accessor
				jblas::mat& getEigenVectors() {
					JFR_PRECOND(eigenvectors.size2() != 0, "PCA::getEigenVectors: no results available");
					return eigenvectors;
				};

			/// Eigen Values accessor
				jblas::vec& getEigenValues() {
					JFR_PRECOND(eigenvalues.size() != 0, "PCA::getEigenValues: no results available");
					return eigenvalues;
				}

			/// Coefficients accessor
				jblas::mat& getCoefficients() {
					JFR_PRECOND(coefficients.size2() != 0, "PCA::getEigenValues: no results available");
					return coefficients;
				};


			/*! Compute PCA using the batch algorithm
				* @param X_ input m*n matrix (ie n vectors of R(m))
					* @param dim_ subspace dimension in [1,min(m,n)] (default: min(m;n))
			*/
					void batchPCA(const jblas::mat& X_, int dim_=0);


			/*! update the PCA with a new vector
				* @param I_ input vector 
					* @param f_ update flag 
					* @param thd_ threshold used if UFlag = ore 
			*/
					void updatePCA(const jblas::vec& I_, UFlag f_=preserve, double thd_=0.25);

			/*! Project an Input vector on the eigenspace.
				* @param I_ input vector
					* @return the image vector
			*/
					jblas::vec project(const jblas::vec& I_) const;

			/*! Reconstruct full vector from its projection
				* @param P_ projection vector
					* @return reconstructed vector
			*/
					jblas::vec reconstruct(const jblas::vec& I_) const;

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
				jblas::mat eigenvectors,coefficients;
				jblas::vec mean, eigenvalues;

			}; // class PCA
		} // namespace jmath
	} // namespace jafar

#endif // HAVE_LAPACK
#endif // HAVE_BOOST_SANDBOX
#endif // JMATH_PCA_HPP
