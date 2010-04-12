/* $Id$ */

/** swig interface file for module correl.
 *
 * \file correl.i
 * \ingroup correl
 */      

%module correl

%{
/* ruby defines ALLOC which conflicts with boost */ 
#undef ALLOC
  
/* 
   * headers necessary to compile the wrapper
   */

#include "jafarConfig.h"

// using namespace jafar::correl;

%}

%include "jafar.i"
%include "correlException.i"

/*
 * headers to be wrapped goes here
 */

// %include "correlTools.i"
// instantiate some print functions
// replace "Type" with appropriate class name
// %template(print) jafar::correl::print<jafar::correl::Type>;


