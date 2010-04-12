# $Id$ #

#
# User part of the makefiles
# module correl
# 

# module version 
MODULE_VERSION = 0
MODULE_REVISION = 1 

# modules dependencies
REQUIRED_MODULES = kernel image
OPTIONAL_MODULES = 

# external libraries dependencies
REQUIRED_EXTLIBS = 
OPTIONAL_EXTLIBS = 

# LDFLAGS +=
LIBS += -lkernel -limage

# CPPFLAGS += -DJFR_NDEBUG
CPPFLAGS += $(BOOST_CPPFLAGS) $(OPENCV_CPPFLAGS)

CXXFLAGS += -g -ggdb -Wall




