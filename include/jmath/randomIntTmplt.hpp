/** @file RandomIntTmplt : a template class for generation of random intergers
 *
 * The IntType can be int or unsigned int, or..
 * you can use directly the type predefined:
 * typedef RandomIntTmplt<int> RandomInt;
 * typedef RandomIntTmplt<unsigned int> RandomUInt;
 * typedef RandomIntVectTmplt<int> RandomIntVect;
 * typedef RandomIntVectTmplt<unsigned int> RandomUIntVect;
 */

#ifndef RANDOM_INT_TEMPLT_HPP
#define RANDOM_INT_TEMPLT_HPP

#include <boost/random.hpp>

/** @brief Random Integer Generator Template
 *  @class RandomIntTmplt
 *  @author Ayman ZUREIKI azureiki@laas.fr
 *  date: January/2005
 *
 * Used to generate random number within the range [min,max]
 * @note : It is i;portant to supply a 'good' seed, for
 * example you can use :
 * SetSeed((uint64_t)std::time(NULL));
 * But, it is poor if you use it very frequently: the time dosn't
 * change a lot, donc give nearly the same random numbers
 */

namespace jafar {
  namespace jmath {

    template <class IntType>
    class RandomIntTmplt
    {
    public:
      RandomIntTmplt(IntType min_=0, IntType max_=9, uint64_t unSeed=1);
      ~RandomIntTmplt(){};
      
      /** @brief get the random interger
       *
       * this function return in each call a different integer
       * randomly selected from the rang [min,max] 
       */
      IntType get();
      
      /** change the seed
       */
      void SetSeed(uint64_t unSeed);
      
    private:
      boost::rand48 r_n_gen;/// Random Number Generator
      boost::uniform_int<> unif_dist;/// uniform distribution type
      boost::variate_generator<boost::rand48&, boost::uniform_int<> > v_g;/// variate generator
    };
    
    /** @class RandomIntVectTmplt return a vector of Random Inetegers
     *  @brief Random generation of vector of integers
     *
     * used to get a vector of random interger whithin the range
     * [min,max] of size nSize
     * unSeed can be used (recommended) to variate the seed of the random
     * algorithm 
     */
    template <class IntType>
    class RandomIntVectTmplt
    {
    public:
      RandomIntVectTmplt(std::size_t nSize, 
			 IntType nMin=0, 
			 IntType nMax=9, 
			 uint64_t unSeed=1);
      ~RandomIntVectTmplt(){};
      
      /** @brief get a vector of random intergers
       *
       * this function return in each call a different vector of integers
       * randomly selected from the rang [min,max] 
       */
      std::vector<IntType>& get();
      
      /** @brief get a vector of random Different integers
       *
       * this function return in each call a different vector of 
       * Different integers randomly selected from the rang [min,max]
       * @note of course nSize < (nMax-nMin) must be verified !
       */
      std::vector<IntType>& getDifferent();
      
      /** change the seed
       */
      void SetSeed(uint64_t unSeed);
      
    private:
      boost::rand48 r_n_gen;/// Random Number Generator
      boost::uniform_int<> unif_dist;/// uniform distribution type

      /// variate generator
      boost::variate_generator<boost::rand48&, boost::uniform_int<> > v_g;
      
      std::vector<IntType> u_;
    };
    
    /* Implementation */
    
    
    
    
    template <class IntType>
    RandomIntTmplt<IntType>::RandomIntTmplt(IntType nMin, 
					    IntType nMax, 
					    uint64_t unSeed)
      :r_n_gen(unSeed),
       unif_dist(nMin, nMax), 
       v_g(r_n_gen,unif_dist)
    {}
    
    template <class IntType>
    IntType RandomIntTmplt<IntType>::get()
    {
      return v_g();
    }
    
    template <class IntType>
    void  RandomIntTmplt<IntType>::SetSeed(uint64_t unSeed)
    {
      r_n_gen.seed((uint64_t)unSeed);
    }
    
    
    
    /*
     * class RandomIntVectTmplt
     */
    
    template <class IntType>
    RandomIntVectTmplt<IntType>::RandomIntVectTmplt(std::size_t nSize, 
						    IntType nMin, 
						    IntType nMax, 
						    uint64_t unSeed)
      : u_(nSize,0), 
	r_n_gen(unSeed),
	unif_dist(nMin, nMax), 
	v_g(r_n_gen,unif_dist)
    {}
    
    template <class IntType>
    std::vector<IntType>& RandomIntVectTmplt<IntType>::get()
    {
      for (unsigned int i=0; i<u_.size(); i++)
	u_[i]=v_g();
      return u_;
    }
    
    
    template <class IntType>
    std::vector<IntType>& RandomIntVectTmplt<IntType>::getDifferent()
    {
      IntType n;
      unsigned int i=0;
      while (i < u_.size()) {
	n = v_g();
	// if the number is not already used, add it
	if (find(u_.begin(), u_.begin()+i, n)==(u_.begin()+i)) {
	  u_[i]=n;
	  i++;
	}
      }
      return u_;
    }
    

    template <class IntType>
    void  RandomIntVectTmplt<IntType>::SetSeed(uint64_t unSeed)
    {
      r_n_gen.seed((uint64_t)unSeed);
    }
    
    /** typedef
     *
     */
    typedef RandomIntTmplt<int> RandomInt;
    typedef RandomIntTmplt<unsigned int> RandomUInt;
    typedef RandomIntVectTmplt<int> RandomIntVect;
    typedef RandomIntVectTmplt<unsigned int> RandomUIntVect;
    

  } // namespace jmath
} // namespace jafar

#endif // RANDOM_INT_TEMPLT_HPP
