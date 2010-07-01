# $Id$ #

build_jafar_module(gdhe
  VERSION 0
  REVISION 1 
  REQUIRED_MODULES kernel 
  #OPTIONAL_MODULES 
  REQUIRED_EXTLIBS gdhe
  #OPTIONAL_EXTLIBS 
  #CPPFLAGS += -DNDEBUG
  CXXFLAGS -g -O0
  #CPPFLAGS_MODULE -Wno-long-long -pedantic -pedantic-errors
)
# $Id$ #

