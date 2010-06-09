# $Id$ #

#
# User part of the makefiles
# module qdisplay
# 

# module version 
MODULE_VERSION = 0
MODULE_REVISION = 1 

# modules dependencies
REQUIRED_MODULES = kernel image jmath
OPTIONAL_MODULES = 

# external libraries dependencies
REQUIRED_EXTLIBS = qt4
OPTIONAL_EXTLIBS = 

LDFLAGS += $(BOOST_LDFLAGS)
LIBS += -lkernel -limage $(QT4_LIBS) $(LAPACK_LIBS) $(BOOST_THREAD_LIBS)

# CPPFLAGS += -DJFR_NDEBUG
CPPFLAGS += $(BOOST_CPPFLAGS) $(QT4_CPPFLAGS) $(OPENCV_CPPFLAGS)  $(BOOST_SANDBOX_CPPFLAGS)

CXXFLAGS += -g -ggdb -Wall
#CPPFLAGS_MODULE = -Werror
