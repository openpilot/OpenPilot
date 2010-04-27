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
#include "correl/explorer.hpp"

using namespace jafar;
//using namespace jafar::correl;

%}

%include "jafar.i"
%include "correlException.i"
%include "correl/explorer.hpp"

/*
 * headers to be wrapped goes here
 */

// %include "correlTools.i"
// instantiate some print functions
// replace "Type" with appropriate class name
// %template(print) jafar::correl::print<jafar::correl::Type>;


%template(ExplorerZncc) jafar::correl::Explorer<jafar::correl::Zncc>;
/*
%extend jafar::correl::Explorer<Correl> {
  static double exploreTranslationSwig(image::Image const& im1, image::Image& im2, int xmin, int xmax, int xstep, int ymin, int ymax, int ystep, CvPoint2D32f &res, float const* weightMatrix = NULL);) {
    return exploreTranslation(&im1, &im2, xmin, xmax, xstep, ymin, ymax, ystep, res.x, res.y, weightMatrix);
//    return jafar::correl::Explorer<Correl>::exploreTranslation(&im1, &im2, xmin, xmax, xstep, ymin, ymax, ystep, res.x, res.y, weightMatrix);
  }
};
*/