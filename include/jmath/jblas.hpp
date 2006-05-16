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

#include "boost/numeric/ublas/vector.hpp"
#include "boost/numeric/ublas/matrix.hpp"
#include "boost/numeric/ublas/symmetric.hpp"
#include "boost/numeric/ublas/banded.hpp"
#include "boost/numeric/ublas/io.hpp"

#include "boost/numeric/ublas/matrix_proxy.hpp"
#include "boost/numeric/ublas/vector_proxy.hpp"

#include "jmath/ublasCompatibility.hpp"

/// shortcut for ublas namespace
namespace ublas = boost::numeric::ublas;

namespace jblas { /// special namespace to typedef ublas datatype.

  /// standard vector type
  typedef boost::numeric::ublas::vector<double> vec;
  typedef boost::numeric::ublas::zero_vector<double> zero_vec;

  /// 1 dimension vector
  typedef boost::numeric::ublas::bounded_vector<double,1> vec1;
  /// 2 dimension vector
  typedef boost::numeric::ublas::bounded_vector<double,2> vec2;
  /// 3 dimension vector
  typedef boost::numeric::ublas::bounded_vector<double,3> vec3;
  /// 4 dimension vector
  typedef boost::numeric::ublas::bounded_vector<double,4> vec4;

  /// standard matrix type
  typedef boost::numeric::ublas::matrix<double> mat;
  typedef boost::numeric::ublas::zero_matrix<double> zero_mat;
  typedef boost::numeric::ublas::identity_matrix<double> identity_mat;

  /// column major matrix
  typedef boost::numeric::ublas::matrix<double, boost::numeric::ublas::column_major> mat_column_major;

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

  /// standard symmetric matrix type
  typedef boost::numeric::ublas::symmetric_matrix<double> sym_mat;

  /// standard banded matrix type
  typedef boost::numeric::ublas::banded_matrix<double> banded_mat;

  /// standard range
  typedef boost::numeric::ublas::vector_range<vec> vec_range; 
  typedef boost::numeric::ublas::matrix_range<mat> mat_range; 
  typedef boost::numeric::ublas::matrix_range<sym_mat> sym_mat_range; 

  /// standard slice
  typedef boost::numeric::ublas::matrix_vector_slice<mat> mat_vec_slice;
  typedef boost::numeric::ublas::matrix_vector_slice<sym_mat> sym_mat_vec_slice;

  typedef boost::numeric::ublas::matrix_column<mat> mat_column;

} // namespace jblas

#endif // JMATH_JBLAS_HPP
