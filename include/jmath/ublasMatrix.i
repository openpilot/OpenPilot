/* $Id$ */

%include "boost/version.hpp"

namespace boost { namespace numeric { namespace ublas {

  template<class T, 
	   class F = boost::numeric::ublas::row_major, 
	   class A = boost::numeric::ublas::unbounded_array<T> >
  class matrix {

  public:

    //       typedef std::size_t size_type;
    typedef unsigned int size_type;
    typedef const T &const_reference;

    matrix();
    matrix(size_type size1, size_type size2);

    size_type size1() const;
    size_type size2() const;
    void resize (size_type size1, size_type size2, bool preserve = true);

//    %rename(get) operator();
//    const_reference operator() (size_type i, size_type j) const;

		%addmethods {
    const_reference get(size_type i,size_type j) const{
      return (*self)(i,j);
    }
    }

#if defined(BOOST_VERSION) && (BOOST_VERSION < 103300)
    %rename(set) insert;
    void insert (size_type i, size_type j, const_reference t);
#else
    %rename(set) insert_element;
    void insert_element (size_type i, size_type j, const_reference t);
#endif

    void clear ();
  };

  template<class T, 
	   class F1 = boost::numeric::ublas::lower, 
	   class F2 = boost::numeric::ublas::row_major, 
	   class A = boost::numeric::ublas::unbounded_array<T> >
  class symmetric_matrix {

  public:

    //     typedef std::size_t size_type;
    typedef unsigned int size_type;
    typedef const T &const_reference;

    symmetric_matrix();
    symmetric_matrix(size_type size);
    symmetric_matrix(size_type size1, size_type size2);


    size_type size1() const;
    size_type size2() const;
    
    void resize (size_type size, bool preserve = true);

//    %rename(get) operator();
//    const_reference operator() (size_type i, size_type j) const;

		%addmethods {
    const_reference get(size_type i, size_type j) const{
      return (*self)(i,j);
    }
    }

#if defined(BOOST_VERSION) && (BOOST_VERSION < 103300)    
    %rename(set) insert;
    void insert (size_type i, size_type j, const_reference t);
#else
    %rename(set) insert_element;
    void insert_element (size_type i, size_type j, const_reference t);
#endif
    
    void clear ();

  };

  template<class M>
  class matrix_range {
  public:

    //     typedef typename M::size_type size_type;
    typedef unsigned int size_type;

    //     typedef typename V::const_reference const_reference;
    typedef const double& const_reference;
    
    size_type size1() const;
    size_type size2() const;

//    %rename(get) operator();
//    const_reference operator() (size_type i, size_type j) const;

		%addmethods {
    const_reference get(size_type i,size_type j) const{
      return (*self)(i,j);
    }
    }

  };

  template<class T, std::size_t M, 
	   std::size_t N, 
	   class F = boost::numeric::ublas::row_major>
  class bounded_matrix {
  public:
    typedef typename matrix_type::size_type size_type;
    bounded_matrix ();
    bounded_matrix (size_type size1, size_type size2);
    bounded_matrix (const bounded_matrix &m);
    template<class A2>
    bounded_matrix (const matrix<T, A2, F> &m);
  };

}}}
