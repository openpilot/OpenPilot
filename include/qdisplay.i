/* $Id$ */

/** swig interface file for module qdisplay.
 *
 * \file qdisplay.i
 * \ingroup qdisplay
 */      

%module qdisplay

%{
/* ruby defines ALLOC which conflicts with boost */ 
#undef ALLOC
  
/* 
   * headers necessary to compile the wrapper
   */

#include "jafarConfig.h"
#include "qdisplay/Viewer.hpp"
#include "qdisplay/ImageItem.hpp"
// using namespace jafar::qdisplay;

%}

%include "jafar.i"
%include "qdisplayException.i"
%include "qdisplay/Viewer.hpp"
%include "qdisplay/ImageItem.hpp"

/*
 * headers to be wrapped goes here
 */

// %include "qdisplayTools.i"
// instantiate some print functions
// replace "Type" with appropriate class name
// %template(print) jafar::qdisplay::print<jafar::qdisplay::Type>;


