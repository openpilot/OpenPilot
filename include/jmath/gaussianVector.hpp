/* $Id$ */

#ifndef JMATH_GAUSSIAN_VECTOR_HPP
#define JMATH_GAUSSIAN_VECTOR_HPP

#include <ostream>

#include "boost/shared_ptr.hpp"

#include "kernel/jafarDebug.hpp"

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

      GaussianVector() {}
      GaussianVector(std::size_t size_);
      GaussianVector(const jblas::vec& x_, const jblas::sym_mat& P_);
      GaussianVector(const GaussianVector& v_);

      inline std::size_t size() const {return x.size();};

      void resize(std::size_t s) {
	if (size() != s) {
	  x.resize(s);
	  P.resize(s,s);
	}
      }

      /// clear x and P
      void clear() {
	x.clear();
	P.clear();
      }

      double probabilityDensity(const jblas::vec& v) const;

#ifndef SWIG
      friend std::ostream& operator <<(std::ostream& s, const GaussianVector& v_);
#endif
    };

#ifndef SWIG
    std::ostream& operator <<(std::ostream& s, const GaussianVector& v_);
#endif

    /** A weighted gaussian vector, add a weight to a GaussianVector.
     *
     * \ingroup jmath
     */
    class WeightedGaussianVector : public GaussianVector {

    public:

      /// weight
      double w;

      WeightedGaussianVector() : GaussianVector(), w(1.0) {}
      WeightedGaussianVector(std::size_t size_);
      WeightedGaussianVector(const jblas::vec& x_, const jblas::sym_mat& P_, double w_=1.0);
      WeightedGaussianVector(const GaussianVector& gv_, double w_=1.0);
      WeightedGaussianVector(const WeightedGaussianVector& v_);


#ifndef SWIG
      friend std::ostream& operator <<(std::ostream& s, const WeightedGaussianVector& v_);
#endif
     };


#ifndef SWIG
    std::ostream& operator <<(std::ostream& s, const WeightedGaussianVector& v_);
#endif

 } // namespace jmath
} // namespace jafar

#endif // JMATH_GAUSSIAN_VECTOR_HPP
