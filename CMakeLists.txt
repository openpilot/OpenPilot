# $Id$ #

#
# User part of the makefiles
# module correl
# 

jafar_build_module(correl
  VERSION 0
  REVISION 1 
  REQUIRED_MODULES kernel image jmath
  #OPTIONAL_MODULES 
  REQUIRED_EXTLIBS opencv
  #OPTIONAL_EXTLIBS 
  # LDFLAGS +=
  # CPPFLAGS +-DJFR_NDEBUG
  CXXFLAGS +-g -ggdb -Wall
)



