# $Id$ #

# module jmath

# module version 
MODULE_VERSION = 0
MODULE_REVISION = 1 

# modules dependencies
USE_MODULES = kernel

# to add custom libraries 
# concider also modifying .config/User.m4 to check for these libs at
# configure time

LIBS += -lkernel

CPPFLAGS += $(BOOST_CPPFLAGS) -DBOOST_UBLAS_USE_EXCEPTIONS

# ttl library
#CPPFLAGS += -I/home/thomas/usr/local/ttl/include -DHAVE_TTL
#LIBS += -ldstrhe -lutils
#LDFLAGS += -L/home/thomas/usr/local/ttl/lib

#CPPFLAGS += -DNDEBUG

CXXFLAGS += -g -ggdb -Wall




