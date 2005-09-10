/* $Id$ */

/** swig/tcl interface file for module jmath.
 *
 * \file jmath.i
 */      

%module jmath

%{

  // to wrap std::string
#include <string> 

  // to control debug output from tcl
  // #include "kernel/jafarDebug.hpp"

#include "jmath/ublasExtra.hpp"

#include "jmath/random.hpp"
#include "jmath/constant.hpp"
#include "jmath/angle.hpp"
#include "jmath/gaussianVector.hpp"

  //#include "jmath/delaunay.hpp"

%}

%include "std_common.i"

// to wrap std::string
%include "std_string.i" 

%include "jmathException.i"

// to control debug output from tcl
// %include "kernel/jafarDebug.hpp"

%include "jmath/jblas.i"

/*
 * wrapped headers 
 */ 

%include "jmathTools.i"
%include "jmath/ublasExtra.hpp"

// %template(create_vec) jafar::jmath::createVector<jblas::vec, double>; // deprecated
%template(setSizeValue) jafar::jmath::setSizeValue<jblas::vec>;
%template(setValue) jafar::jmath::setValueVec<jblas::vec>;
%template(print) jafar::jmath::print<jblas::vec>;

%template(print) jafar::jmath::print<jblas::vec_range>;

%template(setValue) jafar::jmath::setValueVec<jblas::vec3>;
%template(print) jafar::jmath::print<jblas::vec3>;

//%template(create_mat) jafar::jmath::createMatrix<jblas::mat, double>; // deprecated
%template(setSizeValue) jafar::jmath::setSizeValue<jblas::mat>;
%template(setValue) jafar::jmath::setValueMat<jblas::mat>;
%template(print) jafar::jmath::print<jblas::mat>;
%template(prettyPrint) jafar::jmath::prettyFormat<jblas::mat>;

%template(setSizeValue) jafar::jmath::setSizeValue<jblas::mat44>;
%template(setValue) jafar::jmath::setValueMat<jblas::mat44>;
%template(print) jafar::jmath::print<jblas::mat44>;
%template(prettyPrint) jafar::jmath::prettyFormat<jblas::mat44>;

// %template(create_sym_mat) jafar::jmath::createMatrix<jblas::sym_mat, double>; // deprecated
//%template(setSizeValue) jafar::jmath::setSizeValue<jblas::sym_mat>;
//%template(setValue) jafar::jmath::setValueMat<jblas::sym_mat>;
%template(print) jafar::jmath::print<jblas::sym_mat>;
%template(prettyPrint) jafar::jmath::prettyFormat<jblas::sym_mat>;

%template(print) jafar::jmath::print<jblas::sym_mat_range>;
%template(prettyPrint) jafar::jmath::prettyFormat<jblas::sym_mat_range>;

%template(max) jafar::jmath::MatrixTools::max<jblas::mat>;
%template(max) jafar::jmath::MatrixTools::max<jblas::sym_mat>;
%template(trace) jafar::jmath::MatrixTools::trace<jblas::mat>;
%template(trace) jafar::jmath::MatrixTools::trace<jblas::sym_mat>;
//%template(covToEllipsoid) jafar::jmath::MatrixTools::covToEllipsoid<jblas::sym_mat>;


/* random */

%include "jmath/random.hpp"

// %template(EulerT3D_toFrame) jafar::jmath::EulerT3D::toFrame<jblas::vec, jblas::vec, jblas::vec>;
// %template(EulerT3D_toFrameJac) jafar::jmath::EulerT3D::toFrameJac<jblas::vec, jblas::vec>;
// %template(EulerT3D_fromFrame) jafar::jmath::EulerT3D::fromFrame<jblas::vec, jblas::vec, jblas::vec>;
// %template(EulerT3D_fromFrameJac) jafar::jmath::EulerT3D::fromFrameJac<jblas::vec, jblas::vec>;
// %template(EulerT3D_composeFrame) jafar::jmath::EulerT3D::composeFrame<jblas::vec, jblas::vec, jblas::vec>;
// %template(EulerT3D_composeFrameJac) jafar::jmath::EulerT3D::composeFrameJac<jblas::vec, jblas::vec>;

%include "jmath/constant.hpp"
%include "jmath/angle.hpp"

%include "jmath/gaussianVector.hpp"
%template(print) jafar::jmath::print<jafar::jmath::GaussianVector>;
%template(print) jafar::jmath::print<jafar::jmath::WeightedGaussianVector>;

// %include "jmath/delaunay.hpp"
