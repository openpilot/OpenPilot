/* $Id$ */

#ifndef JMATH_RANDOM_HPP
#define JMATH_RANDOM_HPP

#include <vector>

#include "boost/random.hpp"
#include <boost/numeric/ublas/blas.hpp>

#include "jmath/jblas.hpp"


namespace jafar {
  namespace jmath {

		/**
			returns a good random "random seed"
		*/
		unsigned get_srand();
		
		
    /** Uniform distribution generator using the Boost random
     * library. Have a look at demoRandom.tcl.
     *
     * \ingroup jmath
     */
    class UniformDistribution {
    public:

      UniformDistribution(unsigned int seed_ = 1);
      UniformDistribution(double min_, double max_, unsigned int seed_ = 1);
      ~UniformDistribution();

      double get();

    private:

      boost::mt19937 rng;
      boost::uniform_real<double> uniform;

      boost::variate_generator<boost::mt19937, boost::uniform_real<double> > vg;
    };

    /** Multi dimensionnal uniform distribution generator using the
     * Boost random library. Have a look at demoRandom.tcl.
     *
     * \ingroup jmath
     */
    class MultiDimUniformDistribution {

    public:

      MultiDimUniformDistribution(std::size_t dim_, unsigned int seed_ = 1);
      MultiDimUniformDistribution(std::size_t dim_, double min_, double max_, unsigned int seed_ = 1);
      MultiDimUniformDistribution(const jblas::vec& min_, const jblas::vec& max_, unsigned int seed_ = 1);
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

      NormalDistribution(unsigned int seed_ = 1);
      NormalDistribution(double mean_, double sigma_, unsigned int seed_ = 1);

      ~NormalDistribution();

      double get();

    private:
      
      boost::mt19937 rng;
      boost::normal_distribution<double> normal;

      boost::variate_generator<boost::mt19937, boost::normal_distribution<double> > vg;

    };

    /** Multi dimensionnal normal distribution generator using the
     * Boost random library. Have a look at demoRandom.tcl.
     *
     * \ingroup jmath
     */
    class MultiDimNormalDistribution {
      
    public:

      MultiDimNormalDistribution(std::size_t dim_, unsigned int seed_ = 1);
      MultiDimNormalDistribution(std::size_t dim_, double mean_, double cov_, unsigned int seed_ = 1);
      MultiDimNormalDistribution(const jblas::vec& mean_, const jblas::vec& cov_, unsigned int seed_ = 1);
      ~MultiDimNormalDistribution();

      jblas::vec get();

    private:

      std::vector<NormalDistribution*> normalsVec;

    };






		template<typename bubTemplateMatrix>
		void randMatrix(bubTemplateMatrix& M) {
			for (size_t i = 0; i < M.size1(); ++i)
				for (size_t j = 0; j < M.size2(); ++j) {
					M(i, j) = ((rand() + 0.0) / RAND_MAX * 2) - 1.;
				}
		}

		template<typename bubTemplateMatrix>
		void randMatrix(bubTemplateMatrix& M, const size_t row, const size_t col) {
			M.resize(row, col);
			randMatrix(M);
		}

		template<typename bubTemplateVector>
		void randVector(bubTemplateVector& V) {
			for (size_t i = 0; i < V.size(); ++i) {
				V(i) = ((rand() + 0.0) / RAND_MAX * 2) - 1.;
			}
		}

		template<typename bubTemplateVector>
		void randVector(bubTemplateVector& V, const size_t size) {
			V.resize(size);
			randVector(V);
		}


  } // namespace jmath
} // namespace jafar


#endif // JMATH_RANDOM_HPP
