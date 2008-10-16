/* $Id: $ */

namespace jblas {
  /// Bounded symmetric matrix class
  template<class T, std::size_t M, class TRI = boost::numeric::ublas::lower, class L = boost::numeric::ublas::row_major>
  class bounded_symmetric_matrix:
      public boost::numeric::ublas::symmetric_matrix<T, TRI, L, boost::numeric::ublas::bounded_array<T, M * M> > {

      typedef boost::numeric::ublas::symmetric_matrix<T, TRI, L, boost::numeric::ublas::bounded_array<T, M * M> > symmetric_matrix_type;
  public:
      typedef typename symmetric_matrix_type::size_type size_type;
      
      // Construction and destruction
      bounded_symmetric_matrix ();
      bounded_symmetric_matrix (size_type size1, size_type size2);
      bounded_symmetric_matrix (const bounded_symmetric_matrix &m);
      template<class A2>
      bounded_symmetric_matrix (const ublas::matrix<T, L, A2> &m);
      template<class AE>
      bounded_symmetric_matrix (const ublas::matrix_expression<AE> &ae);
      ~bounded_symmetric_matrix ();

      // Assignment
      bounded_symmetric_matrix &operator = (const bounded_symmetric_matrix &m);
      template<class L2, class A2>        // Symmetric matrix assignment
      bounded_symmetric_matrix &operator = (const ublas::symmetric_matrix<T, L2, A2> &m);
      template<class L2, class A2>        // Generic matrix assignment
      bounded_symmetric_matrix &operator = (const ublas::matrix<T, L2, A2> &m);
      template<class C>          // Container assignment without temporary
      bounded_symmetric_matrix &operator = (const ublas::matrix_container<C> &m);
      template<class AE>
      bounded_symmetric_matrix &operator = (const ublas::matrix_expression<AE> &ae);
  };
}
