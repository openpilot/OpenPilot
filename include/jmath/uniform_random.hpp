#ifndef JMATH_UNIFORM_GENERATOR_HPP
#define JMATH_UNIFORM_GENERATOR_HPP

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <limits>
#include <ctime>

/// shortcut for ublas namespace
namespace ublas = boost::numeric::ublas;

namespace jafar {
	namespace jmath {
		///uniform distribution meta struct
		template <typename T> struct uniform_distribution;
		///uniform distribution int specialized
		template<> struct uniform_distribution<int> {
			typedef boost::uniform_int<int> type;
		};
		///uniform distribution unsigned int specialized
		template<> struct uniform_distribution<unsigned int> {
			typedef boost::uniform_int<unsigned int> type;
		};
		///uniform distribution float specialized
		template<> struct uniform_distribution<float> {
			typedef boost::uniform_real<float> type;
		};
		///uniform distribution double specialized
		template<> struct uniform_distribution<double> {
			typedef boost::uniform_real<double> type;
		};

		template<typename T>
		class UNIFORM_GENERATOR {
 			T min;
			T max;
			uint32_t seed_value;
			typedef boost::mt19937 generator_type;
			typedef typename uniform_distribution<T>::type distribution_type;
			distribution_type distribution;
			generator_type rng;
			boost::variate_generator<generator_type&, distribution_type> generator;

		public:
			/**
			 * @param _min: included lower bound
			 * @param _max: excluded higher bound
			 * @param _seed: seeding value
			 */
			UNIFORM_GENERATOR(T _min = 0, 
												T _max = 1, 
												uint32_t _seed = -1) : 
				min(_min), max(_max), seed_value(_seed),
				distribution(_min, _max), generator(rng, distribution) {
				if(seed_value != -1)
					rng.seed(_seed);
			}
			/**
			 * @return a randomly generated number in the interval [min, max[
			 */
			inline T run() {
				return generator();
			}

			/**  
			 * change seed value
			 * @_seed : new seed value
			 */
			void seed(uint32_t _seed) {
				rng.seed(_seed);
			}
			/**
			 * fills vector @ref v with uniform randomly generated numbers
			 */
			inline void fill_vector(ublas::vector<T>& v) {
				for(typename ublas::vector<T>::iterator it = v.begin(); it != v.end(); it++)
					*it = run();
			}			
			inline void fill_vector(std::vector<T>& v) {
				for(typename std::vector<T>::iterator it = v.begin(); it != v.end(); it++)
					*it = run();
			}
			/**
			 * fills matrix @ref m with uniform randomly generated numbers
			 */
			inline void fill_matrix(ublas::matrix<T>& m) {
				for(typename ublas::matrix<T>::iterator1 rit = m.begin1(); rit != m.end1(); rit++)
					for(typename ublas::matrix<T>::iterator2 it = rit.begin(); it != rit.end(); it++)
						*it = run();
			}
		};
		typedef UNIFORM_GENERATOR<double> uniform_generator;
	}

	namespace random {
		/**  
     * @param min : lower bound for generated values
		 * @param max : upper bound for generated values
     * @return vector filled with uniform distribution of random values in [min, max[
     */		
		template<typename T>
		static void uniform_fill(T min, T max, ublas::vector<T>& v) {
			jafar::jmath::UNIFORM_GENERATOR<T> gen(min, max, static_cast<unsigned int>(std::time(0)));
			gen.fill_vector(v);
		}
		/**  
     * @param min : lower bound for generated values
		 * @param max : upper bound for generated values
     * @return vector filled with uniform distribution of random values in [min, max[
     */		
		template<typename T>
		static void uniform_fill(T min, T max, std::vector<T>& v) {
			jafar::jmath::UNIFORM_GENERATOR<T> gen(min, max, static_cast<unsigned int>(std::time(0)));
			gen.fill_vector(v);
		}

		/**  
     * @param min : lower bound for generated values
		 * @param max : upper bound for generated values
     * @return matrix filled with uniform distribution of random values in [min, max[
     */		
		template<typename T>
		static void uniform_fill(T min, T max, ublas::matrix<T>& m) {
			jafar::jmath::UNIFORM_GENERATOR<T> gen(min, max, static_cast<unsigned int>(std::time(0)));
			gen.fill_matrix(m);
		}
	}

}

#endif
