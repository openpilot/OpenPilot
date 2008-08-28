/* $Id$ */

/** \file jblas.hpp
 *
 * This is the standard header defined in jafar to use the boost uBlas
 * library.
 *
 * \ingroup jmath
 */

#ifndef JMATH_JBLAS_HPP
#define JMATH_JBLAS_HPP

#ifdef USE_JMATH_SERIALIZATION
#include "jmath/serialize_vector.hpp"
#include "jmath/serialize_matrix.hpp"
#include "jmath/serialize_symmetric.hpp"
#include "jmath/serialize_banded.hpp"
#else
#include "boost/numeric/ublas/vector.hpp"
#include "boost/numeric/ublas/matrix.hpp"
#include "boost/numeric/ublas/symmetric.hpp"
#include "boost/numeric/ublas/banded.hpp"
#endif
#include "boost/numeric/ublas/io.hpp"
#include <boost/numeric/ublas/triangular.hpp>
#include "boost/numeric/ublas/matrix_proxy.hpp"
#include "boost/numeric/ublas/vector_proxy.hpp"
#include <boost/numeric/ublas/matrix_sparse.hpp>

#include "jmath/ublasCompatibility.hpp"

/** \addtogroup jmath */
/*@{*/

/// shortcut for ublas namespace
namespace ublas = boost::numeric::ublas;

/// special namespace to typedef ublas datatype.
namespace jblas {

  /*
   * scalar type double
   */

  /// standard vector type
  typedef boost::numeric::ublas::vector<double> vec;
  typedef boost::numeric::ublas::vector_range<vec> vec_range; 
  typedef boost::numeric::ublas::zero_vector<double> zero_vec;
  typedef boost::numeric::ublas::unit_vector<double> unit_vec;
  typedef boost::numeric::ublas::scalar_vector<double> scalar_vec;

  /// 1 dimension vector
  typedef boost::numeric::ublas::bounded_vector<double,1> vec1;
  /// 2 dimension vector
  typedef boost::numeric::ublas::bounded_vector<double,2> vec2;
  /// 3 dimension vector
  typedef boost::numeric::ublas::bounded_vector<double,3> vec3;
  /// 4 dimension vector
  typedef boost::numeric::ublas::bounded_vector<double,4> vec4;
  /// 8 dimension vector
  typedef boost::numeric::ublas::bounded_vector<double,8> vec8;

  /// standard matrix type
  typedef boost::numeric::ublas::matrix<double> mat;
  typedef boost::numeric::ublas::matrix_range<mat> mat_range;
  typedef boost::numeric::ublas::zero_matrix<double> zero_mat;
  typedef boost::numeric::ublas::identity_matrix<double> identity_mat;
  typedef boost::numeric::ublas::matrix_vector_range<mat> mat_vec_range;
  typedef boost::numeric::ublas::scalar_matrix<double> scalar_mat;
  /// column major matrix
  typedef boost::numeric::ublas::matrix<double, boost::numeric::ublas::column_major> mat_column_major;
  typedef boost::numeric::ublas::matrix_range<mat_column_major> mat_column_major_range;

  /// diagonal matrix
  typedef boost::numeric::ublas::diagonal_matrix<double> diag_mat;

  /// 2x2 dimension matrix
  typedef boost::numeric::ublas::bounded_matrix<double,2,2> mat22;

  /// 3x3 dimension matrix
  typedef boost::numeric::ublas::bounded_matrix<double,3,3> mat33;
  typedef boost::numeric::ublas::matrix_range<mat33> mat33_range;
  
  /// 4x4 dimension matrix
  typedef boost::numeric::ublas::bounded_matrix<double,4,4> mat44;
  typedef boost::numeric::ublas::matrix_range<mat44> mat44_range;

  /// 6x6 dimension matrix
  typedef boost::numeric::ublas::bounded_matrix<double,6,6> mat66;
  typedef boost::numeric::ublas::matrix_range<mat66> mat66_range;

  /// 8x8 dimension matrix
  typedef boost::numeric::ublas::bounded_matrix<double,8,8> mat88;
  typedef boost::numeric::ublas::matrix_range<mat88> mat88_range;

  /// standard symmetric matrix type
  typedef boost::numeric::ublas::symmetric_matrix<double> sym_mat;
  typedef boost::numeric::ublas::matrix_range<sym_mat> sym_mat_range; 
  typedef boost::numeric::ublas::symmetric_matrix<double, boost::numeric::ublas::upper> up_sym_mat;
  typedef boost::numeric::ublas::matrix_range<up_sym_mat> up_sym_mat_range;
  typedef boost::numeric::ublas::symmetric_matrix<double, boost::numeric::ublas::lower> lo_sym_mat;

  typedef ublas::symmetric_matrix<double, ublas::column_major> sym_mat_column_major;
  /// standard banded matrix type
  typedef boost::numeric::ublas::banded_matrix<double> banded_mat;

  /// standard slice
  typedef boost::numeric::ublas::matrix_vector_slice<mat> mat_vec_slice;
  typedef boost::numeric::ublas::matrix_vector_slice<sym_mat> sym_mat_vec_slice;

  /// standard matrix column
  typedef boost::numeric::ublas::matrix_column<mat> mat_column;

  ///standard double sparse matrix
  typedef boost::numeric::ublas::compressed_matrix<double> sp_mat;
  
  /*
   * scalar type int
   */
  /// unisized vector
  typedef boost::numeric::ublas::vector<int> veci;
  /// 2 dimension vector
  typedef boost::numeric::ublas::bounded_vector<int,2> veci2;

  /*
   * scalar type bool
   */
  
  /** bool matrix.
   * @deprecated use jblas::matb instead (naming scheme)
   */
  typedef boost::numeric::ublas::matrix<bool> bool_mat;

  /// bool matrix
  typedef boost::numeric::ublas::matrix<bool> matb;

  /**
   * boost orientation tags
   */
  typedef boost::numeric::ublas::upper upper;
  typedef boost::numeric::ublas::lower lower;
} // namespace jblas

/*@}*/
/* End of Doxygen group */

#endif // JMATH_JBLAS_HPP
