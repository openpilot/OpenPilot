
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
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/symmetric.hpp>
#include <boost/numeric/ublas/banded.hpp>
#endif
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/triangular.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>

#include "boost/math/quaternion.hpp"

#include "jmath/ublasCompatibility.hpp"
#include "jmath/boundedSymmetricMatrix.hpp"

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
  typedef ublas::vector<double> vec;
  typedef ublas::vector_range<vec> vec_range; 
  typedef ublas::vector_slice<vec> vec_slice;
  typedef ublas::vector_indirect<vec> vec_indirect;
  typedef ublas::vector_range<const vec> cvec_range; 
  typedef ublas::zero_vector<double> zero_vec;
  typedef ublas::unit_vector<double> unit_vec;
  typedef ublas::scalar_vector<double> scalar_vec;

  /// 1 dimension vector
  typedef ublas::bounded_vector<double,1> vec1;
  /// 2 dimension vector
  typedef ublas::bounded_vector<double,2> vec2;
  /// 3 dimension vector
  typedef ublas::bounded_vector<double,3> vec3;
  /// 4 dimension vector
  typedef ublas::bounded_vector<double,4> vec4;
  typedef ublas::vector_range<vec4> vec4_range; 
  /// 6 dimension vector
  typedef ublas::bounded_vector<double,6> vec6;
  /// 7 dimension vector
  typedef ublas::bounded_vector<double,7> vec7;
  /// 8 dimension vector
  typedef ublas::bounded_vector<double,8> vec8;


  /// standard matrix type
  typedef ublas::matrix<double> mat;
  
  typedef ublas::matrix_range<mat> mat_range;
  typedef ublas::matrix_slice<mat> mat_slice;
  typedef ublas::matrix_indirect<mat> mat_indirect;
  typedef ublas::matrix_range<const mat> cmat_range;
  typedef ublas::zero_matrix<double> zero_mat;
  typedef ublas::identity_matrix<double> identity_mat;
  typedef ublas::matrix_vector_range<mat> mat_vec_range;
  typedef ublas::scalar_matrix<double> scalar_mat;
  /// column major matrix
  typedef ublas::matrix<double, ublas::column_major> mat_column_major;
  typedef ublas::matrix_range<const mat_column_major> cmat_column_major_range;
  typedef ublas::matrix_range<mat_column_major> mat_column_major_range;

  /// diagonal matrix
  typedef ublas::diagonal_matrix<double> diag_mat;

  /// Fixed dimension matrices and matrix ranges
  typedef ublas::bounded_matrix<double,2,2> mat22; // 2x2
  typedef ublas::bounded_matrix<double,2,3> mat23; // 2x3
  typedef ublas::bounded_matrix<double,3,2> mat32; // 3x2
  typedef ublas::bounded_matrix<double,3,3> mat33; // 3x3
  typedef ublas::matrix_range<mat33> mat33_range; //  3x3
  typedef ublas::bounded_matrix<double,3,4> mat34; // 3x4
  typedef ublas::matrix_range<mat34> mat34_range; //  3x4
  typedef ublas::bounded_matrix<double,4,3> mat43; // 4x3
  typedef ublas::matrix_range<mat43> mat43_range; //  4x3
  typedef ublas::bounded_matrix<double,4,4> mat44; // 4x4
  typedef ublas::matrix_range<mat44> mat44_range; //  4x4
  typedef ublas::bounded_matrix<double,6,6> mat66; // 6x6
  typedef ublas::matrix_range<mat66> mat66_range; //  6x6
  typedef ublas::bounded_matrix<double,8,8> mat88; // 8x8
  typedef ublas::matrix_range<mat88> mat88_range; //  8x8
  typedef ublas::bounded_matrix<double,9,9> mat99; // 9x9
  typedef ublas::matrix_range<mat99> mat99_range; //  9x9
  
  /// standard symmetric matrix type
  typedef ublas::symmetric_matrix<double> sym_mat;
  /// matrix range
  typedef ublas::matrix_range<sym_mat> sym_mat_range; 
  /// indirect access to a symmetric matrix
  typedef ublas::matrix_indirect<sym_mat> sym_mat_indirect;
  typedef ublas::symmetric_matrix<double, ublas::upper> up_sym_mat;
  typedef ublas::matrix_range<up_sym_mat> up_sym_mat_range;
  typedef ublas::symmetric_matrix<double, ublas::lower> lo_sym_mat;

  typedef ublas::symmetric_matrix<double, ublas::column_major> sym_mat_column_major;
    
  /// 2x2 symmetric matrix type
  typedef bounded_symmetric_matrix<double, 2 > sym_mat22;
  /// 3x3 symmetric matrix type
  typedef bounded_symmetric_matrix<double, 3 > sym_mat33;
  /// 4x4 symmetric matrix type
  typedef bounded_symmetric_matrix<double, 4 > sym_mat44;
  
  /// standard banded matrix type
  typedef ublas::banded_matrix<double> banded_mat;

  /// standard slice
  typedef ublas::matrix_vector_slice<mat> mat_vec_slice;
  typedef ublas::matrix_vector_slice<sym_mat> sym_mat_vec_slice;

  /// standard matrix column
  typedef ublas::matrix_column<mat> mat_column;

  ///standard double sparse matrix
  typedef ublas::compressed_matrix<double> sp_mat;
  
  ///indirect array
  typedef ublas::indirect_array<> ind_array;

  /// Quaternions
  typedef boost::math::quaternion<double> quat;

  /*
   * Scalar type float
   */

  /// standard vector type
  typedef ublas::vector<float> vecf;
  typedef ublas::scalar_vector<float> scalar_vecf;

  /// 1 dimension vector
  typedef ublas::bounded_vector<float,1> vecf1;
  /// 2 dimension vector
  typedef ublas::bounded_vector<float,2> vecf2;
  /// 3 dimension vector
  typedef ublas::bounded_vector<float,3> vecf3;
  /// 4 dimension vector
  typedef ublas::bounded_vector<float,4> vecf4;
  /// 6 dimension vector
  typedef ublas::bounded_vector<float,6> vecf6;
  /// 8 dimension vector
  typedef ublas::bounded_vector<float,8> vecf8;

  /*
   * scalar type int
   */
  /// unisized vector
  typedef ublas::vector<int> veci;
  /// 2 dimension vector
  typedef ublas::bounded_vector<int,2> veci2;
  /// 3 dimension vector
  typedef ublas::bounded_vector<int,3> veci3;
  /// 4 dimension vector
  typedef ublas::bounded_vector<int,4> veci4;
  
  typedef ublas::zero_vector<int> zero_veci;
  typedef ublas::matrix<int> mati;
  typedef ublas::zero_matrix<int> zero_mati;
	
  /*
   * scalar type bool
   */
  
  /** bool matrix.
   * @deprecated use jblas::matb instead (naming scheme)
   */
  typedef ublas::matrix<bool> bool_mat;

  /// bool matrix
  typedef ublas::matrix<bool> matb;
  typedef ublas::vector<bool> vecb;

  /**
   * boost orientation tags
   */
  typedef ublas::upper upper;
  typedef ublas::lower lower;
	
	typedef ublas::symmetric_adaptor<const jblas::mat> sym_adapt;
	/**
	 * boost matrixes proxies
	 */
	typedef ublas::matrix_row< ublas::matrix<double, ublas::column_major> > mat_colum_major_row;
} // namespace jblas

/*@}*/
/* End of Doxygen group */

#endif // JMATH_JBLAS_HPP
