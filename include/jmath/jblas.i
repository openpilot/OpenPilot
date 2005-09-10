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

%include "jmath/swigVector.hpp"
%include "jmath/swigMatrix.hpp"
%include "jmath/jblas.hpp"

/* wrap vector */

// vector of double
%rename(vec) boost::numeric::ublas::vector<double>;
%template(vec) boost::numeric::ublas::vector<double>;

%rename(vec3) boost::numeric::ublas::bounded_vector<double,3>;
%template(vec3) boost::numeric::ublas::bounded_vector<double,3>;

%rename(vec_range) boost::numeric::ublas::vector_range<jblas::vec>;
%template(vec_range) boost::numeric::ublas::vector_range<jblas::vec>;

/* wrap matrix */

// matrix of double
%rename(mat)  boost::numeric::ublas::matrix<double>;
%template(mat) boost::numeric::ublas::matrix<double>;

// symmetric matrix of double
%rename(sym_mat)  boost::numeric::ublas::symmetric_matrix<double>;
%template(sym_mat) boost::numeric::ublas::symmetric_matrix<double>;

%rename(sym_mat_range)  boost::numeric::ublas::matrix_range<jblas::sym_mat>;
%template(sym_mat_range) boost::numeric::ublas::matrix_range<jblas::sym_mat>;

%rename(mat44) boost::numeric::ublas::bounded_matrix<double,4,4>;
%template(mat44) boost::numeric::ublas::bounded_matrix<double,4,4>;
