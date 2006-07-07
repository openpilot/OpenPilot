/* $Id$ */

/** swig/tcl interface file for module jmath.
 *
 * \file jmath.i
 */      

%module jmath

%{

#include <string> 

#include "jafarConfig.h"

#include "kernel/keyValueFile.hpp"
#include "jmath/ublasExtra.hpp"

#include "jmath/random.hpp"
#include "jmath/angle.hpp"
#include "jmath/gaussianVector.hpp"
#include "jmath/pca.hpp"

#include "jmath/linearLeastSquares.hpp"

%}

%import "jafarConfig.h"

%include "std_common.i"

%include "std_string.i" 

%include "jmathException.i"

%include "kernel/keyValueFile.hpp"

%include "jmath/jblas.i"

/*
 * wrapped headers 
 */ 

%include "jmathTools.i"
%include "jmath/ublasExtra.hpp"

%template(setSizeValue) jafar::jmath::setSizeValue<jblas::vec>;
%template(setValue) jafar::jmath::setValueVec<jblas::vec>;
%template(print) jafar::jmath::print<jblas::vec>;

%template(print) jafar::jmath::print<jblas::vec_range>;

%template(setValue) jafar::jmath::setValueVec<jblas::vec2>;
%template(print) jafar::jmath::print<jblas::vec2>;

%template(setValue) jafar::jmath::setValueVec<jblas::vec3>;
%template(print) jafar::jmath::print<jblas::vec3>;

%template(normalize) jafar::jmath::ublasExtra::normalize<jblas::vec>;

%template(setSizeValue) jafar::jmath::setSizeValue<jblas::mat>;
%template(setValue) jafar::jmath::setValueMat<jblas::mat>;
%template(print) jafar::jmath::print<jblas::mat>;
%template(prettyPrint) jafar::jmath::ublasExtra::prettyFormat<jblas::mat>;
%template(lu_inv) jafar::jmath::ublasExtra::lu_inv<jblas::mat, jblas::mat>;
%template(lu_det) jafar::jmath::ublasExtra::lu_det<jblas::mat>;

%template(setSizeValue) jafar::jmath::setSizeValue<jblas::mat33>;
%template(setValue) jafar::jmath::setValueMat<jblas::mat33>;
%template(print) jafar::jmath::print<jblas::mat33>;
%template(prettyPrint) jafar::jmath::ublasExtra::prettyFormat<jblas::mat33>;

%template(setSizeValue) jafar::jmath::setSizeValue<jblas::mat44>;
%template(setValue) jafar::jmath::setValueMat<jblas::mat44>;
%template(print) jafar::jmath::print<jblas::mat44>;
%template(prettyPrint) jafar::jmath::ublasExtra::prettyFormat<jblas::mat44>;

%template(setSizeValue) jafar::jmath::setSizeValue<jblas::sym_mat>;
%template(setValue) jafar::jmath::setValueMat<jblas::sym_mat>;
%template(print) jafar::jmath::print<jblas::sym_mat>;
%template(prettyPrint) jafar::jmath::ublasExtra::prettyFormat<jblas::sym_mat>;
%template(lu_inv) jafar::jmath::ublasExtra::lu_inv<jblas::sym_mat, jblas::sym_mat>;
%template(lu_det) jafar::jmath::ublasExtra::lu_det<jblas::sym_mat>;

%template(print) jafar::jmath::print<jblas::sym_mat_range>;
%template(prettyPrint) jafar::jmath::ublasExtra::prettyFormat<jblas::sym_mat_range>;

%template(max) jafar::jmath::ublasExtra::max<jblas::mat>;
%template(max) jafar::jmath::ublasExtra::max<jblas::sym_mat>;
%template(trace) jafar::jmath::ublasExtra::trace<jblas::mat>;
%template(trace) jafar::jmath::ublasExtra::trace<jblas::sym_mat>;



%include "jmath/random.hpp"

%include "jmath/angle.hpp"

%include "jmath/gaussianVector.hpp"
%template(print) jafar::jmath::print<jafar::jmath::GaussianVector>;
%template(print) jafar::jmath::print<jafar::jmath::WeightedGaussianVector>;

%include "jmath/pca.hpp"

%include "jmath/linearLeastSquares.hpp"

