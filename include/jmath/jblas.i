/* $Id$ */

/** swig/tcl interface file for jblas datatypes.
 *
 * \file jblas.i
 */

%{

#include "jmath/jblas.hpp"

%}

%include "std_common.i"

%include "boost/version.hpp"

%include "jmath/ublasVector.i"
%include "jmath/ublasMatrix.i"
%include "jmath/jblas.hpp"

/* wrap vector */

// vector of double
%template(vec) boost::numeric::ublas::vector<double>;

%template(vec2) boost::numeric::ublas::bounded_vector<double,2>;

%template(vec3) boost::numeric::ublas::bounded_vector<double,3>;

%nodefault boost::numeric::ublas::vector_range<jblas::vec>;
%template(vec_range) boost::numeric::ublas::vector_range<jblas::vec>;


/* wrap matrix */

// matrix of double
%template(mat) boost::numeric::ublas::matrix<double>;

// column major matrix of double
%template(mat_column_major) boost::numeric::ublas::matrix<double, boost::numeric::ublas::column_major>;

// symmetric matrix of double
%template(sym_mat) boost::numeric::ublas::symmetric_matrix<double>;

%nodefault boost::numeric::ublas::matrix_range<jblas::sym_mat>;
%template(sym_mat_range) boost::numeric::ublas::matrix_range<jblas::sym_mat>;

%template(mat44) boost::numeric::ublas::bounded_matrix<double,4,4>;
