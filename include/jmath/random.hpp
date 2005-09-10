/* $Id$ */

#ifndef JMATH_RANDOM_HPP
#define JMATH_RANDOM_HPP

#include <vector>

#include "jmath/jblas.hpp"

#include "boost/random.hpp"

namespace jafar {
  namespace jmath {

    /** Uniform distribution generator using the Boost random
     * library. Have a look at demoRandom.tcl.
     *
     * \ingroup jmath
     */
    class UniformDistribution {
    public:

      UniformDistribution(double min_=0.0, double max_=1.0, long long unsigned int seed_ = 1);
      ~UniformDistribution();

      double get();

    private:

      boost::rand48 rng;
      boost::uniform_real<> uniform;

      boost::variate_generator<boost::rand48, boost::uniform_real<> > vg;
    };

    /** Multi dimensionnal uniform distribution generator using the
     * Boost random library. Have a look at demoRandom.tcl.
     *
     * \ingroup jmath
     */
    class MultiDimUniformDistribution {

    public:

      MultiDimUniformDistribution(std::size_t dim_, long long unsigned int seed_ = 1);
      MultiDimUniformDistribution(const jblas::vec& min_, const jblas::vec& max_, long long unsigned int seed_ = 1);
      ~MultiDimUniformDistribution();

      jblas::vec& get();

    private:

      jblas::vec u;

      std::vector<UniformDistribution*> uniformsVec;
      
    };


    /** Normal distribution generator using the Boost random
     * library. Have a look at demoRandom.tcl.
     *
     * \ingroup jmath
     */
    class NormalDistribution {
    public:

      NormalDistribution(double mean_ = 0, double sigma_ = 1, long long unsigned int seed_ = 1);

      ~NormalDistribution();

      double get();

    private:
      
      boost::rand48 rng;
      boost::normal_distribution<> normal;

      boost::variate_generator<boost::rand48, boost::normal_distribution<> > vg;

    };

    /** Multi dimensionnal normal distribution generator using the
     * Boost random library. Have a look at demoRandom.tcl.
     *
     * \ingroup jmath
     */
    class MultiDimNormalDistribution {
      
    public:

      MultiDimNormalDistribution(std::size_t dim_, long long unsigned int seed_ = 1);
      MultiDimNormalDistribution(const jblas::vec& mean_, const jblas::vec& cov_, long long unsigned int seed_ = 1);
      ~MultiDimNormalDistribution();

      jblas::vec& get();

    private:

      jblas::vec u;

      std::vector<NormalDistribution*> normalsVec;

    };

  } // namespace jmath
} // namespace jafar


#endif // JMATH_RANDOM_HPP
