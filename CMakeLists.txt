# $Id$ #

#
# User part of the makefiles
# module qdisplay
# 

build_jafar_module(qdisplay
  VERSION 0
  REVISION 1 
  REQUIRED_MODULES kernel image jmath
  #OPTIONAL_MODULES 
  REQUIRED_EXTLIBS qt4 boost_thread boost_sandbox
  #OPTIONAL_EXTLIBS 
  #CXXFLAGS
  #CPPFLAGS
#  ROBOTPKG_CATEGORY graphics
  ROBOTPKG_CATEGORY wip
  SHORT_DESCRIPTION "Jafar module to display objects of type jafar::image::JfrImage."
)
