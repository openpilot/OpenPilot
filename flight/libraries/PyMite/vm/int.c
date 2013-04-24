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
#define __FILE_ID__ 0x08


/**
 * \file
 * \brief Integer Object Type
 *
 * Integer object type operations.
 */

#include <stdint.h>
#include <limits.h>

#include "pm.h"


PmReturn_t
int_dup(pPmObj_t pint, pPmObj_t *r_pint)
{
    PmReturn_t retval = PM_RET_OK;

    /* Allocate new int */
    retval = heap_getChunk(sizeof(PmInt_t), (uint8_t **)r_pint);
    PM_RETURN_IF_ERROR(retval);

    /* Copy value */
    OBJ_SET_TYPE(*r_pint, OBJ_TYPE_INT);
    ((pPmInt_t)*r_pint)->val = ((pPmInt_t)pint)->val;
    return retval;
}


PmReturn_t
int_new(int32_t n, pPmObj_t *r_pint)
{
    PmReturn_t retval = PM_RET_OK;

    /* If n is 0,1,-1, return static int objects from global struct */
    if (n == 0)
    {
        *r_pint = PM_ZERO;
        return PM_RET_OK;
    }
    if (n == 1)
    {
        *r_pint = PM_ONE;
        return PM_RET_OK;
    }
    if (n == -1)
    {
        *r_pint = PM_NEGONE;
        return PM_RET_OK;
    }

    /* Else create and return new int obj */
    retval = heap_getChunk(sizeof(PmInt_t), (uint8_t **)r_pint);
    PM_RETURN_IF_ERROR(retval);
    OBJ_SET_TYPE(*r_pint, OBJ_TYPE_INT);
    ((pPmInt_t)*r_pint)->val = n;
    return retval;
}


PmReturn_t
int_positive(pPmObj_t pobj, pPmObj_t *r_pint)
{
    PmReturn_t retval;

    /* Raise TypeError if obj is not an int */
    if (OBJ_GET_TYPE(pobj) != OBJ_TYPE_INT)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* Create new int obj */
    return int_new(((pPmInt_t)pobj)->val, r_pint);
}


PmReturn_t
int_negative(pPmObj_t pobj, pPmObj_t *r_pint)
{
    PmReturn_t retval;

    /* Raise TypeError if obj is not an int */
    if (OBJ_GET_TYPE(pobj) != OBJ_TYPE_INT)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* Create new int obj */
    return int_new(-((pPmInt_t)pobj)->val, r_pint);
}


PmReturn_t
int_bitInvert(pPmObj_t pobj, pPmObj_t *r_pint)
{
    PmReturn_t retval;

    /* Raise TypeError if obj is not an int */
    if (OBJ_GET_TYPE(pobj) != OBJ_TYPE_INT)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* Create new int obj */
    return int_new(~((pPmInt_t)pobj)->val, r_pint);
}


#ifdef HAVE_PRINT
PmReturn_t
int_print(pPmObj_t pint)
{
    /* 2^31-1 has 10 decimal digits, plus sign and zero byte */
    uint8_t tBuffer[10 + 1 + 1];
    uint8_t bytesWritten;
    uint8_t k;
    PmReturn_t retval = PM_RET_OK;

    C_ASSERT(pint != C_NULL);

    /* Raise TypeError if obj is not an int */
    if (OBJ_GET_TYPE(pint) != OBJ_TYPE_INT)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* #196: Changed to use snprintf */
    bytesWritten =
        snprintf((char *)&tBuffer, 12, "%li", (long int)((pPmInt_t)pint)->val);


    /* Sanity check */
    C_ASSERT(bytesWritten != 0);
    C_ASSERT(bytesWritten < sizeof(tBuffer));

    for (k = (uint8_t)0; k < bytesWritten; k++)
    {
        retval = plat_putByte(tBuffer[k]);
        PM_RETURN_IF_ERROR(retval);
    }
    return PM_RET_OK;
}


PmReturn_t
int_printHexByte(uint8_t b)
{
    uint8_t nibble;
    PmReturn_t retval;

    nibble = (b >> 4) + '0';
    if (nibble > '9')
        nibble += ('a' - '0' - 10);
    retval = plat_putByte(nibble);
    PM_RETURN_IF_ERROR(retval);

    nibble = (b & (uint8_t)0x0F) + '0';
    if (nibble > '9')
        nibble += ('a' - '0' - (uint8_t)10);
    retval = plat_putByte(nibble);
    return retval;
}


PmReturn_t
_int_printHex(intptr_t n)
{
    PmReturn_t retval;
    int8_t i;

    /* Print the hex value, most significant byte first */
    for (i = CHAR_BIT * sizeof(intptr_t) - 8; i >= 0; i -= 8)
    {
        retval = int_printHexByte((n >> i) & 0xFF);
        PM_BREAK_IF_ERROR(retval);
    }

    return retval;
}


PmReturn_t
int_printHex(pPmObj_t pint)
{
    C_ASSERT(OBJ_GET_TYPE(pint) == OBJ_TYPE_INT);

    /* Print the integer object */
    return _int_printHex(((pPmInt_t)pint)->val);
}
#endif /* HAVE_PRINT */


PmReturn_t
int_pow(pPmObj_t px, pPmObj_t py, pPmObj_t *r_pn)
{
    int32_t x;
    int32_t y;
    int32_t n;
    PmReturn_t retval;

    /* Raise TypeError if args aren't ints */
    if ((OBJ_GET_TYPE(px) != OBJ_TYPE_INT)
        || (OBJ_GET_TYPE(py) != OBJ_TYPE_INT))
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    x = ((pPmInt_t)px)->val;
    y = ((pPmInt_t)py)->val;

    /* Raise Value error if exponent is negative */
    if (y < 0)
    {
        PM_RAISE(retval, PM_RET_EX_VAL);
        return retval;
    }

    /* Calculate x raised to y */
    n = 1;
    while (y > 0)
    {
        n = n * x;
        y--;
    }
    retval = int_new(n, r_pn);

    return retval;
}
