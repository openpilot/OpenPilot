/* $Id$ */

/** swig/tcl interface file for module jmath.
 *
 * \file jmath.i
 */      

%module jmath

%{

/* ruby defines ALLOC which conflicts with boost */ 
#undef ALLOC

#include <string> 

#include "jafarConfig.h"

#include "kernel/keyValueFile.hpp"
#include "jmath/ublasExtra.hpp"

#include "jmath/random.hpp"
#include "jmath/angle.hpp"
#include "jmath/gaussianVector.hpp"
#include "jmath/pca.hpp"

#include "jmath/linearLeastSquares.hpp"
#include "jmath/linearSolvers.hpp"
#include "jmath/misc.hpp"
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
%include "jmath/misc.hpp"

%template(sqr) jafar::jmath::sqr<double>;

%template(setSizeValue) jafar::jmath::setSizeValue<jblas::vec>;
%template(setValue) jafar::jmath::setValueVec<jblas::vec>;
%template(print) jafar::jmath::print<jblas::vec>;

%template(print) jafar::jmath::print<jblas::vec_range>;

%template(setValue) jafar::jmath::setValueVec<jblas::vec2>;
%template(print) jafar::jmath::print<jblas::vec2>;

%template(setValue) jafar::jmath::setValueVec<jblas::vec3>;
%template(print) jafar::jmath::print<jblas::vec3>;

%template(setValue) jafar::jmath::setValueVec<jblas::vec4>;
%template(print) jafar::jmath::print<jblas::vec4>;

%template(setValue) jafar::jmath::setValueVec<jblas::vec6>;
%template(print) jafar::jmath::print<jblas::vec6>;

%template(setValue) jafar::jmath::setValueVec<jblas::veci4>;
%template(print) jafar::jmath::print<jblas::veci4>;

%template(normalize) jafar::jmath::ublasExtra::normalize<jblas::vec>;

%template(setSizeValue) jafar::jmath::setSizeValue<jblas::mat>;
%template(setValue) jafar::jmath::setValueMat<jblas::mat>;
%template(print) jafar::jmath::print<jblas::mat>;
%template(prettyPrint) jafar::jmath::ublasExtra::prettyFormat<jblas::mat>;
%template(lu_inv) jafar::jmath::ublasExtra::lu_inv<jblas::mat, jblas::mat>;
%template(lu_det) jafar::jmath::ublasExtra::lu_det<jblas::mat>;
//%template(svd_inv) jafar::jmath::ublasExtra::svd_inv<jblas::mat,jblas::mat>;

%template(setSizeValue) jafar::jmath::setSizeValue<jblas::mat22>;
%template(setValue) jafar::jmath::setValueMat<jblas::mat22>;
%template(print) jafar::jmath::print<jblas::mat22>;
%template(prettyPrint) jafar::jmath::ublasExtra::prettyFormat<jblas::mat22>;

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

%template(setValue) jafar::jmath::setValueMat<jblas::sym_mat22>;
%template(print) jafar::jmath::print<jblas::sym_mat22>;
%template(setValue) jafar::jmath::setValueMat<jblas::sym_mat33>;
%template(print) jafar::jmath::print<jblas::sym_mat33>;
%template(setValue) jafar::jmath::setValueMat<jblas::sym_mat44>;
%template(print) jafar::jmath::print<jblas::sym_mat44>;

%template(max) jafar::jmath::ublasExtra::max<jblas::mat>;
%template(max) jafar::jmath::ublasExtra::max<jblas::sym_mat>;
%template(trace) jafar::jmath::ublasExtra::trace<jblas::mat>;
%template(trace) jafar::jmath::ublasExtra::trace<jblas::sym_mat>;

/* wrap operation to be able to do operations on vector/matrix in jafar */
%template(add) jafar::jmath::add<jblas::vec>;
%template(add) jafar::jmath::add<jblas::vec2>;
%template(add) jafar::jmath::add<jblas::vec3>;
%template(add) jafar::jmath::add<jblas::vec4>;
%template(sub) jafar::jmath::sub<jblas::vec>;
%template(sub) jafar::jmath::sub<jblas::vec2>;
%template(sub) jafar::jmath::sub<jblas::vec3>;
%template(sub) jafar::jmath::sub<jblas::vec4>;
%template(mul) jafar::jmath::scalmul<jblas::vec,int>;
%template(mul) jafar::jmath::scalmul<jblas::vec,double>;
%template(div) jafar::jmath::div<jblas::vec, double>;
%template(div) jafar::jmath::div<jblas::vec, int>;
%template(assignVec3) jafar::jmath::assignVec3<jblas::vec,jblas::vec3>;

/* matrix */
%template(add) jafar::jmath::add<jblas::mat>;
%template(sub) jafar::jmath::sub<jblas::mat>;
%template(sub) jafar::jmath::sub<jblas::mat33>;
%template(mul) jafar::jmath::scalmul<jblas::mat,int>;
%template(mul) jafar::jmath::scalmul<jblas::mat,double>;
%template(mul) jafar::jmath::vecmatmul<jblas::mat,jblas::vec>;
%template(mul) jafar::jmath::vecmatmul<jblas::sym_mat,jblas::vec>;
%template(mul) jafar::jmath::vecmatmul<jblas::mat33,jblas::vec3>;
%template(mul) jafar::jmath::vecmatmul<jblas::mat33,jblas::vec>;
%template(mul) jafar::jmath::vecmatmul<jblas::mat44,jblas::vec>;
%template(mul) jafar::jmath::matmatmul<jblas::mat22,jblas::mat22,jblas::mat22>;
%template(mul) jafar::jmath::matmatmul<jblas::mat33,jblas::mat33,jblas::mat33>;
%template(mul) jafar::jmath::matmatmul<jblas::mat,jblas::mat,jblas::mat>;
%template(mul) jafar::jmath::matmatmul<jblas::sym_mat,jblas::mat,jblas::mat>;
%template(mul) jafar::jmath::matmatmul<jblas::sym_mat,jblas::sym_mat,jblas::sym_mat>;
%template(mul) jafar::jmath::matmatmul<jblas::mat,jblas::sym_mat,jblas::mat>;
%template(inner_prod) jafar::jmath::inner_prod<jblas::vec>;
%template(prod_xt_P_x) jafar::jmath::ublasExtra::prod_xt_P_x<jblas::sym_mat,jblas::vec>;
%template(div) jafar::jmath::div<jblas::mat, double>;
%template(div) jafar::jmath::div<jblas::mat, float>;
%template(div) jafar::jmath::div<jblas::mat, int>;
%template(div) jafar::jmath::div<jblas::mat33, double>;
%template(div) jafar::jmath::div<jblas::mat33, float>;
%template(inv) jafar::jmath::inv<jblas::mat44>;
%template(inv) jafar::jmath::inv<jblas::mat>;
%template(inv) jafar::jmath::inv<jblas::sym_mat>;
%template(trans) jafar::jmath::trans<jblas::vec>;
%template(trans) jafar::jmath::trans<jblas::mat>;
%template(trans) jafar::jmath::trans<jblas::mat22>;
%template(assignMat) jafar::jmath::assignMat<jblas::sym_mat,jblas::mat>;

%template(getElementAt) jafar::jmath::getElementAt<jblas::vec>;
%template(getElementAt) jafar::jmath::getElementAt<jblas::vec2>;
%template(getElementAt) jafar::jmath::getElementAt<jblas::vec3>;
%template(getElementAt) jafar::jmath::getElementAt<jblas::vec4>;
%template(getElementAt) jafar::jmath::getMatElementAt<jblas::mat>;
%template(getElementAt) jafar::jmath::getMatElementAt<jblas::mat22>;
%template(getElementAt) jafar::jmath::getMatElementAt<jblas::mat33>;
%template(getElementAt) jafar::jmath::getMatElementAt<jblas::mat44>;

%template(setElementAt) jafar::jmath::setElementAt<jblas::vec>;
%template(setElementAt) jafar::jmath::setElementAt<jblas::vec2>;
%template(setElementAt) jafar::jmath::setElementAt<jblas::vec3>;
%template(setElementAt) jafar::jmath::setElementAt<jblas::vec4>;

%include "jmath/random.hpp"

%include "jmath/angle.hpp"

%include "jmath/gaussianVector.hpp"
%template(print) jafar::jmath::print<jafar::jmath::GaussianVector>;
%template(print) jafar::jmath::print<jafar::jmath::WeightedGaussianVector>;

%include "jmath/pca.hpp"

%include "jmath/linearLeastSquares.hpp"

%include "jmath/linearSolvers.hpp"

