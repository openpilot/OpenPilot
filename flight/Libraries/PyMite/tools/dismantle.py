#!/usr/bin/env python

# This file is Copyright 2003, 2006, 2007, 2009, 2010 Dean Hall.
#
# This file is part of the Python-on-a-Chip program.
# Python-on-a-Chip is free software: you can redistribute it and/or modify
# it under the terms of the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1.
# 
# Python-on-a-Chip is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# A copy of the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1
# is seen in the file COPYING in this directory.

"""
Dismantles a .py file by compiling it
into a code object and recursively dismantling
and disassembling the code object.
"""

__usage__ = """Usage:
    dismantle.py sourcefilename.py
"""

## @file
#  @copybrief dismantle

## @package dismantle
#  @brief Dismantles a .py file by compiling it
#  into a code object and recursively dismantling
#  and disassembling the code object.


STRINGTOOLONG = 32


import dis, sys, types, py_compile, time


def dismantle_file(fn):
    """Dismantles the .py file, fn. Returns the root code object.
    """

    #create a code object
    f = open(fn)
    source = f.read()
    f.close()
    return dismantle(source, fn)


def dismantle(source, fn=""):

    # If no filename given, just dismantle source, skip magic and ignore
    if fn == "":
        magic = 0
        ignore = 0
        fn = "fn"
        pyc = ""

    else:
        # Compile to .pyc and open
        py_compile.compile(fn)
        f = open(fn + 'c','rb')
        pyc = f.read()
        f.close()

        # Check for magic number
        magic = int((ord(pyc[0])      ) | (ord(pyc[1]) <<  8) |
                    (ord(pyc[2]) << 16) | (ord(pyc[3]) << 24))

        # Grab the next 4 bytes (don't know what they do)
        ignore = int((ord(pyc[4])      ) | (ord(pyc[5]) <<  8) |
                     (ord(pyc[6]) << 16) | (ord(pyc[7]) << 24))

    code = compile(source, fn, "exec")

    # Print header
    print "BEGIN DISMANTLE"
    print "date:           \t", time.ctime(time.time())
    print "src file:       \t", fn
    print "src file size:  \t", len(source), "bytes"
    print "pyc file size:  \t", len(pyc), "bytes"
    print
    print "magic:          \t0x%08x" % magic
    print "ignore:         \t0x%08x" % ignore
    print

    # Recurse into the code object
    rdismantle(code)

    print "END DISMANTLE"
    return code


def rdismantle(co, parent = None):
    """Dismantles the code object, co.  Prints the co_* field values and
    the co_code disassembly for each code object in the file and recurses
    into any code objects in the constant pool.
    """

    # Create full name
    if parent:
        fullname = parent + "." + co.co_name
    else:
        fullname = co.co_name

    # Print object fields and values
    print "fullname:       \t", fullname
    print " co_name:       \t", co.co_name
    print " co_filename:   \t", co.co_filename
    print " co_firstlineno:\t", co.co_firstlineno
    print " co_flags:      \t0x%04x"  % co.co_flags
    print " co_stacksize:  \t", co.co_stacksize
    print " co_lnotab[%3d]:\t%s" % \
          (len(co.co_lnotab), repr(co.co_lnotab[:8]))
    print " co_argcount:   \t", co.co_argcount
    print " co_nlocals:    \t", co.co_nlocals
    print " co_freevars:   \t", co.co_freevars
    print " co_cellvars:   \t", co.co_cellvars

    # Print vital compound components
    tabspacing = "\t\t"

    print " co_varnames:"
    i = 0
    for item in co.co_varnames:
        print tabspacing, i, ":\t", item
        i += 1

    print " co_names:       "
    i = 0
    for item in co.co_names:
        print tabspacing, i, ":\t", item
        i += 1

    print " co_consts:      "
    i = 0
    for item in co.co_consts:
        if type(item) == types.StringType and \
           len(item) > STRINGTOOLONG:
                print tabspacing, i, ":\t", repr(item[:STRINGTOOLONG]), "..."
        else:
            print tabspacing, i, ":\t", repr(item)
        i += 1

    # Print disassembly
    print " co_code:"
    dis.dis(co)
    print "\n"

    # Dismantle code objects in constant pool
    for obj in co.co_consts:
        if type(obj) == types.CodeType:
            rdismantle(obj, fullname)
    return


def main():
    """Dismantles the source file given as an arg.
    """

    if len(sys.argv) == 2:
        return dismantle_file(sys.argv[1])
    else:
        print __usage__


if __name__ == "__main__":
    main()

