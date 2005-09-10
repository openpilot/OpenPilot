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

# LDFLAGS +=
LIBS += -lkernel
# to use ublas bindings to lapack
#LIBS += -llapack

CPPFLAGS += $(BOOST_CPPFLAGS) -DBOOST_UBLAS_USE_EXCEPTIONS 
# to use ublas bindings
#CPPFLAGS += -I/home/tlemaire/usr/local/boost-sandbox
CPPFLAGS += -DNDEBUG

CXXFLAGS += -g -ggdb -Wall




