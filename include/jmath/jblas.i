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
%template(Vec) boost::numeric::ublas::vector<double>;
%template(Vec2) boost::numeric::ublas::bounded_vector<double,2>;
%template(Vec3) boost::numeric::ublas::bounded_vector<double,3>;
%template(Vec4) boost::numeric::ublas::bounded_vector<double,4>;
%template(Vec6) boost::numeric::ublas::bounded_vector<double,6>;
%template(Veci4) boost::numeric::ublas::bounded_vector<int,4>;


%nodefault boost::numeric::ublas::vector_range<jblas::vec>;
%template(Vec_range) boost::numeric::ublas::vector_range<jblas::vec>;

%template(Sym_mat22) jblas::bounded_symmetric_matrix<double, 2>;
%template(Sym_mat33) jblas::bounded_symmetric_matrix<double, 3>;
//%template(Sym_mat44) jblas::bounded_symmetric_matrix<double, 4>;

/* wrap matrix */

// matrix of double
%template(Mat) boost::numeric::ublas::matrix<double>;
%template(Mat22) boost::numeric::ublas::bounded_matrix<double,2,2>;
%template(Mat33) boost::numeric::ublas::bounded_matrix<double,3,3>;
%template(Mat44) boost::numeric::ublas::bounded_matrix<double,4,4>;
%template(Mat99) boost::numeric::ublas::bounded_matrix<double,9,9>;

// column major matrix of double
%template(Mat_column_major) boost::numeric::ublas::matrix<double, boost::numeric::ublas::column_major>;

// symmetric matrix of double
%extend boost::numeric::ublas::symmetric_matrix {
  %template(assignMat) assign<boost::numeric::ublas::matrix<double> > ;
  %template(assignSymMat) assign<boost::numeric::ublas::symmetric_matrix<double> >;
};
%template(Sym_mat) boost::numeric::ublas::symmetric_matrix<double>;


%nodefault boost::numeric::ublas::matrix_range<jblas::sym_mat>;
%template(Sym_mat_range) boost::numeric::ublas::matrix_range<jblas::sym_mat>;

%template(Mat44) boost::numeric::ublas::bounded_matrix<double,4,4>;
/* %template(Mat55) boost::numeric::ublas::bounded_matrix<double,5,5>; */
