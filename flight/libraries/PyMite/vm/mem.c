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
#define __FILE_ID__ 0x0D


/**
 * \file
 * \brief VM Memory
 *
 * VM memory operations.
 * Implementations and stubs for getByte and memCopy functions.
 * Functions to load object images from static memory.
 */


#include "pm.h"


uint16_t
mem_getWord(PmMemSpace_t memspace, uint8_t const **paddr)
{
    /* PyMite is little endian; get low byte first */
    uint8_t blo = mem_getByte(memspace, paddr);
    uint8_t bhi = mem_getByte(memspace, paddr);

    return (uint16_t)(blo | (bhi << (int8_t)8));
}


uint32_t
mem_getInt(PmMemSpace_t memspace, uint8_t const **paddr)
{
    /* PyMite is little endian; get low word first */
    uint16_t wlo = mem_getWord(memspace, paddr);
    uint32_t whi = mem_getWord(memspace, paddr);

    return (uint32_t)(wlo | (whi << (int8_t)16));
}


#ifdef HAVE_FLOAT
float
mem_getFloat(PmMemSpace_t memspace, uint8_t const **paddr)
{
    union
    {
        char c[4];
        float f;
    }
    v;

#ifdef PM_FLOAT_BIG_ENDIAN
    /* If the architecture is Big Endian, reverse the bytes of the float */
    v.c[3] = mem_getByte(memspace, paddr);
    v.c[2] = mem_getByte(memspace, paddr);
    v.c[1] = mem_getByte(memspace, paddr);
    v.c[0] = mem_getByte(memspace, paddr);

#else
    v.c[0] = mem_getByte(memspace, paddr);
    v.c[1] = mem_getByte(memspace, paddr);
    v.c[2] = mem_getByte(memspace, paddr);
    v.c[3] = mem_getByte(memspace, paddr);

#ifndef PM_FLOAT_LITTLE_ENDIAN
#warning Neither PM_FLOAT_LITTLE_ENDIAN nor PM_FLOAT_BIG_ENDIAN is defined \
         for this platform; defaulting to little endian.
#endif
#endif

    return v.f;
}
#endif /* HAVE_FLOAT */


void
mem_copy(PmMemSpace_t memspace,
         uint8_t **pdest, uint8_t const **psrc, uint16_t count)
{
    /* Copy memory from RAM */
    if (memspace == MEMSPACE_RAM)
    {
        sli_memcpy(*pdest, *psrc, count);
        *psrc += count;
        *pdest += count;
        return;
    }

    /* Copy memory from non-RAM to RAM */
    else
    {
        uint8_t b;

        for (; count > 0; count--)
        {
            b = mem_getByte(memspace, psrc);
            **pdest = b;
            (*pdest)++;
        }
        return;
    }
}


uint16_t
mem_getStringLength(PmMemSpace_t memspace, uint8_t const *const pstr)
{
    uint8_t const *psrc;

    /* If source is in RAM, use a possibly optimized strlen */
    if (memspace == MEMSPACE_RAM)
    {
        return sli_strlen((char const *)pstr);
    }

    /* Otherwise calculate string length */
    psrc = pstr;
    while (mem_getByte(memspace, &psrc) != (uint8_t)0);
    return psrc - pstr - 1;
}


PmReturn_t
mem_cmpn(uint8_t *cname, uint8_t cnamelen, PmMemSpace_t memspace,
         uint8_t const **paddr)
{
    uint8_t i;
    uint8_t b;

    /* Iterate over all characters */
    for (i = 0; i < cnamelen; i++)
    {
        b = mem_getByte(memspace, paddr);
        if (cname[i] != b)
        {
            return PM_RET_NO;
        }
    }
    return PM_RET_OK;
}
