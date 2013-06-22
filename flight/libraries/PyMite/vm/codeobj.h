/*
# This file is Copyright 2003, 2006, 2007, 2009, 2010 Dean Hall.
#
# This file is part of the PyMite VM.
# The PyMite VM is free software: you can redistribute it and/or modify
# it under the terms of the GNU GENERAL PUBLIC LICENSE Version 2.
#
# The PyMite VM is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# A copy of the GNU GENERAL PUBLIC LICENSE Version 2
# is seen in the file COPYING in this directory.
*/


#ifndef __CODEOBJ_H__
#define __CODEOBJ_H__


/**
 * \file
 * \brief CodeObj Type
 *
 * CodeObj type header.
 */


/** Code image field offset consts */
#define CI_TYPE_FIELD       0
#define CI_SIZE_FIELD       1
#define CI_ARGCOUNT_FIELD   3
#define CI_FLAGS_FIELD      4
#define CI_STACKSIZE_FIELD  5
#define CI_NLOCALS_FIELD    6

#ifdef HAVE_CLOSURES
# define CI_FREEVARS_FIELD  7
# ifdef HAVE_DEBUG_INFO
#  define CI_FIRST_LINE_NO  8
#  define CI_NAMES_FIELD    10
# else
#  define CI_NAMES_FIELD    8
# endif /* HAVE_DEBUG_INFO */
#else
# ifdef HAVE_DEBUG_INFO
#  define CI_FIRST_LINE_NO  7
#  define CI_NAMES_FIELD    9
# else
#  define CI_NAMES_FIELD    7
# endif /* HAVE_DEBUG_INFO */
#endif /* HAVE_CLOSURES */


/** Native code image size */
#define NATIVE_IMAGE_SIZE   4

/* Masks for co_flags (from Python's code.h) */
#define CO_OPTIMIZED 0x01
#define CO_NEWLOCALS 0x02
#define CO_VARARGS 0x04
#define CO_VARKEYWORDS 0x08
#define CO_NESTED 0x10
#define CO_GENERATOR 0x20
#define CO_NOFREE 0x40

/**
 * Code Object
 *
 * An extended object that holds only the most frequently used parts
 * of the static code image.  Other parts can be obtained by
 * inspecting the code image itself.
 */
typedef struct PmCo_s
{
    /** Object descriptor */
    PmObjDesc_t od;
    /** Address in progmem of the code image, or of code img obj in heap */
    uint8_t const *co_codeimgaddr;
    /** Address in RAM of names tuple */
    pPmTuple_t co_names;
    /** Address in RAM of constants tuple */
    pPmTuple_t co_consts;
    /** Address in memspace of bytecode (or native function) */
    uint8_t const *co_codeaddr;

#ifdef HAVE_DEBUG_INFO
    /** Address in memspace of the line number table */
    uint8_t const *co_lnotab;
    /** Address in memspace of the filename */
    uint8_t const *co_filename;
    /** Line number of first source line of lnotab */
    uint16_t co_firstlineno;
#endif /* HAVE_DEBUG_INFO */

#ifdef HAVE_CLOSURES
    /** Address in RAM of cellvars tuple */
    pPmTuple_t co_cellvars;
    /** Number of freevars */
    uint8_t co_nfreevars;
#endif /* HAVE_CLOSURES */

    /** Memory space selector */
    PmMemSpace_t co_memspace:8;
    /** Number of positional arguments the function expects */
    uint8_t co_argcount;
    /** Compiler flags */
    uint8_t co_flags;
    /** Stack size */
    uint8_t co_stacksize;
    /** Number of local variables */
    uint8_t co_nlocals;
} PmCo_t,
 *pPmCo_t;

/**
 * Native Code Object
 *
 * An extended object that holds only the most frequently used parts
 * of the static native image.  Other parts can be obtained by
 * inspecting the native image itself.
 */
typedef struct PmNo_s
{
    /** object descriptor */
    PmObjDesc_t od;
    /** expected num args to the func */
    int8_t no_argcount;
    /** index into native function table */
    int16_t no_funcindx;
} PmNo_t,
 *pPmNo_t;


/**
 * Creates a CodeObj by loading info from a code image in memory.
 *
 * An image is a static representation of a Python object.
 * The process of converting an object to and from an image
 * is also called marshalling.
 * In PyMite, code images are the equivalent of .pyc files.
 * Code images can only contain a select subset of object types
 * (None, Int, Float, String, Slice?, Tuple, and CodeImg).
 * All other types (Lists, Dicts, CodeObjs, Modules, Classes,
 * Functions, ClassInstances) are built at runtime.
 *
 * All multibyte values are in Little Endian order
 * (least significant byte comes first in the byte stream).
 *
 * memspace and *paddr determine the start of the code image.
 * Load the code object with values from the code image,
 * including the names and consts tuples.
 * Leave contents of paddr pointing one byte past end of
 * code img.
 *
 * The code image has the following structure:
 *      -type:      8b - OBJ_TYPE_CIM
 *      -size:      16b - number of bytes
 *                  the code image occupies.
 *      -argcount:  8b - number of arguments to this code obj.
 *      -stacksz:   8b - the maximum arg-stack size needed.
 *      -nlocals:   8b - number of local vars in the code obj.
 *      -names:     Tuple - tuple of string objs.
 *      -consts:    Tuple - tuple of objs.
 *      -code:      8b[] - bytecode array.
 *
 * @param   memspace memory space containing image
 * @param   paddr ptr to ptr to code img in memspace
 *          return by reference: paddr points one byte
 *          past end of code img
 * @param   r_pco Return arg.  New code object with fields
 *          filled in.
 * @return  Return status
 */
PmReturn_t
co_loadFromImg(PmMemSpace_t memspace, uint8_t const **paddr, pPmObj_t *r_pco);

/**
 * Recursively sets image address of the CO and all its nested COs
 * in its constant pool.  This is done so that an image that was
 * received during an interactive session will persist as long as any
 * of its COs/funcs/objects is still alive.
 *
 * @param   pco Pointer to root code object whose images are set
 * @param   pimg Pointer to very top of code image (PmodeImgObj)
 */
void co_rSetCodeImgAddr(pPmCo_t pco, uint8_t const *pimg);

/**
 * Creates a Native code object by loading a native image.
 *
 * An image is a static representation of a Python object.
 * A native image is much smaller than a regular image
 * because only two items of data are needed after the type:
 * the number of args the func expects and the index into
 * the native function table.
 * A reference to the image is not needed since it is
 * just as efficient to store the info in RAM as it is to
 * store a pointer and memspace value.
 *
 * memspace and *paddr determine the start of the native image.
 * Loads the argcount and the func index from the native object.
 * Leaves contents of paddr pointing one byte past end of
 * code img.
 *
 * The native image has the following structure:
 *      -type:      8b - OBJ_TYPE_CIM
 *      -argcount:  8b - number of arguments to this code obj.
 *      -code:      16b - index into native function table.
 *
 * @param   memspace memory space containing image
 * @param   paddr ptr to ptr to code img in memspace (return)
 * @param   r_pno Return by reference, new code object
 * @return  Return status
 */
PmReturn_t no_loadFromImg(PmMemSpace_t memspace,
                          uint8_t const **paddr, pPmObj_t *r_pno);

#endif /* __CODEOBJ_H__ */
