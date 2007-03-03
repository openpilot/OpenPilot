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

		template<class Archive, class T, std::size_t M, std::size_t N>
		inline void serialize
		Archive & ar,
		boost::numeric::ublas::bounded_matrix<T, M, N> &m,
		const unsigned int file_version
		) {
			for(int i=0; i<M;i++)
				for(int j=0; j<N;i++)
					ar & m(i,j);
		}

#ifdef BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP
}; };
#else
}; }; };
#endif

#endif
