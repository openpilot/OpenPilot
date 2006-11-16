# $Id$ #

#
# User part of the makefiles
# module qdisplay
# 

# module version 
MODULE_VERSION = 0
MODULE_REVISION = 1 

# modules dependencies
REQUIRED_MODULES = kernel image
OPTIONAL_MODULES = datareader

# external libraries dependencies
REQUIRED_EXTLIBS = qt4
OPTIONAL_EXTLIBS = 

# LDFLAGS +=
LIBS += -lkernel -limage  $(QT4_LIBS)

# CPPFLAGS += -DJFR_NDEBUG
CPPFLAGS += $(BOOST_CPPFLAGS) $(QT4_CPPFLAGS) $(OPENCV_CPPFLAGS)

CXXFLAGS += -g -ggdb -Wall




