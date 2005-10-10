/* $Id$ */

#include <cmath>

#include "kernel/jafarException.hpp"

#include "jmath/gaussianVector.hpp"
#include "jmath/ublasExtra.hpp"

using namespace jblas;
using namespace ublas;

using namespace jafar::jmath;

/*
 * class GaussianVector
 */

GaussianVector::GaussianVector(std::size_t size_) :
  x(size_), P(size_) 
{
  x.clear();
  P.clear();
}

GaussianVector::GaussianVector(const vec& x_, const sym_mat& P_) :
  x(x_), P(P_) 
{
  JFR_INVARIANT(x.size() == P.size1(),
                "GaussianVector::GaussianVector: size of x and its covariance must be the same");
}

GaussianVector::GaussianVector(const GaussianVector& v_) :
  x(v_.x), P(v_.P) {}

double GaussianVector::probabilityDensity(const jblas::vec& v) 
{
  JFR_PRECOND(v.size() == size(),
              "GaussianVector::value: size of v must match size of gaussian");

  sym_mat P_inv(size(), size());
  MatrixTools::inv(P, P_inv);
//   mat y(size(),1);
//   mat_column y_col(y, 0);
//   y_col.assign(v-x);
//   double num = exp(-0.5 * prod<mat>(trans(y), prod<mat>(P_inv, y))(0,0));

  vec y = v-x;
  double num = exp(-0.5 * inner_prod(y, prod(P_inv, y)));

  double den = pow(2*M_PI,size()/2)*sqrt(MatrixTools::det(P));
  return num / den;
}

std::ostream& jafar::jmath::operator <<(std::ostream& s, const GaussianVector& v_) {
  s << v_.x << " - " << v_.P;
  return s;
}

/*
 * class WeightedGaussianVector
 */

WeightedGaussianVector::WeightedGaussianVector(std::size_t size_) :
  GaussianVector(size_), w(1) {}

WeightedGaussianVector::WeightedGaussianVector(const vec& x_, const sym_mat& P_, double w_) :
  GaussianVector(x_, P_),
  w(w_) {}

WeightedGaussianVector::WeightedGaussianVector(const WeightedGaussianVector& v_) :
  GaussianVector(v_), w(v_.w) {}

std::ostream& jafar::jmath::operator <<(std::ostream& s, const WeightedGaussianVector& v_) {
  s << v_.w << " - " << v_.x << " - " << v_.P;
  return s;
}
