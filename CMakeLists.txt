# $Id$ #

build_jafar_module(jmath
  VERSION 0
  REVISION 2
  REQUIRED_MODULES kernel 
  #OPTIONAL_MODULES 
  REQUIRED_EXTLIBS lapack boost_sandbox
  OPTIONAL_EXTLIBS flann
  CPPFLAGS -DBOOST_UBLAS_USE_EXCEPTIONS
  #CXXFLAGS
  #CPPFLAGS_MODULE
  ROBOTPKG_CATEGORY math
  SHORT_DESCRIPTION "Jafar module that defines some common mathematical tools: vector and matrix manipulation, random number generation,..."
)
