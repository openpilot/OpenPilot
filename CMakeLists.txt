# $Id: CMakeLists.txt 4492 2010-05-31 08:56:24Z nksallem $ #

#
# User part of the makefiles
# module rtslam
# 

build_jafar_module(rtslam
  VERSION 0
  REVISION 1 

  REQUIRED_MODULES kernel image jmath correl
  OPTIONAL_MODULES qdisplay gdhe dseg
# for robotpkg generation:
#  REQUIRED_MODULES kernel image jmath correl qdisplay gdhe

  REQUIRED_EXTLIBS boost_thread boost_system boost_filesystem boost_regex
  OPTIONAL_EXTLIBS qt4 viam MTI posterLib
# for robotpkg generation:
#  REQUIRED_EXTLIBS boost_thread boost_system boost_filesystem boost_regex gdhe qt4 viam MTI posterLib opencv

#  CXXFLAGS -g -ggdb -O0 -Wall
#  CXXFLAGS -O2 -g -DJFR_NDEBUG -DNDEBUG -DBOOST_UBLAS_NDEBUG -Wall -pthread

#  ROBOTPKG_CATEGORY localization
  ROBOTPKG_CATEGORY wip
  SHORT_DESCRIPTION "Jafar module to do fast and generic EKF Slam."

)
