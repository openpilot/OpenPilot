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


#ifndef __GLOBAL_H__
#define __GLOBAL_H__


/**
 * \file
 * \brief VM Globals
 *
 * VM globals header.
 */


/** The global root PmGlobals Dict object */
#define PM_PBUILTINS    (pPmObj_t)(gVmGlobal.builtins)

/** The global None object */
#define PM_NONE         (pPmObj_t)(gVmGlobal.pnone)

/** The global False object */
#define PM_FALSE        (pPmObj_t)(gVmGlobal.pfalse)

/** The global True object */
#define PM_TRUE         (pPmObj_t)(gVmGlobal.ptrue)

/** The global integer 0 object */
#define PM_ZERO         (pPmObj_t)(gVmGlobal.pzero)

/** The global integer 1 object */
#define PM_ONE          (pPmObj_t)(gVmGlobal.pone)

/** The global integer -1 object */
#define PM_NEGONE       (pPmObj_t)(gVmGlobal.pnegone)

/** The global string "code" */
#define PM_CODE_STR     (pPmObj_t)(gVmGlobal.pcodeStr)

#ifdef HAVE_CLASSES
/** The global string "__init__" */
#define PM_INIT_STR     (pPmObj_t)(gVmGlobal.pinitStr)
#endif /* HAVE_CLASSES */

#ifdef HAVE_GENERATORS
/** The global string "Generator" */
#define PM_GENERATOR_STR (pPmObj_t)(gVmGlobal.pgenStr)
/** The global string "next" */
#define PM_NEXT_STR (pPmObj_t)(gVmGlobal.pnextStr)
#endif /* HAVE_GENERATORS */

#ifdef HAVE_ASSERT
/** The global string "Exception" */
#define PM_EXCEPTION_STR (pPmObj_t)(gVmGlobal.pexnStr)
#endif /* HAVE_ASSERT */

#ifdef HAVE_BYTEARRAY
/** The global string "bytearray" */
#define PM_BYTEARRAY_STR (pPmObj_t)(gVmGlobal.pbaStr)
#endif /* HAVE_BYTEARRAY */

/**
 * This struct contains ALL of PyMite's globals
 */
typedef struct PmVmGlobal_s
{
    /** Global none obj (none) */
    pPmObj_t pnone;

    /** Global integer 0 obj */
    pPmInt_t pzero;

    /** Global integer 1 obj */
    pPmInt_t pone;

    /** Global integer -1 obj */
    pPmInt_t pnegone;

    /** Global boolean False obj */
    pPmInt_t pfalse;

    /** Global boolean True obj */
    pPmInt_t ptrue;

    /** The string "code", used in interp.c RAISE_VARARGS */
    pPmString_t pcodeStr;

    /** Dict for builtins */
    pPmDict_t builtins;

    /** Paths to available images */
    PmImgPaths_t imgPaths;

    /** The single native frame.  Static alloc so it won't be GC'd */
    PmNativeFrame_t nativeframe;

    /** PyMite release value for when an error occurs */
    uint8_t errVmRelease;

    /** PyMite source file ID number for when an error occurs */
    uint8_t errFileId;

    /** Line number for when an error occurs */
    uint16_t errLineNum;

    /** Thread list */
    pPmList_t threadList;

    /** Ptr to current thread */
    pPmThread_t pthread;

#ifdef HAVE_CLASSES
    /* NOTE: placing this field before the nativeframe field causes errors */
    /** The string "__init__", used in interp.c CALL_FUNCTION */
    pPmString_t pinitStr;
#endif /* HAVE_CLASSES */

#ifdef HAVE_GENERATORS
    /** The string "Generator", used in interp.c CALL_FUNCTION */
    pPmString_t pgenStr;
    /** The string "next", used in interp.c FOR_ITER */
    pPmString_t pnextStr;
#endif /* HAVE_GENERATORS */

#ifdef HAVE_ASSERT
    /** The string "Exception", used in RAISE_VARARGS */
    pPmString_t pexnStr;
#endif /* HAVE_ASSERT */

#ifdef HAVE_BYTEARRAY
    /** The global string "bytearray" */
    pPmString_t pbaStr;
#endif /* HAVE_BYTEARRAY */

#ifdef HAVE_PRINT
    /** Remembers when a space is needed before printing the next object */
    uint8_t needSoftSpace;
    /** Remembers when something has printed since the last newline */
    uint8_t somethingPrinted;
#endif /* HAVE_PRINT */

    /** Flag to trigger rescheduling */
    uint8_t reschedule;
} PmVmGlobal_t,
 *pPmVmGlobal_t;


extern volatile PmVmGlobal_t gVmGlobal;


/**
 * Initializes the global struct
 *
 * @return Return status
 */
PmReturn_t global_init(void);

/**
 * Sets the builtins dict into the given module's attrs.
 *
 * If not yet done, loads the "__bt" module via global_loadBuiltins().
 * Restrictions described in that functions documentation apply.
 *
 * @param pmod Module whose attrs receive builtins
 * @return Return status
 */
PmReturn_t global_setBuiltins(pPmFunc_t pmod);

/**
 * Loads the "__bt" module and sets the builtins dict (PM_PBUILTINS)
 * to point to __bt's attributes dict.
 * Creates "None" = None entry in builtins.
 *
 * When run, there should not be any other threads in the interpreter
 * thread list yet.
 *
 * @return  Return status
 */
PmReturn_t global_loadBuiltins(void);

#endif /* __GLOBAL_H__ */
