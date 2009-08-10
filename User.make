# $Id$ #

# module jmath

# module version 
MODULE_VERSION = 0
MODULE_REVISION = 1 

# modules dependencies
REQUIRED_MODULES = kernel 
OPTIONAL_MODULES = 

# external libraries dependencies
REQUIRED_EXTLIBS = lapack boost_sandbox
OPTIONAL_EXTLIBS = 


LDFLAGS += $(BOOST_LDFLAGS) $(LAPACK_LDFLAGS) 
LIBS += -lkernel $(LAPACK_LIBS)

CPPFLAGS += $(BOOST_CPPFLAGS) $(BOOST_SANDBOX_CPPFLAGS) \
	              -DBOOST_UBLAS_USE_EXCEPTIONS

#CPPFLAGS += -DNDEBUG

CXXFLAGS += -g -ggdb -Wall
CPPFLAGS_MODULE = -Wno-long-long -pedantic -pedantic-errors

