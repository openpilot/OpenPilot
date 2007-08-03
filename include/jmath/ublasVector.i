/* $Id$ */

%include "boost/version.hpp"


  
namespace boost { namespace numeric { namespace ublas {

  template<class T, class A = boost::numeric::ublas::unbounded_array<T> >
  class vector {

  public:

    //     typedef std::size_t size_type;
    typedef unsigned int size_type;
    typedef const T &const_reference;
    //typedef typename type_traits<T>::const_reference const_reference;
    typedef T &reference;
    
    vector();
    
    vector(size_type size_);

    size_type size() const;

    void resize(size_type size, bool preserve = true);

//    %rename(get) operator();
//    const_reference operator() (size_type i) const;

		%addmethods {
    const_reference get(size_type i) const{
      return (*self)(i);
    }
    }

#if defined(BOOST_VERSION) && (BOOST_VERSION < 103300)
    %rename(set) insert;
    void insert (size_type i, const_reference t);
#else
    %rename(set) insert_element;
    void insert_element (size_type i, const_reference t);
#endif

    void clear ();
  };


  template<class V>
  class vector_range {
  public:

    //     typedef typename V::size_type size_type; 
    typedef unsigned int size_type;
    //     typedef typename V::const_reference const_reference;
    typedef const double& const_reference;
     

    size_type size() const;

//   %rename(get) operator();
//    const_reference operator() (size_type i) const;

		%addmethods {
    	const_reference get(size_type i) const{
      	return (*self)(i);
    	}
    }
  };

  template<class T, std::size_t N>
  class bounded_vector {
  public:
    typedef unsigned int size_type;
    typedef const T &const_reference;
    typedef T &reference;
    bounded_vector ();
    bounded_vector (size_type size);
    bounded_vector (const bounded_vector &v);

//    %rename(get) operator();
//    const_reference operator() (size_type i) const;
	
		%addmethods {
    const_reference get(size_type i) const{
      return (*self)(i);
    }
    }

#if defined(BOOST_VERSION) && (BOOST_VERSION < 103300)
    %rename(set) insert;
    void insert (size_type i, const_reference t);
#else
    %rename(set) insert_element;
    void insert_element (size_type i, const_reference t);
#endif
     
  };

}}}
