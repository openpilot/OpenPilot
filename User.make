# $Id$ #

#
# User part of the makefiles
# module rtslam
# 

# module version 
MODULE_VERSION = 0
MODULE_REVISION = 1 

# modules dependencies
REQUIRED_MODULES = kernel jmath image correl
OPTIONAL_MODULES = qdisplay

# external libraries dependencies
REQUIRED_EXTLIBS = boost_sandbox opencv 
OPTIONAL_EXTLIBS = qt4 viam

# LDFLAGS +=
LIBS += -lkernel -ljmath -limage -lqdisplay -lcorrel -lviam

CPPFLAGS += $(OPENCV_CPPFLAGS) $(QT4_CPPFLAGS) $(BOOST_CPPFLAGS) $(BOOST_SANDBOX_CPPFLAGS) $(VIAM_CPPFLAGS) -I$(ROBOTPKG_BASE)/include

# debug:
#CXXFLAGS += -g -O0 -ggdb -Wall -pthread

# release:
CXXFLAGS += -Wall -pthread -O2
CPPFLAGS += -DJFR_NDEBUG -DNDEBUG
