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
#define __FILE_ID__ 0x07


/**
 * \file
 * \brief Image routines
 *
 * Created to eliminate a circular include
 * among mem, string and obj.
 */


#include "pm.h"


/*
 * Searches for a module's name in a contiguous array of images
 * in the given namespace starting at the given address.
 * A module's name is stored in the last index of the names tuple of an image.
 */
static PmReturn_t
img_findInPath(uint8_t *cname, uint8_t cnamelen, PmMemSpace_t memspace,
               uint8_t const **paddr)
{
    uint8_t const *imgtop;
    PmType_t type;
    uint16_t len;
    int16_t size = 0;
    uint8_t i = 0;

    /* Addr is top of img */
    imgtop = *paddr;

    /* Get img's type byte */
    type = (PmType_t)mem_getByte(memspace, paddr);

    /* Search all sequential images */
    while (type == OBJ_TYPE_CIM)
    {
        /* Use size field to calc addr of next potential img */
        size = mem_getWord(memspace, paddr);

        /* Point to names tuple */
        *paddr = imgtop + CI_NAMES_FIELD;

        /* Ensure it's a tuple */
        type = (PmType_t)mem_getByte(memspace, paddr);
        C_ASSERT(type == OBJ_TYPE_TUP);

        /* Scan to last name in tuple (it's the module's name) */
        i = mem_getByte(memspace, paddr) - (uint8_t)1;
        for (; i > 0; i--)
        {
            /* Ensure obj is a string */
            type = (PmType_t)mem_getByte(memspace, paddr);
            C_ASSERT(type == OBJ_TYPE_STR);

            /* Skip the length of the string */
            len = mem_getWord(memspace, paddr);
            (*paddr) += len;
        }

        /* Ensure it's a string */
        type = (PmType_t)mem_getByte(memspace, paddr);
        C_ASSERT(type == OBJ_TYPE_STR);

        /* If strings match, return the address of this image */
        if ((cnamelen == mem_getWord(memspace, paddr))
            && (PM_RET_OK == mem_cmpn(cname, cnamelen, memspace, paddr)))
        {
            *paddr = imgtop;
            return PM_RET_OK;
        }

        /* Calc imgtop for next iteration */
        imgtop += size;

        /* Point to next potential img */
        *paddr = imgtop;

        /* Check if another img follows this one */
        type = (PmType_t)mem_getByte(memspace, paddr);
    }
    return PM_RET_NO;
}


PmReturn_t
img_findInPaths(pPmObj_t pname, PmMemSpace_t *r_memspace,
                uint8_t const **r_imgaddr)
{
    uint8_t i;
    PmReturn_t retval = PM_RET_NO;

    /* Search in each path in the paths */
    for (i = 0; i < gVmGlobal.imgPaths.pathcount; i++)
    {
        *r_imgaddr = gVmGlobal.imgPaths.pimg[i];
        *r_memspace = gVmGlobal.imgPaths.memspace[i];
        retval = img_findInPath(((pPmString_t)pname)->val,
                                ((pPmString_t)pname)->length,
                                *r_memspace, r_imgaddr);
        if (retval == PM_RET_NO)
        {
            continue;
        }
        else if (retval == PM_RET_OK)
        {
            break;
        }
        else
        {
            return retval;
        }
    }

    return retval;
}


PmReturn_t
img_appendToPath(PmMemSpace_t memspace, uint8_t const * const paddr)
{
    uint8_t i;

    if (gVmGlobal.imgPaths.pathcount >= PM_NUM_IMG_PATHS)
    {
        return PM_RET_NO;
    }

    i = gVmGlobal.imgPaths.pathcount;

    gVmGlobal.imgPaths.memspace[i] = memspace;
    gVmGlobal.imgPaths.pimg[i] = paddr;
    gVmGlobal.imgPaths.pathcount++;

    return PM_RET_OK;
}
