/*
# This file is Copyright 2010 Dean Hall.
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
#define __FILE_ID__ 0x19


/**
 * \file
 * \brief VM Bytearray Type
 *
 * VM Bytearray object type operations.
 */

#include "pm.h"
#ifdef HAVE_BYTEARRAY


#define ROUND_UP_TO_MUL_OF_FOUR(n) n = (((n) + 3) & ~3)


/* Returns a container that can hold at least n bytes */
static
PmReturn_t
bytes_new(int16_t n, pPmObj_t *r_pobj)
{
    PmReturn_t retval = PM_RET_OK;
    pPmBytes_t pb = C_NULL;

    ROUND_UP_TO_MUL_OF_FOUR(n);

    /* Allocate a container */
    retval = heap_getChunk(sizeof(PmBytes_t) + n, (uint8_t **)&pb);
    PM_RETURN_IF_ERROR(retval);
    OBJ_SET_TYPE(pb, OBJ_TYPE_BYS);
    pb->length = n;

    *r_pobj = (pPmObj_t)pb;
    return retval;
}


/* Returns the int or one-char string as a byte */
static
PmReturn_t
bytes_getByteFromObj(pPmObj_t pobj, uint8_t *b)
{
    PmReturn_t retval = PM_RET_OK;

    if (OBJ_GET_TYPE(pobj) == OBJ_TYPE_INT)
    {
        if ((((pPmInt_t)pobj)->val > 255) || (((pPmInt_t)pobj)->val < 0))
        {
            PM_RAISE(retval, PM_RET_EX_VAL);
            return retval;
        }

        *b = (uint8_t)((pPmInt_t)pobj)->val;
    }

    else if (OBJ_GET_TYPE(pobj) == OBJ_TYPE_STR)
    {
        if (((pPmString_t)pobj)->length != 1)
        {
            PM_RAISE(retval, PM_RET_EX_VAL);
            return retval;
        }
        *b = ((pPmString_t)pobj)->val[0];
    }

    else
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
    }
    return retval;
}


PmReturn_t
bytearray_new(pPmObj_t pobj, pPmObj_t *r_pobj)
{
    PmReturn_t retval = PM_RET_OK;
    pPmBytearray_t pba = C_NULL;
    pPmBytes_t pb = C_NULL;
    pPmObj_t pitem;
    int32_t i;
    int16_t n;
    uint8_t b;
    uint8_t objid;

    /* If object is an instance, get the thing it is containing */
    if (OBJ_GET_TYPE(pobj) == OBJ_TYPE_CLI)
    {
        retval = dict_getItem((pPmObj_t)((pPmInstance_t)pobj)->cli_attrs,
                              PM_NONE,
                              (pPmObj_t *)&pba);
        PM_RETURN_IF_ERROR(retval);
        pobj = (pPmObj_t)pba;
    }

    /* Get the requested length of the new bytearray */
    switch (OBJ_GET_TYPE(pobj))
    {
        case OBJ_TYPE_INT:
            i = ((pPmInt_t)pobj)->val;
            if ((i < 0) || (i > 65535))
            {
                PM_RAISE(retval, PM_RET_EX_VAL);
                return retval;
            }
            n = i;
            break;

        case OBJ_TYPE_STR:
            n = ((pPmString_t)pobj)->length;
            break;

        case OBJ_TYPE_LST:
            n = ((pPmList_t)pobj)->length;
            break;

        case OBJ_TYPE_TUP:
            n = ((pPmTuple_t)pobj)->length;
            break;

        case OBJ_TYPE_BYA:
            n = ((pPmBytearray_t)pobj)->length;
            break;

        default:
            PM_RAISE(retval, PM_RET_EX_TYPE);
            return retval;
    }

    /* Allocate a bytearray */
    retval = heap_getChunk(sizeof(PmBytearray_t), (uint8_t **)&pba);
    PM_RETURN_IF_ERROR(retval);
    OBJ_SET_TYPE(pba, OBJ_TYPE_BYA);
    pba->length = n;
    pba->val = C_NULL;

    /* Allocate the bytes container */
    heap_gcPushTempRoot((pPmObj_t)pba, &objid);
    retval = bytes_new(n, (pPmObj_t *)&pb);
    heap_gcPopTempRoot(objid);
    PM_RETURN_IF_ERROR(retval);
    pba->val = pb;

    /* Fill the bytes */
    switch (OBJ_GET_TYPE(pobj))
    {
        case OBJ_TYPE_INT:
            sli_memset((unsigned char *)&(pb->val), '\0', n);
            break;

        case OBJ_TYPE_BYA:
            pitem = (pPmObj_t)((pPmBytearray_t)pobj)->val;
            sli_memcpy(&(pb->val[0]), &(((pPmBytes_t)pitem)->val[0]), n);
            break;

        case OBJ_TYPE_STR:
            sli_memcpy(&(pb->val[0]), &(((pPmString_t)pobj)->val[0]), n);
            break;

        case OBJ_TYPE_LST:
        case OBJ_TYPE_TUP:
            for (i = 0; i < n; i++)
            {
                retval = seq_getSubscript(pobj, i, &pitem);
                PM_RETURN_IF_ERROR(retval);
                retval = bytes_getByteFromObj(pitem, &b);
                PM_RETURN_IF_ERROR(retval);
                pb->val[i] = b;
            }
            break;
    }

    *r_pobj = (pPmObj_t)pba;
    return retval;
}


PmReturn_t
bytearray_getItem(pPmObj_t pobj, int16_t index, pPmObj_t *r_pobj)
{
    PmReturn_t retval = PM_RET_OK;
    pPmBytearray_t pba;
    pPmBytes_t pb;
    int32_t n;

    pba = (pPmBytearray_t)pobj;

    /* Adjust a negative index */
    if (index < 0)
    {
        index += pba->length;
    }

    /* Check the bounds of the index */
    if ((index < 0) || (index >= pba->length))
    {
        PM_RAISE(retval, PM_RET_EX_INDX);
        return retval;
    }

    /* Create int from byte at index */
    pb = pba->val;
    n = (int32_t)pb->val[index];
    retval = int_new(n, r_pobj);

    return retval;
}


PmReturn_t
bytearray_setItem(pPmObj_t pba, int16_t index, pPmObj_t pobj)
{
    PmReturn_t retval;
    pPmBytes_t pb;
    uint8_t b = 0;

    /* Adjust a negative index */
    if (index < 0)
    {
        index += ((pPmBytearray_t)pba)->length;
    }

    /* Check the bounds of the index */
    if ((index < 0) || (index >= ((pPmBytearray_t)pba)->length))
    {
        PM_RAISE(retval, PM_RET_EX_INDX);
        return retval;
    }

    /* Set the item */
    retval = bytes_getByteFromObj(pobj, &b);
    pb = ((pPmBytearray_t)pba)->val;
    pb->val[index] = b;

    return retval;
}


PmReturn_t
bytearray_print(pPmObj_t pobj)
{
    PmReturn_t retval;
    pPmBytes_t pb;

    obj_print(PM_BYTEARRAY_STR, C_FALSE, C_FALSE);
    plat_putByte('(');
    plat_putByte('b');
    pb = ((pPmBytearray_t)pobj)->val;
    retval = string_printFormattedBytes(&(pb->val[0]),
                                        C_TRUE,
                                        ((pPmBytearray_t)pobj)->length);
    plat_putByte(')');
    return retval;
}
#endif /* HAVE_BYTEARRAY */
