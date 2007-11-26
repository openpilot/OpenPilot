# $Id$ #

# module jmath

# module version 
MODULE_VERSION = 0
MODULE_REVISION = 1 

# modules dependencies
REQUIRED_MODULES = kernel 
OPTIONAL_MODULES = 

# external libraries dependencies
REQUIRED_EXTLIBS = 
OPTIONAL_EXTLIBS = lapack boost_sandbox


LDFLAGS += $(BOOST_LDFLAGS) $(LAPACK_LDFLAGS) 
LIBS += -lkernel $(LAPACK_LIBS)

CPPFLAGS += $(BOOST_CPPFLAGS) $(BOOST_SANDBOX_CPPFLAGS) \
	              -DBOOST_UBLAS_USE_EXCEPTIONS

# ttl library
#CPPFLAGS += -I/home/thomas/usr/local/ttl/include -DHAVE_TTL
#LIBS += -ldstrhe -lutils
#LDFLAGS += -L/home/thomas/usr/local/ttl/lib

#CPPFLAGS += -DNDEBUG

CXXFLAGS += -g -ggdb -Wall
CPPFLAGS_MODULE = -pedantic -pedantic-errors -Werror 

