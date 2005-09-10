#ifndef _JMATH_SERIALIZATION_HPP_
#define _JMATH_SERIALIZATION_HPP_

#include "jmath/jblas.hpp"

#ifdef BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP
namespace boost { namespace serialization {
#else
namespace boost { namespace numeric { namespace ublas {
#endif
    template<class Archive, class T, std::size_t S>
    inline void serialize(
    Archive & ar,
    boost::numeric::ublas::bounded_vector<T, S> &v,
    const unsigned int file_version
    ){
      for(int i = 0; i < S; i++)
      {
        ar & v(i);
      }
    }

#ifdef BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP
}; };
#else
}; }; };
#endif

#endif
