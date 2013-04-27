#
# PyMite - A flyweight Python interpreter for 8-bit and larger microcontrollers.
# Copyright 2002 Dean Hall.  All rights reserved.
# PyMite is offered through one of two licenses: commercial or open-source.
# See the LICENSE file at the root of this package for licensing details.
#

#
# Runs the interactive interpreter
#

print "Hello"

import ipm
ipm.ipm(globals())

print "Good-bye"