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
#define __FILE_ID__ 0x03


/**
 * \file
 * \brief VM Frame
 *
 * VM frame operations.
 */


#include "pm.h"


PmReturn_t
frame_new(pPmObj_t pfunc, pPmObj_t *r_pobj)
{
    PmReturn_t retval = PM_RET_OK;
    int16_t fsize = 0;
    pPmCo_t pco = C_NULL;
    pPmFrame_t pframe = C_NULL;
    uint8_t *pchunk;

    /* Get fxn's code obj */
    pco = ((pPmFunc_t)pfunc)->f_co;

    /* TypeError if passed func's CO is not a true COB */
    if (OBJ_GET_TYPE(pco) != OBJ_TYPE_COB)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

#ifdef HAVE_GENERATORS
    /* #207: Initializing a Generator using CALL_FUNC needs extra stack slot */
    fsize = sizeof(PmFrame_t) + (pco->co_stacksize + pco->co_nlocals + 2) * sizeof(pPmObj_t);
#elif defined(HAVE_CLASSES)
    /* #230: Calling a class's __init__() takes two extra spaces on the stack */
    fsize = sizeof(PmFrame_t) + (pco->co_stacksize + pco->co_nlocals + 1) * sizeof(pPmObj_t);
#else
    fsize = sizeof(PmFrame_t) + (pco->co_stacksize + pco->co_nlocals - 1) * sizeof(pPmObj_t);
#endif /* HAVE_CLASSES */

#ifdef HAVE_CLOSURES
    /* #256: Add support for closures */
    fsize = fsize + pco->co_nfreevars
            + ((pco->co_cellvars == C_NULL) ? 0 : pco->co_cellvars->length);
#endif /* HAVE_CLOSURES */

    /* Allocate a frame */
    retval = heap_getChunk(fsize, &pchunk);
    PM_RETURN_IF_ERROR(retval);
    pframe = (pPmFrame_t)pchunk;

    /* Set frame fields */
    OBJ_SET_TYPE(pframe, OBJ_TYPE_FRM);
    pframe->fo_back = C_NULL;
    pframe->fo_func = (pPmFunc_t)pfunc;
    pframe->fo_memspace = pco->co_memspace;

    /* Init instruction pointer and block stack */
    pframe->fo_ip = pco->co_codeaddr;
    pframe->fo_blockstack = C_NULL;

    /* Get globals and attrs from the function object */
    pframe->fo_globals = ((pPmFunc_t)pfunc)->f_globals;
    pframe->fo_attrs = ((pPmFunc_t)pfunc)->f_attrs;

#ifndef HAVE_CLOSURES
    /* Empty stack points to one past locals */
    pframe->fo_sp = &(pframe->fo_locals[pco->co_nlocals]);
#else
    /* #256: Add support for closures */
    pframe->fo_sp = &(pframe->fo_locals[pco->co_nlocals + pco->co_nfreevars
        + ((pco->co_cellvars == C_NULL) ? 0 : pco->co_cellvars->length)]);
#endif /* HAVE_CLOSURES */

    /* By default, this is a normal frame, not an import or __init__ one */
    pframe->fo_isImport = 0;
#ifdef HAVE_CLASSES
    pframe->fo_isInit = 0;
#endif

    /* Clear the stack */
    sli_memset((unsigned char *)&(pframe->fo_locals), (char const)0,
               (unsigned int)fsize - sizeof(PmFrame_t));

    /* Return ptr to frame */
    *r_pobj = (pPmObj_t)pframe;
    return retval;
}
