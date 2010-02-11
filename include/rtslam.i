/* $Id$ */

/** swig interface file for module rtslam.
 *
 * \file rtslam.i
 * \ingroup rtslam
 */      

%module rtslam

%{
/* ruby defines ALLOC which conflicts with boost */ 
#undef ALLOC
  
/* 
   * headers necessary to compile the wrapper
   */

#include "jafarConfig.h"

// using namespace jafar::rtslam;

%}

%include "jafar.i"
%include "rtslamException.i"

/*
 * headers to be wrapped goes here
 */

// %include "rtslamTools.i"
// instantiate some print functions
// replace "Type" with appropriate class name
// %template(print) jafar::rtslam::print<jafar::rtslam::Type>;


