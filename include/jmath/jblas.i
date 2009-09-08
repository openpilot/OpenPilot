/* $Id$ */

/** swig/tcl interface file for jblas datatypes.
 *
 * \file jblas.i
 */

%{

#include "jmath/jblas.hpp"
#include "jmath/boundedSymmetricMatrix.hpp"

%}

%include "std_common.i"

%include "boost/version.hpp"

%include "jmath/ublasVector.i"
%include "jmath/ublasMatrix.i"
%include "jmath/jblas.hpp"
%include "jmath/boundedSymmetricMatrix.i"

/* wrap vector */

// vector of double
%template(vec) boost::numeric::ublas::vector<double>;
%template(vec2) boost::numeric::ublas::bounded_vector<double,2>;
%template(vec3) boost::numeric::ublas::bounded_vector<double,3>;
%template(vec4) boost::numeric::ublas::bounded_vector<double,4>;
%template(vec6) boost::numeric::ublas::bounded_vector<double,6>;
%template(veci4) boost::numeric::ublas::bounded_vector<int,4>;


%nodefault boost::numeric::ublas::vector_range<jblas::vec>;
%template(vec_range) boost::numeric::ublas::vector_range<jblas::vec>;

%template(sym_mat22) jblas::bounded_symmetric_matrix<double, 2>;
%template(sym_mat33) jblas::bounded_symmetric_matrix<double, 3>;
%template(sym_mat44) jblas::bounded_symmetric_matrix<double, 4>;

/* wrap matrix */

// matrix of double
%template(mat) boost::numeric::ublas::matrix<double>;
%template(mat22) boost::numeric::ublas::bounded_matrix<double,2,2>;
%template(mat33) boost::numeric::ublas::bounded_matrix<double,3,3>;
%template(mat44) boost::numeric::ublas::bounded_matrix<double,4,4>;
%template(mat99) boost::numeric::ublas::bounded_matrix<double,9,9>;

// column major matrix of double
%template(mat_column_major) boost::numeric::ublas::matrix<double, boost::numeric::ublas::column_major>;

// symmetric matrix of double
%extend boost::numeric::ublas::symmetric_matrix {
  %template(assignMat) assign<boost::numeric::ublas::matrix<double> > ;
  %template(assignSymMat) assign<boost::numeric::ublas::symmetric_matrix<double> >;
};
%template(sym_mat) boost::numeric::ublas::symmetric_matrix<double>;


%nodefault boost::numeric::ublas::matrix_range<jblas::sym_mat>;
%template(sym_mat_range) boost::numeric::ublas::matrix_range<jblas::sym_mat>;

%template(mat44) boost::numeric::ublas::bounded_matrix<double,4,4>;
/* %template(mat55) boost::numeric::ublas::bounded_matrix<double,5,5>; */