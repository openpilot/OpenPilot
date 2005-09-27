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

LDFLAGS += -L/home/thomas/usr/local/ttl/lib

LIBS += -lkernel

# to use ublas bindings to lapack
#LIBS += -llapack

CPPFLAGS += $(BOOST_CPPFLAGS) -DBOOST_UBLAS_USE_EXCEPTIONS
# ttl library
#CPPFLAGS += -I/home/thomas/usr/local/ttl/include -DHAVE_TTL
#TTL_LIBS = -ldstrhe -lutils 
#LIBS += TTL_LIBS

#CPPFLAGS += -DNDEBUG

CXXFLAGS += -g -ggdb -Wall




