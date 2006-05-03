/* $Id$ */

#include "jafarConfig.h"

#ifdef HAVE_BOOST_SANDBOX
#ifdef HAVE_LAPACK

#include <cmath>
#include "jmath/pca.hpp"
#include "jmath/jmathException.hpp"
#include "boost/numeric/bindings/lapack/syev.hpp"
#include "boost/numeric/bindings/traits/ublas_matrix.hpp"
#include "boost/numeric/bindings/traits/ublas_vector.hpp"

namespace lapack = boost::numeric::bindings::lapack;
using namespace jafar::jmath;

void PCAtools::batchPCA(const jblas::mat& X_) {
  int m = X_.size1();
  int n = X_.size2();

  jblas::mat centeredX(X_);
  // compute mean and center X
  mean.resize(m);
  for (int i=0; i<m; i++) {
    mean(i) = 0.0;
    for (int j=0; j<n; j++)
      mean(i) += centeredX(i,j);
	  
    mean(i) /= n;
    for (int j=0; j<n; j++)
      centeredX(i,j) -= mean(i);
  }
  // compute svd
  if (m <= n) {
    ublas::matrix<double,ublas::column_major> A = ublas::prod(centeredX,ublas::trans(centeredX));
    A /= n;
    eigenvalues.resize(m);
    int ierr = lapack::syev('V','U',A,eigenvalues,lapack::optimal_workspace());	  
    eigenvectors = A;
  } else {
    ublas::matrix<double,ublas::column_major> A = ublas::prod(ublas::trans(centeredX),centeredX);
    A /= n;
    eigenvalues.resize(n);
    int ierr = lapack::syev('V','U',A,eigenvalues,lapack::optimal_workspace());
    eigenvectors = ublas::prod(centeredX,A);
    for(int j=0; j<n; j++) {
      jblas::mat_column Uc(eigenvectors,j);
      Uc /= sqrt(n * eigenvalues(j));
    }
  }
  coefficients = ublas::prod(ublas::trans(eigenvectors),centeredX); 
  pca_performed = true;
}

void PCAtools::incrementPCA(const jblas::vec& I_) {
  JFR_PRECOND(pca_performed,
	      "PCAtools::project: no eigenspace available");
  JFR_PRECOND(I_.size() == eigenvectors.size1(),
	      "PCAtools::project: wrong size of the input vector. should be " << eigenvectors.size1());
  jblas::vec a = ublas::prod(ublas::trans(eigenvectors),(I_-mean));
  jblas::vec y = ublas::prod(eigenvectors,a)+mean;
  jblas::vec r = I_ - y;
  double normR = ublas::norm_2(r);
  jblas::mat Up(eigenvectors.size1(),eigenvectors.size2()+1);
  Up.clear();
  ublas::project(Up,ublas::range(0,eigenvectors.size1()),
		 ublas::range(0,eigenvectors.size2())).assign(eigenvectors);
  ublas::column(Up,Up.size2()-1) = r/normR;
  jblas::mat Ap(coefficients.size1()+1,coefficients.size2()+1);
  Ap.clear();
  ublas::project(Up,ublas::range(0,coefficients.size1()),
		 ublas::range(0,coefficients.size2())).assign(coefficients);
  for(int i=0; i < a.size(); i++)
    Ap(i,(Ap.size2()-1)) = a(i);
  for(int i=0; i < (Ap.size2()-1); i++)
    Ap(Ap.size1()-1,i) = 0;
  Ap(Ap.size1()-1,Ap.size2()-1) = normR;
  PCAtools pca(Ap);
  jblas::vec meanpp = pca.getMean();
  jblas::mat Upp = pca.getEigenVectors();
  jblas::vec alphap = pca.getEigenValues();
  ublas::scalar_vector<double> Uno(Up.size2());
  coefficients = ublas::prod(ublas::trans(Upp),
			     (Ap - ublas::outer_prod(meanpp,Uno)));
  eigenvectors = ublas::prod(Up,Upp);
  mean = mean + ublas::prod(Up,meanpp);
  eigenvalues = alphap;
}

jblas::vec PCAtools::project(const jblas::vec& I_, int k_) const {
  JFR_PRECOND(pca_performed,
	      "PCAtools::project: no eigenspace available");
  JFR_PRECOND(I_.size() == eigenvectors.size1(),
	      "PCAtools::project: wrong size of the input vector. should be " << eigenvectors.size1());
  JFR_PRECOND( (0 < k_) && (k_ <= eigenvectors.size2()),
	       "PCAtools::project: dimension of subspace out of range. should be in [1," 
	       << eigenvectors.size2() << "]");
  return ublas::prod(ublas::trans(ublas::project(eigenvectors,ublas::range(0,eigenvectors.size1()),ublas::range(0,k_))),(I_-mean));
}

jblas::vec PCAtools::reconstruct(const jblas::vec& P_) const {
  JFR_PRECOND(pca_performed,
	      "PCAtools::project: no eigenspace available");
  JFR_PRECOND(P_.size() <= eigenvectors.size2(),
	      "PCAtools::project: wrong size of the input vector. should be in [1," << eigenvectors.size2() << "]");
  return (ublas::prod(ublas::project(eigenvectors,ublas::range(0,eigenvectors.size1()),ublas::range(0,P_.size())),P_) + mean);
}


void PCAtools::loadKeyValueFile(jafar::kernel::KeyValueFile const& keyValueFile) {
  JFR_TRACE_BEGIN;
  keyValueFile.getItem("mean", mean);
  keyValueFile.getItem("eigenvalues", eigenvalues);
  keyValueFile.getItem("eigenvectors", eigenvectors);
  keyValueFile.getItem("coefficients", coefficients);
  JFR_TRACE_END("PCAtools::load");
  pca_performed = true;
}


void PCAtools::saveKeyValueFile(jafar::kernel::KeyValueFile & keyValueFile) {
  JFR_PRECOND(pca_performed,"PCAtools::save: no data to save");
  JFR_TRACE_BEGIN;
  keyValueFile.setItem("mean", mean);
  keyValueFile.setItem("eigenvalues", eigenvalues);
  keyValueFile.setItem("eigenvectors", eigenvectors);
  keyValueFile.setItem("coefficients", coefficients);
  JFR_TRACE_END("PCAtools::save");
}

#endif
#endif
