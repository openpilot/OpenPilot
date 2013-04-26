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
#define __FILE_ID__ 0x01


/**
 * \file
 * \brief CodeObj Type
 *
 * CodeObj type operations.
 */


#include "pm.h"


/* The image format is defined by co_to_str() in src/tools/pmImgCreator.py */
PmReturn_t
co_loadFromImg(PmMemSpace_t memspace, uint8_t const **paddr, pPmObj_t *r_pco)
{
    PmReturn_t retval = PM_RET_OK;
    pPmObj_t pobj;
    pPmCo_t pco = C_NULL;
    uint8_t *pchunk;
    uint8_t objid;
#ifdef HAVE_DEBUG_INFO
    uint8_t objtype;
    uint16_t len_str;
#endif /* HAVE_DEBUG_INFO */

    /* Store ptr to top of code img (less type byte) */
    uint8_t const *pci = *paddr - 1;

    /* Get size of code img */
    uint16_t size = mem_getWord(memspace, paddr);

    /* Allocate a code obj */
    retval = heap_getChunk(sizeof(PmCo_t), &pchunk);
    PM_RETURN_IF_ERROR(retval);
    pco = (pPmCo_t)pchunk;

    /* Fill in the CO struct */
    OBJ_SET_TYPE(pco, OBJ_TYPE_COB);
    pco->co_memspace = memspace;
    pco->co_argcount = mem_getByte(memspace, paddr);
    pco->co_flags = mem_getByte(memspace, paddr);
    pco->co_stacksize = mem_getByte(memspace, paddr);
    pco->co_nlocals = mem_getByte(memspace, paddr);

    /* Do not set code image address if image is in RAM.
     * CIs in RAM have their image address set in obj_loadFromImgObj() */
    pco->co_codeimgaddr = (memspace == MEMSPACE_RAM) ? C_NULL : pci;

    /* Set these to null in case a GC occurs before their objects are alloc'd */
    pco->co_names = C_NULL;
    pco->co_consts = C_NULL;

#ifdef HAVE_CLOSURES
    pco->co_nfreevars = mem_getByte(memspace, paddr);
    pco->co_cellvars = C_NULL;
#endif /* HAVE_CLOSURES */

#ifdef HAVE_DEBUG_INFO
    pco->co_firstlineno = mem_getWord(memspace, paddr);
    pco->co_lnotab = C_NULL;
    pco->co_filename = C_NULL;
#endif /* HAVE_DEBUG_INFO */

    /* Load names (tuple obj) */
    heap_gcPushTempRoot((pPmObj_t)pco, &objid);
    retval = obj_loadFromImg(memspace, paddr, &pobj);
    heap_gcPopTempRoot(objid);
    PM_RETURN_IF_ERROR(retval);
    pco->co_names = (pPmTuple_t)pobj;

#ifdef HAVE_DEBUG_INFO
    /* Get address in memspace of line number table (including length) */
    objtype = mem_getByte(memspace, paddr);
    C_ASSERT(objtype == OBJ_TYPE_STR);
    pco->co_lnotab = *paddr;
    len_str = mem_getWord(memspace, paddr);
    *paddr = *paddr + len_str;

    /* Get address in memspace of CO's filename (excluding length) */
    objtype = mem_getByte(memspace, paddr);
    C_ASSERT(objtype == OBJ_TYPE_STR);
    len_str = mem_getWord(memspace, paddr);
    pco->co_filename = *paddr;
    *paddr = *paddr + len_str;
#endif /* HAVE_DEBUG_INFO */

    /* Load consts (tuple obj) assume it follows names */
    heap_gcPushTempRoot((pPmObj_t)pco, &objid);
    retval = obj_loadFromImg(memspace, paddr, &pobj);
    heap_gcPopTempRoot(objid);
    PM_RETURN_IF_ERROR(retval);
    pco->co_consts = (pPmTuple_t)pobj;

#ifdef HAVE_CLOSURES
    heap_gcPushTempRoot((pPmObj_t)pco, &objid);
    retval = obj_loadFromImg(memspace, paddr, &pobj);
    heap_gcPopTempRoot(objid);
    PM_RETURN_IF_ERROR(retval);

    /* Save RAM, don't keep empty tuple */
    if (((pPmTuple_t)pobj)->length == 0)
    {
        heap_freeChunk(pobj);
    }
    else
    {
        pco->co_cellvars = (pPmTuple_t)pobj;
    }
#endif /* HAVE_CLOSURES */

    /* Start of bcode always follows consts */
    pco->co_codeaddr = *paddr;

    /* Set addr to point one past end of img */
    *paddr = pci + size;

    *r_pco = (pPmObj_t)pco;
    return PM_RET_OK;
}


void
co_rSetCodeImgAddr(pPmCo_t pco, uint8_t const *pimg)
{
    uint8_t i;

    pco->co_codeimgaddr = pimg;

    /* Set the image address for any COs in the constant pool */
    for (i = 0; i < pco->co_consts->length; i++)
    {
        if (OBJ_GET_TYPE(pco->co_consts->val[i]) == OBJ_TYPE_COB)
        {
            co_rSetCodeImgAddr((pPmCo_t)pco->co_consts->val[i], pimg);
        }
    }

    return;
}


PmReturn_t
no_loadFromImg(PmMemSpace_t memspace, uint8_t const **paddr, pPmObj_t *r_pno)
{
    PmReturn_t retval = PM_RET_OK;
    pPmNo_t pno = C_NULL;
    uint8_t *pchunk;

    /* Allocate a code obj */
    retval = heap_getChunk(sizeof(PmNo_t), &pchunk);
    PM_RETURN_IF_ERROR(retval);
    pno = (pPmNo_t)pchunk;

    /* Fill in the NO struct */
    OBJ_SET_TYPE(pno, OBJ_TYPE_NOB);
    pno->no_argcount = mem_getByte(memspace, paddr);

    /* Get index into native fxn table */
    pno->no_funcindx = (int16_t)mem_getWord(memspace, paddr);

    *r_pno = (pPmObj_t)pno;
    return PM_RET_OK;
}
