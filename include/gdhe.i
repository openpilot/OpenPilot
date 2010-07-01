/* $Id$ */

/** swig interface file for module gdhe.
 *
 * \file gdhe.i
 * \ingroup gdhe
 */      

%module gdhe

%{
/* ruby defines ALLOC which conflicts with boost */ 
#undef ALLOC
  
/* 
   * headers necessary to compile the wrapper
   */

#include "jafarConfig.h"

// using namespace jafar::gdhe;

%}

%include "jafar.i"
%include "gdheException.i"

/*
 * headers to be wrapped goes here
 */

// %include "gdheTools.i"
// instantiate some print functions
// replace "Type" with appropriate class name
// %template(print) jafar::gdhe::print<jafar::gdhe::Type>;


