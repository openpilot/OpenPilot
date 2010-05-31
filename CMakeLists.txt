# $Id$ #

build_jafar_module(jmath
  VERSION 0
  REVISION 1 
  REQUIRED_MODULES kernel 
  #OPTIONAL_MODULES 
  REQUIRED_EXTLIBS lapack boost_sandbox
  #OPTIONAL_EXTLIBS 
  CPPFLAGS -DBOOST_UBLAS_USE_EXCEPTIONS
  #CPPFLAGS += -DNDEBUG
  CXXFLAGS -g -ggdb -Wall
  #CPPFLAGS_MODULE -Wno-long-long -pedantic -pedantic-errors
)
