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
PyMite Image Creator
====================

Converts Python source files to a PyMite code image library.
Performs code filtering to ensure it will run in PyMite.
Formats the image as a raw binary file or a C file
containing a byte array.

16- and 32-bit values are in LITTLE ENDIAN order.
This matches both Python and the AVR compiler's access to EEPROM.

The order of the images in the output is undetermined.

If the Python source contains a native code declaration
and '--native-file=filename" is specified, the native code
is formatted as C functions and an array of functions and output
to the given filename.  When native functions are present, the user
should specify where the native functions should be placed-- in the
standard library or the user library--using the argument -s or -u,
respectively.
"""

## @file
#  @copybrief pmImgCreator

## @package pmImgCreator
#  @brief PyMite Image Creator
#
#  See the source docstring for more details.

__usage__ = """USAGE:
    pmImgCreator.py -f pmfeaturesfilename [-b|c] [-s|u] [OPTIONS] -o imgfilename file0.py [files...]

    -f <fn> Specify the file containing the PM_FEATURES dict to use
    -b      Generates a raw binary file of the image
    -c      Generates a C file of the image (default)

    -s      Place native functions in the PyMite standard library (default)
    -u      Place native functions in the user library

    OPTIONS:
    --native-file=filename  If specified, pmImgCreator will write a C source
                            file with native functions from the python files.
    --memspace=ram|flash    Sets the memory space in which the image will be
                            placed (default is "ram")
    """


import exceptions, string, sys, types, dis, os, time, getopt, struct, types


################################################################
# CONSTANTS
################################################################

# Exit error codes (from /usr/include/sysexits.h)
EX_USAGE = 64

# remove documentation string from const pool
REMOVE_DOC_STR = False

# Pm obj descriptor type constants
# Must match PmType_e in pm.h
OBJ_TYPE_NON = 0x00     # None
OBJ_TYPE_INT = 0x01     # Signed integer
OBJ_TYPE_FLT = 0x02     # Floating point 32b
OBJ_TYPE_STR = 0x03     # String
OBJ_TYPE_TUP = 0x04     # Tuple (static sequence)
OBJ_TYPE_COB = 0x05     # Code obj
OBJ_TYPE_MOD = 0x06     # Module obj
OBJ_TYPE_CLO = 0x07     # Class obj
OBJ_TYPE_FXN = 0x08     # Funtion obj
OBJ_TYPE_CLI = 0x09     # Class instance
OBJ_TYPE_CIM = 0x0A     # Code image
OBJ_TYPE_NIM = 0x0B     # Native func img
OBJ_TYPE_NOB = 0x0C     # Native func obj
# All types after this never appear in an image

# Number of bytes in a native image (constant)
NATIVE_IMG_SIZE = 4

# Maximum number of objs in a tuple
MAX_TUPLE_LEN = 253

# Maximum number of chars in a string (XXX bytes vs UTF-8 chars?)
MAX_STRING_LEN = 999

# Maximum number of chars in a code img
MAX_IMG_LEN = 32767

# Masks for co_flags (from Python's code.h)
CO_OPTIMIZED = 0x0001
CO_NEWLOCALS = 0x0002
CO_VARARGS = 0x0004
CO_VARKEYWORDS = 0x0008
CO_NESTED = 0x0010
CO_GENERATOR = 0x0020
CO_NOFREE = 0x0040

# String used to ID a native method
NATIVE_INDICATOR = "__NATIVE__"
NATIVE_INDICATOR_LENGTH = len(NATIVE_INDICATOR)

# String name of function table variable
NATIVE_TABLE_NAME = {"std": "std_nat_fxn_table",
                     "usr": "usr_nat_fxn_table"
                    }

# String name to prefix all native functions
NATIVE_FUNC_PREFIX = "nat_"

# Maximum number of locals a native func can have (frame.h)
NATIVE_MAX_NUM_LOCALS = 8

# Issue #51: In Python 2.5, the module identifier changed from '?' to '<module>'
if float(sys.version[:3]) < 2.5:
    MODULE_IDENTIFIER = "?"
else:
    MODULE_IDENTIFIER = "<module>"

# Old #152: Byte to append after the last image in the list
IMG_LIST_TERMINATOR = "\xFF"


################################################################
# GLOBALS
################################################################

# Number of bytes from top of code img to start of consts:
# type, sizelo, sizehi, co_argcount, co_flags, co_stacksize, co_nlocals
CO_IMG_FIXEDPART_SIZE = 7

# PyMite's unimplemented bytecodes (from Python 2.0 through 2.5)
UNIMPLEMENTED_BCODES = [
    "SLICE+1", "SLICE+2", "SLICE+3",
    "STORE_SLICE+0", "STORE_SLICE+1", "STORE_SLICE+2", "STORE_SLICE+3",
    "DELETE_SLICE+0", "DELETE_SLICE+1", "DELETE_SLICE+2", "DELETE_SLICE+3",
    "PRINT_ITEM_TO", "PRINT_NEWLINE_TO",
    "WITH_CLEANUP",
    "EXEC_STMT",
    "END_FINALLY",
    "SETUP_EXCEPT", "SETUP_FINALLY",
    "BUILD_SLICE",
    "CALL_FUNCTION_VAR", "CALL_FUNCTION_KW", "CALL_FUNCTION_VAR_KW",
    "EXTENDED_ARG",
    ]


################################################################
# CLASS
################################################################

class PmImgCreator:
    def __init__(self, pmfeatures_filename):

        # Issue #88: Consolidate HAVE_* platform feature definitions
        # Execute the pmfeatures file to get the features dict
        locs = {}
        execfile(pmfeatures_filename, {}, locs)
        global PM_FEATURES
        PM_FEATURES = locs['PM_FEATURES']
        assert type(PM_FEATURES) == dict

        # Modify some globals based on the platform features
        global CO_IMG_FIXEDPART_SIZE
        global UNIMPLEMENTED_BCODES

        if PM_FEATURES["HAVE_CLOSURES"]:
            CO_IMG_FIXEDPART_SIZE += 1  # [co_nfreevars]

        if PM_FEATURES["HAVE_DEBUG_INFO"]:
            CO_IMG_FIXEDPART_SIZE += 2  # [co_firstlineno]

        if not PM_FEATURES["HAVE_DEL"]:
            UNIMPLEMENTED_BCODES.extend([
                "DELETE_SUBSCR",
                "DELETE_NAME",
                "DELETE_GLOBAL",
                "DELETE_ATTR",
                "DELETE_FAST",
                ])

        if not PM_FEATURES["HAVE_IMPORTS"]:
            UNIMPLEMENTED_BCODES.extend([
                "IMPORT_STAR",
                "IMPORT_FROM",
                ])

        if not PM_FEATURES["HAVE_ASSERT"]:
            UNIMPLEMENTED_BCODES.extend([
                "RAISE_VARARGS",
                ])

        if not PM_FEATURES["HAVE_CLASSES"]:
            UNIMPLEMENTED_BCODES.extend([
                "BUILD_CLASS",
                ])

        # Issue #7: Add support for the yield keyword
        if not PM_FEATURES["HAVE_GENERATORS"]:
            UNIMPLEMENTED_BCODES.extend([
                "YIELD_VALUE",
                ])

        # Issue #44: Add support for the backtick operation (UNARY_CONVERT)
        if not PM_FEATURES["HAVE_BACKTICK"]:
            UNIMPLEMENTED_BCODES.extend([
                "UNARY_CONVERT",
                ])

        # Issue #13: Add support for Python 2.6 bytecodes.
        # The *_TRUE_DIVIDE bytecodes require support for float type
        if not PM_FEATURES["HAVE_FLOAT"]:
            UNIMPLEMENTED_BCODES.extend([
                "BINARY_TRUE_DIVIDE",
                "INPLACE_TRUE_DIVIDE",
                ])

        # Issue #56: Add support for decorators
        if not PM_FEATURES["HAVE_CLOSURES"]:
            UNIMPLEMENTED_BCODES.extend([
                "MAKE_CLOSURE",
                "LOAD_CLOSURE",
                "LOAD_DEREF",
                "STORE_DEREF",
                ])


        self.formatFromExt = {".c": self.format_img_as_c,
                              ".bin": self.format_img_as_bin,
                             }

        # bcode to mnemonic conversion (sparse list of strings)
        bcodes = dis.opname[:]

        # remove invalid bcodes
        for i in range(len(bcodes)):
            if bcodes[i][0] == '<':
                bcodes[i] = None

        # remove unimplmented bcodes
        for bcname in UNIMPLEMENTED_BCODES:
            if bcname in bcodes:
                i = bcodes.index(bcname)
                bcodes[i] = None

        # set class variables
        self.bcodes = bcodes

        # function renames
        self._U8_to_str = chr
        self._str_to_U8 = ord


    def set_options(self,
                    outfn,
                    imgtype,
                    imgtarget,
                    memspace,
                    nativeFilename,
                    infiles,
                   ):
        self.outfn = outfn
        self.imgtype = imgtype
        self.imgtarget = imgtarget
        self.memspace = memspace
        self.nativeFilename = nativeFilename
        self.infiles = infiles

################################################################
# CONVERSION FUNCTIONS
################################################################

    def convert_files(self,):
        """Attempts to convert all source files.
        Creates a dict whose keys are the filenames
        and values are the code object string.
        """
        # init image dict
        imgs = {"imgs": [], "fns": []}

        # init module table and native table
        self.nativemods = []
        self.nativetable = []

        # if creating usr lib, create placeholder in 0th index
        if self.imgtarget == "usr":
            self.nativetable.append((NATIVE_FUNC_PREFIX + "placeholder_func",
                                    "\n    /*\n"
                                    "     * Use placeholder because an index \n"
                                    "     * value of zero denotes the stdlib.\n"
                                    "     * This function should not be called.\n"
                                    "     */\n"
                                    "    PmReturn_t retval;\n"
                                    "    PM_RAISE(retval, PM_RET_EX_SYS);\n"
                                    "    return retval;\n"
                                   ))
            self.nfcount = 1
        else:
            self.nfcount = 0

        # for each src file, convert and format
        for fn in self.infiles:

            # try to compile and convert the file
            co = compile(open(fn).read(), fn, 'exec')
            imgs["fns"].append(fn)
            imgs["imgs"].append(self.co_to_str(co))

        # Append null terminator to list of images
        imgs["fns"].append("img-list-terminator")
        imgs["imgs"].append(IMG_LIST_TERMINATOR)

        self.imgDict = imgs
        return


    def _str_to_U16(self, s):
        """Convert two bytes from a sequence to a 16-bit word.

        The bytes are expected in little endian order.
        LSB first.
        """

        return self._str_to_U8(s[0]) | (self._str_to_U8(s[1]) << 8)


    def _U16_to_str(self, w):
        """Convert the 16-bit word, w, to a string of two bytes.

        The 2 byte string is in little endian order.
        DOES NOT INSERT TYPE BYTE.
        """

        return self._U8_to_str(w & 0xff) + \
               self._U8_to_str((w >> 8) & 0xff)


    def _float_to_str(self, f):
        """Convert the float object, f, to a string of four bytes
        in little-endian order.
        """
        return struct.pack("<f", f)


    def _seq_to_str(self, seq):
        """Convert a Python sequence to a PyMite image.

        The sequence is converted to a tuple of objects.
        This handles both co_consts and co_names.
        This is recursive to handle tuples in the const pool.
        Return string shows type in the leading byte.
        """

        # OPT
        _U8_to_str = self._U8_to_str

        # ensure tuple fits within limits
        assert len(seq) <= MAX_TUPLE_LEN

        # image string init with tuple marker and num elements
        imgstr = _U8_to_str(OBJ_TYPE_TUP) + \
                 _U8_to_str(len(seq))

        # iterate through the sequence of objects
        for i in range(len(seq)):
            obj = seq[i]
            objtype = type(obj)

            # if it is a string
            if objtype == types.StringType:
                # ensure string is not too long
                assert len(obj) <= MAX_STRING_LEN
                # marker, string length, string itself
                imgstr += _U8_to_str(OBJ_TYPE_STR) + \
                          self._U16_to_str(len(obj)) + obj

            # if it is an integer
            elif objtype == types.IntType:
                # marker, int (little endian)
                imgstr += _U8_to_str(OBJ_TYPE_INT) + \
                          _U8_to_str(obj & 0xff) + \
                          _U8_to_str((obj >>  8) & 0xff) + \
                          _U8_to_str((obj >> 16) & 0xff) + \
                          _U8_to_str((obj >> 24) & 0xff)

            # if it is a code object
            elif objtype == types.CodeType:
                #determine if it's native or regular
                if (len(obj.co_consts) > 0 and
                    (type(obj.co_consts[0]) == types.StringType) and
                    (obj.co_consts[0][0:NATIVE_INDICATOR_LENGTH] ==
                    NATIVE_INDICATOR)):
                    imgstr += self.no_to_str(obj)
                else:
                    imgstr += self.co_to_str(obj)

            # if it is a tuple
            elif objtype == types.TupleType:
                imgstr += self._seq_to_str(obj)

            # if it is None
            elif objtype == types.NoneType:
                # marker, none (0)
                imgstr += _U8_to_str(OBJ_TYPE_NON)

            # if it is a float
            elif objtype == types.FloatType and PM_FEATURES["HAVE_FLOAT"]:
                imgstr += _U8_to_str(OBJ_TYPE_FLT) + self._float_to_str(obj)

            # other type?
            else:
                raise exceptions.NotImplementedError(
                          "Unhandled type %s." % objtype)
        return imgstr


    # NOTE: if the total size of the fixed-sized fields changes,
    # be sure to change CO_IMG_FIXEDPART_SIZE above
    def co_to_str(self, co):
        """Converts a Python code object to a PyMite image.

        The code image is relocatable and goes in the device's
        memory. Return string shows type in the leading byte.
        """

        # filter code object elements
        consts, names, code, nativecode = self._filter_co(co)

        # Type and size inserted below

        # Appends various constant-sized fields to the image
        imgstr = self._U8_to_str(co.co_argcount) + \
                 self._U8_to_str(co.co_flags & 0xFF) + \
                 self._U8_to_str(co.co_stacksize) + \
                 self._U8_to_str(co.co_nlocals)

        # Issue #56: Add support for closures
        if PM_FEATURES["HAVE_CLOSURES"]:
            imgstr += self._U8_to_str(len(co.co_freevars))

        # Issue #103: Add debug info to exception reports
        if PM_FEATURES["HAVE_DEBUG_INFO"]:
            imgstr += self._U16_to_str(co.co_firstlineno)

        # Variable length objects
        # Appends names (tuple) to the image
        s = self._seq_to_str(names)
        lennames = len(s)
        imgstr += s

        # Issue #103: Add debug info to exception reports
        lenlnotab = 0
        lenfilename = 0
        if PM_FEATURES["HAVE_DEBUG_INFO"]:

            # Appends line number table (string) to the image
            assert len(co.co_lnotab) <= MAX_STRING_LEN
            s = self._U8_to_str(OBJ_TYPE_STR) + \
                self._U16_to_str(len(co.co_lnotab)) + co.co_lnotab
            lenlnotab = len(s)
            imgstr += s

            # Appends filename (string) to the image
            # fn = os.path.split(co.co_filename)[-1]
            fn = co.co_filename
            assert len(fn) <= MAX_STRING_LEN
            s = self._U8_to_str(OBJ_TYPE_STR) + \
                self._U16_to_str(len(fn) + 1) + fn + '\0'
            lenfilename = len(s)
            imgstr += s

        # Appends consts tuple to the image
        s = self._seq_to_str(consts)
        lenconsts = len(s)
        imgstr += s

        # Issue #56: Add support for closures
        # Appends cellvars tuple to the image
        if PM_FEATURES["HAVE_CLOSURES"]:
            # Lookup the index of the cellvar name in co_varnames; -1 if not in
            l = [-1,] * len(co.co_cellvars)
            for i,name in enumerate(co.co_cellvars):
                if name in co.co_varnames:
                    l[i] = list(co.co_varnames).index(name)
            s = self._seq_to_str(tuple(l))
            lenconsts += len(s)
            imgstr += s

        # Appends bytecode (or native index) to image
        imgstr += code

        size = CO_IMG_FIXEDPART_SIZE + lennames + lenlnotab + lenfilename + \
               lenconsts + len(code)

        # Inserts type and size (skipped earlier)
        imgstr = self._U8_to_str(OBJ_TYPE_CIM) + \
                 self._U16_to_str(size) + \
                 imgstr
        assert len(imgstr) <= MAX_IMG_LEN

        return imgstr


    def no_to_str(self, co):
        """Convert a native code object to a PyMite image.

        The native image is relocatable and goes in the device's
        memory. Return string shows type in the leading byte.
        """

        # filter code object elements
        consts, names, code, nativecode = self._filter_co(co)

        # list of strings to build image

        # set image type byte
        objtype = OBJ_TYPE_NIM

        # Create native image string
        # (type, argcount, funcindex)
        imgstr = (self._U8_to_str(OBJ_TYPE_NIM) +
                  self._U8_to_str(co.co_argcount) +
                  code)

        # ensure string length fits within S16 type
        assert len(imgstr) <= MAX_IMG_LEN

        return imgstr


################################################################
# FILTER FUNCTION
################################################################

    def _filter_co(self, co):
        """Run the Python code obj, co, through various filters.

        Ensure it is compliant with PyMite restrictions.

        Consts filter:
            Ensure num consts is less than 256.
            Replace __doc__ with None if present.

        Flags filter:
            Check co_flags for flags that indicate an unsupported feature
            Supported flags: CO_NOFREE, CO_OPTIMIZED, CO_NEWLOCALS, CO_NESTED,
            Unsupported flags: CO_VARARGS, CO_VARKEYWORDS
            Conditionally supported flags: CO_GENERATOR if HAVE_GENERATORS

        Native code filter:
            If this function has a native indicator,
            extract the native code from the doc string
            and clear the doc string.
            Ensure num args is less or equal to
            NATIVE_MAX_NUM_LOCALS.

        Names/varnames filter:
            Ensure num names is less than 256.
            If co_name is the module identifier replace it with
            the trimmed module name
            otherwise just append the name to co_name.

        Bcode filter:
            Raise NotImplementedError for an invalid bcode.

        If all is well, return the filtered consts list,
        names list, code string and native code.
        """

        ## General filter
        # ensure values fit within U8 type size
        assert len(co.co_consts) < 256, "too many constants."
        assert len(co.co_names) < 256, "too many names."
        assert co.co_argcount < 256, "too many arguments."
        assert co.co_stacksize < 256, "too large of a stack."
        assert co.co_nlocals < 256, "too many local variables."

        # make consts a list so a single element can be modified
        consts = list(co.co_consts)

        # Check co_flags
        unsupported_flags = CO_VARARGS | CO_VARKEYWORDS
        if not PM_FEATURES["HAVE_GENERATORS"]:
            unsupported_flags |= CO_GENERATOR
        assert co.co_flags & unsupported_flags == 0,\
            "Unsupported code identified by co_flags (%s)." % hex(co.co_flags)

        # get trimmed src file name and module name
        fn = os.path.basename(co.co_filename)
        mn = os.path.splitext(fn)[0]

        # init native code
        nativecode = None

        ## Bcode filter
        # bcode string
        s = co.co_code
        # filtered code string
        code = ""
        # iterate through the string
        lno = 0
        i = 0
        len_s = len(s)
        while i < len_s:

            #get char
            c = ord(s[i])

            #ensure no illegal bytecodes are present
            if self.bcodes[c] == None:
                raise NotImplementedError(
                        "Illegal bytecode (%d/%s/%s) "
                        "comes at offset %d in file %s." %
                        (c, hex(c), dis.opname[c],
                         i, co.co_filename))

            #if simple bcode, copy one byte
            if c < dis.HAVE_ARGUMENT:
                code += s[i]
                i += 1

            #else copy three bytes
            else:

                # Raise error if default arguments exist and are not configured
                if (not PM_FEATURES["HAVE_DEFAULTARGS"]
                    and c == dis.opmap["MAKE_FUNCTION"]
                    and self._str_to_U16(s[i+1:i+3]) > 0):

                    raise NotImplementedError(
                            "Bytecode (%d/%s/%s) not configured "
                            "to support default arguments; "
                            "comes at offset %d in file %s." %
                            (c, hex(c), dis.opname[c], i, co.co_filename))

                # Otherwise, copy the code (3 bytes)
                code += s[i:i+3]
                i += 3

        # if the first const is a String,
        if (len(consts) > 0 and type(consts[0]) == types.StringType):

            ## Native code filter
            # if this CO is intended to be a native func.
            if (consts[0][:NATIVE_INDICATOR_LENGTH] ==
                NATIVE_INDICATOR):

                # ensure num args is less or equal
                # to NATIVE_MAX_NUM_LOCALS
                assert co.co_nlocals <= NATIVE_MAX_NUM_LOCALS

                # extract native code and clear doc string
                nativecode = consts[0][NATIVE_INDICATOR_LENGTH:]
                consts[0] = None

                # If this co is a module
                # Old #28: Module root must keep its bytecode
                if co.co_name == MODULE_IDENTIFIER:
                    self.nativemods.append((co.co_filename, nativecode))

                # Else this co is a function;
                # replace code with native table index
                else:
                    # stdlib code gets a positive index
                    if self.imgtarget == "std":
                        code = self._U16_to_str(len(self.nativetable))
                    # usr code gets a negative index
                    else:
                        code = self._U16_to_str(-len(self.nativetable))

                    # native function name is
                    # "nat_<modname>_<pyfuncname>".
                    # append (nat func name, nat code) to table
                    self.nativetable.append(
                        ("%s%02d_%s_%s"
                         % (NATIVE_FUNC_PREFIX, self.nfcount, mn, co.co_name),
                         nativecode))
                    self.nfcount += 1

            ## Consts filter
            # if want to remove __doc__ string
            # WARNING: this heuristic is not always accurate
            elif (REMOVE_DOC_STR and len(co.co_names) > 0
                  and co.co_names[0] == "__doc__"):
                consts[0] = None

        ## Names filter
        names = list(co.co_names)

        # Remove __doc__ name if requested
        if REMOVE_DOC_STR and len(names) > 0 and names[0] == "__doc__":
            names[0] = ''

        # if co_name is the module identifier change it to module name
        if co.co_name == MODULE_IDENTIFIER:
            names.append(mn)
        # else use unmodified co_name
        else:
            names.append(co.co_name)


        return consts, names, code, nativecode


################################################################
# IMAGE WRITING FUNCTIONS
################################################################

    def write_image_file(self,):
        """Writes an image file
        """
        fmtfxn = self.formatFromExt[self.imgtype]
        f = open(self.outfn, 'wb')
        f.write(fmtfxn())
        f.close()

    def write_native_file(self,):
        """Writes native functions if filename was given
        """
        if not self.nativeFilename:
            return
        f = open(self.nativeFilename, 'wb')
        f.write(self.format_native_table())
        f.close()


    def format_img_as_bin(self,):
        """format_img_as_bin() --> string

        Write image bytes to raw binary string.
        The resulting string is suitable to write to a file.
        """

        # no reformatting necessary, join all object images
        return string.join(self.imgDict["imgs"], "")


    def format_img_as_c(self,):
        """format_img_as_c() --> string

        Format image bytes to a string that is a C byte array.
        The C byte array can be located in RAM
        or program memory.  The byte array is named lib_img.
        """

        # list of filenames
        fns = self.imgDict["fns"]
        imgs = self.imgDict["imgs"]

        # create intro
        fileBuff = []
        fileBuff.append("/**\n"
                        " * PyMite library image file.\n"
                        " *\n"
                        " * Automatically created from:\n"
                        " * \t%s\n"
                        " * by pmImgCreator.py on\n"
                        " * %s.\n"
                        " * \n"
                        " * Byte count: %d\n"
                        " * \n"
                        " * Selected memspace type: %s\n"
                        " * \n"
                        " * DO NOT EDIT THIS FILE.\n"
                        " * ANY CHANGES WILL BE LOST.\n"
                        " */\n\n"
                        % (string.join(fns, "\n *\t"),
                           time.ctime(time.time()),
                           len(string.join(imgs, "")),
                           self.memspace.upper()
                          )
                       )
        fileBuff.append("/* Place the image into %s */\n"
                        "#ifdef __cplusplus\n"
                        "extern\n"
                        "#endif\n"
                        "unsigned char const\n"
                        % self.memspace.upper()
                       )

        if self.memspace.lower() == "flash":
            fileBuff.append("#if defined(__AVR__)\n"
                            "__attribute__((progmem))\n"
                            "#endif\n"
                           )

        fileBuff.append("%slib_img[] =\n"
                        "{\n"
                        % (self.imgtarget)
                       )

        # for each src file, convert and format
        i = 0
        for fn in fns:

            # get img string for this file
            img = imgs[i]
            i += 1

            # print all bytes
            fileBuff.append("\n\n/* %s */" % fn)
            j = 0
            while j < len(img):
                if (j % 8) == 0:
                    fileBuff.append("\n    ")
                fileBuff.append("0x%02X, " % ord(img[j]))
                j += 1

        # finish off array
        fileBuff.append("\n};\n")

        return string.join(fileBuff, "")


    def format_native_table(self,):
        """format_native_table() --> string

        Format native table to a C file containing
        native functions and a function table.
        """
        # create intro
        fileBuff = []
        fileBuff.append("#undef __FILE_ID__\n"
                        "#define __FILE_ID__ 0x0A\n"
                        "/**\n"
                        " * PyMite %s native function file\n"
                        " *\n"
                        " * automatically created by pmImgCreator.py\n"
                        " * on %s\n"
                        " *\n"
                        " * DO NOT EDIT THIS FILE.\n"
                        " * ANY CHANGES WILL BE LOST.\n"
                        " *\n"
                        " * @file    %s\n"
                        " */\n\n"
                        "#define __IN_LIBNATIVE_C__\n"
                        "#include \"pm.h\"\n\n"
                        % (self.imgtarget,
                           time.ctime(time.time()),
                           self.nativeFilename
                          )
                       )

        # module-level native sections (for #include headers)
        for (modname, modstr) in self.nativemods:
            fileBuff.append("/* From: %s */%s\n" % (modname, modstr))

        # for each entry create fxn
        for (funcname, funcstr) in self.nativetable:
            fileBuff.append("PmReturn_t\n"
                            "%s(pPmFrame_t *ppframe)\n"
                            "{\n"
                            "%s\n"
                            "}\n\n" % (funcname, funcstr))

        # create fxn table
        fileBuff.append("/* Native function lookup table */\n"
                        "pPmNativeFxn_t const %s[] =\n"
                        "{\n" % NATIVE_TABLE_NAME[self.imgtarget])

        # put all native funcs in the table
        for (funcname, funcstr) in self.nativetable:
            fileBuff.append("    %s,\n" % funcname)
        fileBuff.append("};\n")

        return string.join(fileBuff, "")


################################################################
# MAIN
################################################################

def parse_cmdline():
    """Parses the command line for options.
    """
    try:
        opts, args = getopt.getopt(sys.argv[1:],
                                   "f:bcsuo:",
                                   ["memspace=", "native-file="])
    except:
        print __usage__
        sys.exit(EX_USAGE)

    # Parse opts for the image type to write
    imgtype = ".c"
    imgtarget = "std"
    memspace = "ram"
    outfn = None
    nativeFilename = None
    for opt in opts:
        if opt[0] == "-b":
            imgtype = ".bin"
        elif opt[0] == "-c":
            imgtype = ".c"
        elif opt[0] == "-s":
            imgtarget = "std"
        elif opt[0] == "-u":
            imgtarget = "usr"
        elif opt[0] == "--memspace":
            # Error if memspace switch given without arg
            if not opt[1] or (opt[1].lower() not in ["ram", "flash"]):
                print "Only one of these memspace types allowed: ram, flash"
                print __usage__
                sys.exit(EX_USAGE)
            memspace = opt[1]
        elif opt[0] == "--native-file":
            # Error if switch given without arg
            if not opt[1]:
                print "Specify a filename like this: --native-file=libnative.c"
                print __usage__
                sys.exit(EX_USAGE)
            nativeFilename = opt[1]
        elif opt[0] == "-f":
            pmfeatures_filename = opt[1]
        elif opt[0] == "-o":
            # Error if out filename switch given without arg
            if not opt[1]:
                print __usage__
                sys.exit(EX_USAGE)
            outfn = opt[1]

    # Error if no image type was given
    if not imgtype:
        print __usage__
        sys.exit(EX_USAGE)

    # Error if no input filenames are given
    if len(args) == 0:
        print __usage__
        sys.exit(EX_USAGE)

    return outfn, imgtype, imgtarget, memspace, nativeFilename, args, pmfeatures_filename


def main():
    outfn, imgtyp, imgtarget, memspace, natfn, fns, pmfn = parse_cmdline()
    pic = PmImgCreator(pmfn)
    pic.set_options(outfn, imgtyp, imgtarget, memspace, natfn, fns)
    pic.convert_files()
    pic.write_image_file()
    pic.write_native_file()


if __name__ == "__main__":
    main()
