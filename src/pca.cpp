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
using namespace std;
using namespace jafar::jmath;

void PCA::batchPCA(const jblas::mat& X_, int dim_) {
  int m = X_.size1();
  int n = X_.size2();

  JFR_PRECOND((0 <= dim_) && (dim_ <= min(m,n)),
	      "PCA::batchPCA: wrong dimension input. must be in [1," << min(m,n) << "]");

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
    jblas::vec alpha(m);
    int ierr = lapack::syev('V','U',A,alpha,lapack::optimal_workspace());	  
    if (ierr==0)
      JFR_RUN_TIME("PCA::batchPCA: error in lapack::syev() function, ierr=" << ierr);
    eigenvectors.resize(m,m);
    for(int i=0;i<m;i++) {
      eigenvalues(i) = alpha(m-i-1);
      ublas::column(eigenvectors,i) = ublas::column(A,m-i-1);
    }
  } else {
    ublas::matrix<double,ublas::column_major> A = ublas::prod(ublas::trans(centeredX),centeredX);
    A /= n;
    jblas::vec alpha(n);
    int ierr = lapack::syev('V','U',A,alpha,lapack::optimal_workspace());
    if (ierr==0)
      JFR_RUN_TIME("PCA::batchPCA: error in lapack::syev() function, ierr=" << ierr);
    eigenvectors.resize(m,n);
    for(int i=0;i<n;i++) {
      eigenvalues(i) = alpha(n-i-1);
      ublas::column(eigenvectors,i) = ublas::column(A,n-i-1);
    }
    eigenvectors = ublas::prod(centeredX,eigenvectors);
    for(int j=0; j<n; j++) {
      jblas::mat_column Uc(eigenvectors,j);
      Uc /= sqrt(n * eigenvalues(j));
    }
  }
  // reduce the subspace if needed
  if ((0 < dim_) && (dim_ < min(m,n))) {
    eigenvalues = ublas::project(eigenvalues,ublas::range(min(m,n)-dim_,min(m,n)));
    eigenvectors = ublas::project(eigenvectors,ublas::range(0,m),
				  ublas::range(min(m,n)-dim_,min(m,n)));
  }
  if (!basis_only)  
    coefficients = ublas::prod(ublas::trans(eigenvectors),centeredX); 
}


void PCA::updatePCA(const jblas::vec& I_, UFlag f_, double thd_) {
  JFR_PRECOND(mean.size() != 0,
	      "PCA::project: no eigenspace available");
  JFR_PRECOND(I_.size() == eigenvectors.size1(),
	      "PCA::project: wrong size of the input vector. should be " << eigenvectors.size1());
  int n = eigenvectors.size2();
  jblas::vec meanp = (n*mean+I_)/static_cast<double>(n+1);
  jblas::vec a = ublas::prod(ublas::trans(eigenvectors),(I_-mean));
  jblas::vec y = ublas::prod(eigenvectors,a)+mean;
  jblas::vec h = y-I_;
  double normh = ublas::norm_2(h);
  if (normh > 0) 
    h /= normh;
  else
    h.clear();
  double gamma = ublas::inner_prod(ublas::trans(h),(I_-mean));
  ublas::matrix<double,ublas::column_major> D(a.size()+1,a.size()+1);
  D.clear();
  ublas::project(D,ublas::range(0,a.size()),
		 ublas::range(0,a.size())) = ublas::outer_prod(a,a);
  D /= static_cast<double>(n)/static_cast<double>((n+1)*(n+1));
  for(std::size_t i=0; i < a.size(); i++) {
    D(i,i) += static_cast<double>(n)/static_cast<double>(n+1)*eigenvalues(i);
    D(D.size1()-1,i) = static_cast<double>(n)/static_cast<double>((n+1)*(n+1))*gamma*a(i);
    D(i,D.size2()-1) = static_cast<double>(n)/static_cast<double>((n+1)*(n+1))*gamma*a(i);
    D(D.size1()-1,D.size2()-1) = static_cast<double>(n)/static_cast<double>((n+1)*(n+1))*gamma*gamma;
  }
  jblas::vec alphap(D.size1());
  int ierr = lapack::syev('V','U',D,alphap,lapack::optimal_workspace());	  
  if (ierr==0)
    JFR_RUN_TIME("PCA::updatePCA: error in lapack::syev() function, ierr=" << ierr);
  jblas::mat R(D.size1(),D.size2());
  eigenvalues.resize(eigenvalues.size()+1);
  for(std::size_t i=0;i<eigenvalues.size();i++) {
    eigenvalues(i) = alphap(eigenvalues.size()-i-1);
    ublas::column(R,i) = ublas::column(D,D.size2()-i-1);
  }
  jblas::mat Up(eigenvectors.size1(),eigenvectors.size2()+1);
  Up.clear();
  ublas::project(Up,ublas::range(0,eigenvectors.size1()),
		 ublas::range(0,eigenvectors.size2())).assign(eigenvectors);
  ublas::column(Up,Up.size2()-1) = h;
  eigenvectors = ublas::prod(Up,R);
  if (!basis_only) {
    jblas::vec etha = ublas::prod(ublas::trans(Up),(mean-meanp));
    coefficients.resize(coefficients.size1()+1,coefficients.size2()+1);
    for(std::size_t i=0; i<coefficients.size2()-1; i++) {
      coefficients(coefficients.size1()-1,i) = 0;
      ublas::column(coefficients,i) = ublas::prod(ublas::trans(R),
						  ublas::column(coefficients,i))+etha;
    }
    a.resize(a.size()+1);
    a(a.size()-1) = 0;
    ublas::column(coefficients,coefficients.size2()-1) = ublas::prod(ublas::trans(R),a)+etha;
  }
  mean = meanp;
  switch (f_) {   
  case ore: 
  case increase:
    if (eigenvectors.size1() >= eigenvectors.size2())
      break;
  case preserve:
    if (!basis_only)
      coefficients = ublas::project(coefficients,
				    ublas::range(0,coefficients.size1()-1),
				    ublas::range(0,coefficients.size2()));
    eigenvectors = ublas::project(eigenvectors,
				  ublas::range(0,eigenvectors.size1()),
				  ublas::range(0,eigenvectors.size2()-1));
    eigenvalues.resize(eigenvalues.size()-1);
    break;
  default:
    JFR_RUN_TIME("PCA::UpdatePCA: unknown UFlag");
  }
}

jblas::vec PCA::project(const jblas::vec& I_) const {
  JFR_PRECOND(mean.size() != 0,
	      "PCA::project: no eigenspace available");
  JFR_PRECOND(I_.size() == eigenvectors.size1(),
	      "PCA::project: wrong size of the input vector. should be " << eigenvectors.size1());
  return ublas::prod(ublas::trans(eigenvectors),(I_-mean));
}

jblas::vec PCA::reconstruct(const jblas::vec& P_) const {
  JFR_PRECOND(mean.size() != 0,
	      "PCA::project: no eigenspace available");
  JFR_PRECOND(P_.size() <= eigenvectors.size2(),
	      "PCA::project: wrong size of the input vector. should be in [1," << eigenvectors.size2() << "]");
  return (ublas::prod(eigenvectors,P_) + mean);
}


void PCA::loadKeyValueFile(jafar::kernel::KeyValueFile const& keyValueFile) {
  JFR_TRACE_BEGIN;
  keyValueFile.getItem("mean", mean);
  keyValueFile.getItem("eigenvalues", eigenvalues);
  keyValueFile.getItem("eigenvectors", eigenvectors);
  keyValueFile.getItem("coefficients", coefficients);
  JFR_TRACE_END("PCA::load");
}


void PCA::saveKeyValueFile(jafar::kernel::KeyValueFile & keyValueFile) {
  JFR_PRECOND(mean.size() != 0,"PCA::save: no data to save");
  JFR_TRACE_BEGIN;
  keyValueFile.setItem("mean", mean);
  keyValueFile.setItem("eigenvalues", eigenvalues);
  keyValueFile.setItem("eigenvectors", eigenvectors);
  keyValueFile.setItem("coefficients", coefficients);
  JFR_TRACE_END("PCA::save");
}

#endif
#endif
