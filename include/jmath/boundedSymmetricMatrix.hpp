/* $Id: $ */

#ifndef _JBLAS_BOUNDED_SYMMETRIC_MATRIX_HPP_
#define _JBLAS_BOUNDED_SYMMETRIC_MATRIX_HPP_

namespace ublas = boost::numeric::ublas;

namespace jblas {
  /// Bounded symmetric matrix class
  template<class T, std::size_t M, class TRI = boost::numeric::ublas::lower, class L = boost::numeric::ublas::row_major>
  class bounded_symmetric_matrix:
      public boost::numeric::ublas::symmetric_matrix<T, TRI, L, boost::numeric::ublas::bounded_array<T, M * M> > {

      typedef boost::numeric::ublas::symmetric_matrix<T, TRI, L, boost::numeric::ublas::bounded_array<T, M * M> > symmetric_matrix_type;
  public:
      typedef typename symmetric_matrix_type::size_type size_type;
      static const size_type max_size1 = M;
      static const size_type max_size2 = M;

      // Construction and destruction
      BOOST_UBLAS_INLINE
      bounded_symmetric_matrix ():
          symmetric_matrix_type (M, M) {}
      BOOST_UBLAS_INLINE
      bounded_symmetric_matrix (size_type size1, size_type size2):
          symmetric_matrix_type (size1, size2) {}
      BOOST_UBLAS_INLINE
      bounded_symmetric_matrix (const bounded_symmetric_matrix &m):
          symmetric_matrix_type (m) {}
      template<class A2>              // Allow matrix<T, L, bounded_array<M,N> > construction
      BOOST_UBLAS_INLINE
      bounded_symmetric_matrix (const ublas::matrix<T, L, A2> &m):
          symmetric_matrix_type (m) {}
      template<class AE>
      BOOST_UBLAS_INLINE
      bounded_symmetric_matrix (const ublas::matrix_expression<AE> &ae):
          symmetric_matrix_type (ae) {}
      BOOST_UBLAS_INLINE
      ~bounded_symmetric_matrix () {}

      // Assignment
      BOOST_UBLAS_INLINE
      bounded_symmetric_matrix &operator = (const bounded_symmetric_matrix &m) {
          symmetric_matrix_type::operator = (m);
          return *this;
      }
      template<class L2, class A2>        // Symmetric matrix assignment
      BOOST_UBLAS_INLINE
      bounded_symmetric_matrix &operator = (const ublas::symmetric_matrix<T, L2, A2> &m) {
          symmetric_matrix_type::operator = (m);
          return *this;
      }
      template<class L2, class A2>        // Generic matrix assignment
      BOOST_UBLAS_INLINE
      bounded_symmetric_matrix &operator = (const ublas::matrix<T, L2, A2> &m) {
          symmetric_matrix_type::operator = (m);
          return *this;
      }
      template<class C>          // Container assignment without temporary
      BOOST_UBLAS_INLINE
      bounded_symmetric_matrix &operator = (const ublas::matrix_container<C> &m) {
          symmetric_matrix_type::operator = (m);
          return *this;
      }
      template<class AE>
      BOOST_UBLAS_INLINE
      bounded_symmetric_matrix &operator = (const ublas::matrix_expression<AE> &ae) {
          symmetric_matrix_type::operator = (ae);
          return *this;
      }
  };
}

#endif
