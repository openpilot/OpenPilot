/* $Id$ */

#ifndef FILTER_GAUSSIAN_VECTOR_HPP
#define FILTER_GAUSSIAN_VECTOR_HPP

#include <ostream>

#include "boost/shared_ptr.hpp"

#include "jmath/jblas.hpp"

namespace jafar {
  namespace jmath {

    /** A simple gaussian vector represented by its mean and
     * covariance.
     *
     * \ingroup jmath
     */
    class GaussianVector {

    public:

      /// mean
      jblas::vec x;
      /// covariance
      jblas::sym_mat P;

      GaussianVector(std::size_t size_);
      GaussianVector(const jblas::vec& x_, const jblas::sym_mat& P_);
      GaussianVector(const GaussianVector& v_);

      inline std::size_t size() const {return x.size();};

      double probabilityDensity(const jblas::vec& v);

      friend std::ostream& operator <<(std::ostream& s, const GaussianVector& v_);
    };

    std::ostream& operator <<(std::ostream& s, const GaussianVector& v_);

    typedef boost::shared_ptr<GaussianVector> GaussianVector_ptr;

    /** A weighted gaussian vector, add a weight to a GaussianVector.
     *
     * \ingroup jmath
     */
    class WeightedGaussianVector : public GaussianVector {

    public:

      /// weight
      double w;

      WeightedGaussianVector(std::size_t size_);
      WeightedGaussianVector(const jblas::vec& x_, const jblas::sym_mat& P_, double w_=1.0);
      WeightedGaussianVector(const WeightedGaussianVector& v_);

      friend std::ostream& operator <<(std::ostream& s, const WeightedGaussianVector& v_);
     };

    std::ostream& operator <<(std::ostream& s, const WeightedGaussianVector& v_);
    typedef boost::shared_ptr<WeightedGaussianVector> WeightedGaussianVector_ptr;

 } // namespace jmath
} // namespace jafar

#endif // FILTER_GAUSSIAN_VECTOR_HPP
