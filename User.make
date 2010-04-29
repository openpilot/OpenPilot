# $Id$ #

#
# User part of the makefiles
# module rtslam
# 

# module version 
MODULE_VERSION = 0
MODULE_REVISION = 1 

# modules dependencies
REQUIRED_MODULES = kernel jmath
OPTIONAL_MODULES = 

# external libraries dependencies
REQUIRED_EXTLIBS = boost_sandbox 
OPTIONAL_EXTLIBS = 

# LDFLAGS +=
LIBS += -lkernel -ljmath 

# CPPFLAGS += -DJFR_NDEBUG
CPPFLAGS += $(BOOST_CPPFLAGS) $(BOOST_SANDBOX_CPPFLAGS) 

CXXFLAGS += -g -O0 -ggdb -Wall
