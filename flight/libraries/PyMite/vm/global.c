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


#undef __FILE_ID__
#define __FILE_ID__ 0x05


/**
 * \file
 * \brief VM Globals
 *
 * VM globals operations.
 * PyMite's global struct def and initial values.
 */


#include "pm.h"


extern unsigned char const *stdlib_img;

static uint8_t const *bistr = (uint8_t const *)"__bi";


/** Most PyMite globals all in one convenient place */
volatile PmVmGlobal_t gVmGlobal;


PmReturn_t
global_init(void)
{
    PmReturn_t retval;
    uint8_t *codestr = (uint8_t *)"code";
    uint8_t *pchunk;
    pPmObj_t pobj;
#ifdef HAVE_CLASSES
    uint8_t const *initstr = (uint8_t const *)"__init__"; 
#endif /* HAVE_CLASSES */
#ifdef HAVE_GENERATORS
    uint8_t const *genstr = (uint8_t const *)"Generator";
    uint8_t const *nextstr = (uint8_t const *)"next";
#endif /* HAVE_GENERATORS */
#ifdef HAVE_ASSERT
    uint8_t const *exnstr = (uint8_t const *)"Exception";
#endif /* HAVE_ASSERT */
#ifdef HAVE_BYTEARRAY
    uint8_t const *pbastr = (uint8_t const *)"bytearray";
#endif /* HAVE_BYTEARRAY */

    /* Clear the global struct */
    sli_memset((uint8_t *)&gVmGlobal, '\0', sizeof(PmVmGlobal_t));

    /* Set the PyMite release num (for debug and post mortem) */
    gVmGlobal.errVmRelease = PM_RELEASE;

    /* Init zero */
    retval = heap_getChunk(sizeof(PmInt_t), &pchunk);
    PM_RETURN_IF_ERROR(retval);
    pobj = (pPmObj_t)pchunk;
    OBJ_SET_TYPE(pobj, OBJ_TYPE_INT);
    ((pPmInt_t)pobj)->val = (int32_t)0;
    gVmGlobal.pzero = (pPmInt_t)pobj;

    /* Init one */
    retval = heap_getChunk(sizeof(PmInt_t), &pchunk);
    PM_RETURN_IF_ERROR(retval);
    pobj = (pPmObj_t)pchunk;
    OBJ_SET_TYPE(pobj, OBJ_TYPE_INT);
    ((pPmInt_t)pobj)->val = (int32_t)1;
    gVmGlobal.pone = (pPmInt_t)pobj;

    /* Init negone */
    retval = heap_getChunk(sizeof(PmInt_t), &pchunk);
    PM_RETURN_IF_ERROR(retval);
    pobj = (pPmObj_t)pchunk;
    OBJ_SET_TYPE(pobj, OBJ_TYPE_INT);
    ((pPmInt_t)pobj)->val = (int32_t)-1;
    gVmGlobal.pnegone = (pPmInt_t)pobj;

    /* Init False */
    retval = heap_getChunk(sizeof(PmBoolean_t), &pchunk);
    PM_RETURN_IF_ERROR(retval);
    pobj = (pPmObj_t)pchunk;
    OBJ_SET_TYPE(pobj, OBJ_TYPE_BOOL);
    ((pPmBoolean_t) pobj)->val = (int32_t)C_FALSE;
    gVmGlobal.pfalse = (pPmInt_t)pobj;

    /* Init True */
    retval = heap_getChunk(sizeof(PmBoolean_t), &pchunk);
    PM_RETURN_IF_ERROR(retval);
    pobj = (pPmObj_t)pchunk;
    OBJ_SET_TYPE(pobj, OBJ_TYPE_BOOL);
    ((pPmBoolean_t) pobj)->val = (int32_t)C_TRUE;
    gVmGlobal.ptrue = (pPmInt_t)pobj;

    /* Init None */
    retval = heap_getChunk(sizeof(PmObj_t), &pchunk);
    PM_RETURN_IF_ERROR(retval);
    pobj = (pPmObj_t)pchunk;
    OBJ_SET_TYPE(pobj, OBJ_TYPE_NON);
    gVmGlobal.pnone = pobj;

    /* Init "code" string obj */
    retval = string_new((uint8_t const **)&codestr, &pobj);
    PM_RETURN_IF_ERROR(retval);
    gVmGlobal.pcodeStr = (pPmString_t)pobj;

#ifdef HAVE_CLASSES
    /* Init "__init__" string obj */
    retval = string_new((uint8_t const **)&initstr, &pobj);
    PM_RETURN_IF_ERROR(retval);
    gVmGlobal.pinitStr = (pPmString_t)pobj;
#endif /* HAVE_CLASSES */

#ifdef HAVE_GENERATORS
    /* Init "Generator" string obj */
    retval = string_new((uint8_t const **)&genstr, &pobj);
    PM_RETURN_IF_ERROR(retval);
    gVmGlobal.pgenStr = (pPmString_t)pobj;
    
    /* Init "next" string obj */
    retval = string_new((uint8_t const **)&nextstr, &pobj);
    PM_RETURN_IF_ERROR(retval);
    gVmGlobal.pnextStr = (pPmString_t)pobj;
#endif /* HAVE_GENERATORS */

#ifdef HAVE_ASSERT
    /* Init "Exception" string obj */
    retval = string_new((uint8_t const **)&exnstr, &pobj);
    PM_RETURN_IF_ERROR(retval);
    gVmGlobal.pexnStr = (pPmString_t)pobj;
#endif /* HAVE_ASSERT */

#ifdef HAVE_BYTEARRAY
    /* Init "bytearray" string obj */
    retval = string_new((uint8_t const **)&pbastr, &pobj);
    PM_RETURN_IF_ERROR(retval);
    gVmGlobal.pbaStr = (pPmString_t)pobj;
#endif /* HAVE_BYTEARRAY */

    /* Init empty builtins */
    gVmGlobal.builtins = C_NULL;

    /* Init native frame */
    OBJ_SET_SIZE(&gVmGlobal.nativeframe, sizeof(PmNativeFrame_t));
    OBJ_SET_TYPE(&gVmGlobal.nativeframe, OBJ_TYPE_NFM);
    gVmGlobal.nativeframe.nf_func = C_NULL;
    gVmGlobal.nativeframe.nf_stack = C_NULL;
    gVmGlobal.nativeframe.nf_active = C_FALSE;
    gVmGlobal.nativeframe.nf_numlocals = 0;

    /* Create empty threadList */
    retval = list_new(&pobj);
    gVmGlobal.threadList = (pPmList_t)pobj;

    /* Init the PmImgPaths with std image info */
    gVmGlobal.imgPaths.memspace[0] = MEMSPACE_PROG;
    gVmGlobal.imgPaths.pimg[0] = (uint8_t *)&stdlib_img;
    gVmGlobal.imgPaths.pathcount = 1;

#ifdef HAVE_PRINT
    gVmGlobal.needSoftSpace = C_FALSE;
    gVmGlobal.somethingPrinted = C_FALSE;
#endif /* HAVE_PRINT */

    return retval;
}


PmReturn_t
global_setBuiltins(pPmFunc_t pmod)
{
    PmReturn_t retval = PM_RET_OK;
    pPmObj_t pkey = C_NULL;
    uint8_t const *pbistr = bistr;
    uint8_t objid;

    if (PM_PBUILTINS == C_NULL)
    {
        /* Need to load builtins first */
        global_loadBuiltins();
    }

    /* Put builtins module in the module's attrs dict */
    retval = string_new(&pbistr, &pkey);
    PM_RETURN_IF_ERROR(retval);

    heap_gcPushTempRoot(pkey, &objid);
    retval = dict_setItem((pPmObj_t)pmod->f_attrs, pkey, PM_PBUILTINS);
    heap_gcPopTempRoot(objid);

    return retval;
}


PmReturn_t
global_loadBuiltins(void)
{
    PmReturn_t retval = PM_RET_OK;
    pPmObj_t pkey = C_NULL;
    uint8_t const *nonestr = (uint8_t const *)"None";
    uint8_t const *falsestr = (uint8_t const *)"False";
    uint8_t const *truestr = (uint8_t const *)"True";
    pPmObj_t pstr = C_NULL;
    pPmObj_t pbimod;
    uint8_t const *pbistr = bistr;

    /* Import the builtins */
    retval = string_new(&pbistr, &pstr);
    PM_RETURN_IF_ERROR(retval);
    retval = mod_import(pstr, &pbimod);
    PM_RETURN_IF_ERROR(retval);

    /* Must interpret builtins' root code to set the attrs */
    C_ASSERT(gVmGlobal.threadList->length == 0);
    interp_addThread((pPmFunc_t)pbimod);
    retval = interpret(INTERP_RETURN_ON_NO_THREADS);
    PM_RETURN_IF_ERROR(retval);

    /* Builtins points to the builtins module's attrs dict */
    gVmGlobal.builtins = ((pPmFunc_t)pbimod)->f_attrs;

    /* Set None manually */
    retval = string_new(&nonestr, &pkey);
    PM_RETURN_IF_ERROR(retval);
    retval = dict_setItem(PM_PBUILTINS, pkey, PM_NONE);
    PM_RETURN_IF_ERROR(retval);

    /* Set False manually */
    retval = string_new(&falsestr, &pkey);
    PM_RETURN_IF_ERROR(retval);
    retval = dict_setItem(PM_PBUILTINS, pkey, PM_FALSE);
    PM_RETURN_IF_ERROR(retval);

    /* Set True manually */
    retval = string_new(&truestr, &pkey);
    PM_RETURN_IF_ERROR(retval);
    retval = dict_setItem(PM_PBUILTINS, pkey, PM_TRUE);
    PM_RETURN_IF_ERROR(retval);

    /* Deallocate builtins module */
    retval = heap_freeChunk((pPmObj_t)pbimod);

    return retval;
}
