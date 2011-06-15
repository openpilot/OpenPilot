# $Id$ #

build_jafar_module(gdhe
  VERSION 0
  REVISION 1 
  REQUIRED_MODULES kernel jmath
  #OPTIONAL_MODULES 
  REQUIRED_EXTLIBS gdhe boost_sandbox boost_system boost_thread boost_filesystem
  #OPTIONAL_EXTLIBS 
  #CXXFLAGS
  #CPPFLAGS_MODULE -Wno-long-long -pedantic -pedantic-errors
#  ROBOTPKG_CATEGORY graphics
  ROBOTPKG_CATEGORY wip
  SHORT_DESCRIPTION "Jafar module to talk to a gdhe server in c++"
)
# $Id$ #

