# $Id$ #

#
# User part of the makefiles
# module correl
# 

build_jafar_module(correl
  VERSION 0
  REVISION 1 
  REQUIRED_MODULES kernel image jmath
  #OPTIONAL_MODULES 
  REQUIRED_EXTLIBS opencv
  #OPTIONAL_EXTLIBS 
  #LDFLAGS 
  #CPPFLAGS
  #CXXFLAGS
  #CPPFLAGS_MODULE
#  ROBOTPKG_CATEGORY image
  ROBOTPKG_CATEGORY wip
  SHORT_DESCRIPTION "Jafar module with image correlation functions"
)



