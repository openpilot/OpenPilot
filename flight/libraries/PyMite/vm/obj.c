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
#define __FILE_ID__ 0x0F


/**
 * \file
 * \brief Object Type
 *
 * Object type operations.
 */


#include "pm.h"


PmReturn_t
obj_loadFromImg(PmMemSpace_t memspace,
                uint8_t const **paddr, pPmObj_t *r_pobj)
{
    PmReturn_t retval = PM_RET_OK;
    PmObj_t obj;


    /* Get the object descriptor */
    obj.od = (PmObjDesc_t)0x0000;
    OBJ_SET_TYPE(&obj, mem_getByte(memspace, paddr));

    switch (OBJ_GET_TYPE(&obj))
    {
        case OBJ_TYPE_NON:
            /* If it's the None object, return global None */
            *r_pobj = PM_NONE;
            break;

        case OBJ_TYPE_INT:
            /* Read an integer and create an integer object with the value */
            retval = int_new(mem_getInt(memspace, paddr), r_pobj);
            break;

#ifdef HAVE_FLOAT
        case OBJ_TYPE_FLT:
            /* Read a float and create an float object with the value */
            retval = float_new(mem_getFloat(memspace, paddr), r_pobj);
            break;
#endif /* HAVE_FLOAT */

        case OBJ_TYPE_STR:
            retval = string_loadFromImg(memspace, paddr, r_pobj);
            break;

        case OBJ_TYPE_TUP:
            retval = tuple_loadFromImg(memspace, paddr, r_pobj);
            break;

        case OBJ_TYPE_NIM:
            /* If it's a native code img, load into a code obj */
            retval = no_loadFromImg(memspace, paddr, r_pobj);
            break;

        case OBJ_TYPE_CIM:
            /* If it's a code img, load into a code obj */
            retval = co_loadFromImg(memspace, paddr, r_pobj);
            break;

        default:
            /* All other types should not be in an img obj */
            PM_RAISE(retval, PM_RET_EX_SYS);
            break;
    }
    return retval;
}


PmReturn_t
obj_loadFromImgObj(pPmObj_t pimg, pPmObj_t *r_pobj)
{
    uint8_t const *imgaddr;
    PmReturn_t retval;

    C_ASSERT(OBJ_GET_TYPE(pimg) == OBJ_TYPE_CIO);
    imgaddr = (uint8_t const *)&(((pPmCodeImgObj_t)pimg)->val);

    retval = obj_loadFromImg(MEMSPACE_RAM, &imgaddr, r_pobj);
    C_ASSERT(OBJ_GET_TYPE(*r_pobj) == OBJ_TYPE_COB);

    /* All COs must reference the top of the code img obj 
     * so the image is marked and prevented from being reclaimed */
    co_rSetCodeImgAddr((pPmCo_t)*r_pobj, (uint8_t const *)pimg);

    return retval;
}


/* Returns true if the obj is false */
int8_t
obj_isFalse(pPmObj_t pobj)
{
    C_ASSERT(pobj != C_NULL);

    switch (OBJ_GET_TYPE(pobj))
    {
        case OBJ_TYPE_NON:
            /* None evaluates to false, so return true */
            return C_TRUE;

        case OBJ_TYPE_INT:
            /* Only the integer zero is false */
            return ((pPmInt_t)pobj)->val == 0;

#ifdef HAVE_FLOAT
        case OBJ_TYPE_FLT:
            /* The floats 0.0 and -0.0 are false */
            return (((pPmFloat_t) pobj)->val == 0.0)
                || (((pPmFloat_t) pobj)->val == -0.0);
#endif /* HAVE_FLOAT */

        case OBJ_TYPE_STR:
            /* An empty string is false */
            return ((pPmString_t)pobj)->length == 0;

        case OBJ_TYPE_TUP:
            /* An empty tuple is false */
            return ((pPmTuple_t)pobj)->length == 0;

        case OBJ_TYPE_LST:
            /* An empty list is false */
            return ((pPmList_t)pobj)->length == 0;

        case OBJ_TYPE_DIC:
            /* An empty dict is false */
            return ((pPmDict_t)pobj)->length == 0;

        case OBJ_TYPE_BOOL:
            /* C int zero means false */
            return ((pPmBoolean_t) pobj)->val == 0;

        default:
            /*
             * The following types are always not false:
             * CodeObj, Function, Module, Class, ClassInstance.
             */
            return C_FALSE;
    }
}


/* Returns true if the item is in the container object */
PmReturn_t
obj_isIn(pPmObj_t pobj, pPmObj_t pitem)
{
    PmReturn_t retval = PM_RET_NO;
    pPmObj_t ptestItem;
    int16_t i;
    uint8_t c;

    switch (OBJ_GET_TYPE(pobj))
    {
        case OBJ_TYPE_TUP:
            /* Iterate over tuple to find item */
            for (i = 0; i < ((pPmTuple_t)pobj)->length; i++)
            {
                PM_RETURN_IF_ERROR(tuple_getItem(pobj, i, &ptestItem));

                if (obj_compare(pitem, ptestItem) == C_SAME)
                {
                    retval = PM_RET_OK;
                    break;
                }
            }
            break;

        case OBJ_TYPE_STR:
            /* Raise a TypeError if item is not a string */
            if ((OBJ_GET_TYPE(pitem) != OBJ_TYPE_STR))
            {
                retval = PM_RET_EX_TYPE;
                break;
            }

            /* Empty string is alway present */
            if (((pPmString_t)pitem)->length == 0)
            {
                retval = PM_RET_OK;
                break;
            }

            /* Raise a ValueError if the string is more than 1 char */
            else if (((pPmString_t)pitem)->length != 1)
            {
                retval = PM_RET_EX_VAL;
                break;
            }

            /* Iterate over string to find char */
            c = ((pPmString_t)pitem)->val[0];
            for (i = 0; i < ((pPmString_t)pobj)->length; i++)
            {
                if (c == ((pPmString_t)pobj)->val[i])
                {
                    retval = PM_RET_OK;
                    break;
                }
            }
            break;

        case OBJ_TYPE_LST:
            /* Iterate over list to find item */
            for (i = 0; i < ((pPmList_t)pobj)->length; i++)
            {
                PM_RETURN_IF_ERROR(list_getItem(pobj, i, &ptestItem));

                if (obj_compare(pitem, ptestItem) == C_SAME)
                {
                    retval = PM_RET_OK;
                    break;
                }
            }
            break;

        case OBJ_TYPE_DIC:
            /* Check if the item is one of the keys of the dict */
            retval = dict_getItem(pobj, pitem, &ptestItem);
            if (retval == PM_RET_EX_KEY)
            {
                retval = PM_RET_NO;
            }
            break;

        default:
            retval = PM_RET_EX_TYPE;
            break;
    }

    return retval;
}


int8_t
obj_compare(pPmObj_t pobj1, pPmObj_t pobj2)
{
#ifdef HAVE_BYTEARRAY
    PmReturn_t retval;
    pPmObj_t pobj;
#endif /* HAVE_BYTEARRAY */

    C_ASSERT(pobj1 != C_NULL);
    C_ASSERT(pobj2 != C_NULL);

    /* Check if pointers are same */
    if (pobj1 == pobj2)
    {
        return C_SAME;
    }

    /* If types are different, objs must differ */
    if (OBJ_GET_TYPE(pobj1) != OBJ_GET_TYPE(pobj2))
    {
        return C_DIFFER;
    }

#ifdef HAVE_BYTEARRAY
    /* If object is an instance, get the thing it contains */
    if (OBJ_GET_TYPE(pobj1) == OBJ_TYPE_CLI)
    {
        retval = dict_getItem((pPmObj_t)((pPmInstance_t)pobj1)->cli_attrs,
                              PM_NONE,
                              &pobj);
        PM_RETURN_IF_ERROR(retval);
        pobj1 = pobj;
    }
    if (OBJ_GET_TYPE(pobj2) == OBJ_TYPE_CLI)
    {
        retval = dict_getItem((pPmObj_t)((pPmInstance_t)pobj2)->cli_attrs,
                              PM_NONE,
                              &pobj);
        PM_RETURN_IF_ERROR(retval);
        pobj2 = pobj;
    }

    /* If types are different, objs must differ */
    if (OBJ_GET_TYPE(pobj1) != OBJ_GET_TYPE(pobj2))
    {
        return C_DIFFER;
    }
#endif /* HAVE_BYTEARRAY */

    /* Otherwise handle types individually */
    switch (OBJ_GET_TYPE(pobj1))
    {
        case OBJ_TYPE_NON:
            return C_SAME;

        case OBJ_TYPE_INT:
            return ((pPmInt_t)pobj1)->val ==
                ((pPmInt_t)pobj2)->val ? C_SAME : C_DIFFER;

#ifdef HAVE_FLOAT
        case OBJ_TYPE_FLT:
        {
            pPmObj_t r_pobj;

            float_compare(pobj1, pobj2, &r_pobj, COMP_EQ);
            return (r_pobj == PM_TRUE) ? C_SAME : C_DIFFER;
        }
#endif /* HAVE_FLOAT */

        case OBJ_TYPE_STR:
            return string_compare((pPmString_t)pobj1, (pPmString_t)pobj2);

        case OBJ_TYPE_TUP:
        case OBJ_TYPE_LST:
#ifdef HAVE_BYTEARRAY
        case OBJ_TYPE_BYA:
#endif /* HAVE_BYTEARRAY */
            return seq_compare(pobj1, pobj2);

        case OBJ_TYPE_DIC:
            /* #17: PyMite does not support Dict comparisons (yet) */
        default:
            break;
    }

    /* All other types would need same pointer to be true */
    return C_DIFFER;
}


#ifdef HAVE_PRINT
PmReturn_t
obj_print(pPmObj_t pobj, uint8_t is_expr_repr, uint8_t is_nested)
{
    PmReturn_t retval = PM_RET_OK;

    C_ASSERT(pobj != C_NULL);

    /* Something gets printed unless it's None in an unnested expression */
    if (!((OBJ_GET_TYPE(pobj) == OBJ_TYPE_NON) && is_expr_repr && !is_nested))
    {
        gVmGlobal.somethingPrinted = C_TRUE;
    }

    switch (OBJ_GET_TYPE(pobj))
    {
        case OBJ_TYPE_NON:
            if (!is_expr_repr || is_nested)
            {
                plat_putByte('N');
                plat_putByte('o');
                plat_putByte('n');
                retval = plat_putByte('e');
            }
            break;
        case OBJ_TYPE_INT:
            retval = int_print(pobj);
            break;
#ifdef HAVE_FLOAT
        case OBJ_TYPE_FLT:
            retval = float_print(pobj);
            break;
#endif /* HAVE_FLOAT */
        case OBJ_TYPE_STR:
            retval = string_print(pobj, (is_expr_repr || is_nested));
            break;
        case OBJ_TYPE_TUP:
            retval = tuple_print(pobj);
            break;
        case OBJ_TYPE_LST:
            retval = list_print(pobj);
            break;
        case OBJ_TYPE_DIC:
            retval = dict_print(pobj);
            break;
        case OBJ_TYPE_BOOL:
            if (((pPmBoolean_t) pobj)->val == C_TRUE)
            {
                plat_putByte('T');
                plat_putByte('r');
                plat_putByte('u');
            }
            else
            {
                plat_putByte('F');
                plat_putByte('a');
                plat_putByte('l');
                plat_putByte('s');
            }
            retval = plat_putByte('e');
            break;

        case OBJ_TYPE_CLI:
#ifdef HAVE_BYTEARRAY
            {
                pPmObj_t pobj2;

                retval = dict_getItem((pPmObj_t)((pPmInstance_t)pobj)->cli_attrs,
                                      PM_NONE,
                                      (pPmObj_t *)&pobj2);
                if ((retval == PM_RET_OK)
                    && (OBJ_GET_TYPE(pobj2) == OBJ_TYPE_BYA))
                {
                    retval = bytearray_print(pobj2);
                    break;
                }
            }
#endif /* HAVE_BYTEARRAY */

        case OBJ_TYPE_COB:
        case OBJ_TYPE_MOD:
        case OBJ_TYPE_CLO:
        case OBJ_TYPE_FXN:
        case OBJ_TYPE_CIM:
        case OBJ_TYPE_NIM:
        case OBJ_TYPE_NOB:
        case OBJ_TYPE_THR:
        case OBJ_TYPE_CIO:
        case OBJ_TYPE_MTH:
        case OBJ_TYPE_SQI:
            plat_putByte('<');
            plat_putByte('o');
            plat_putByte('b');
            plat_putByte('j');
            plat_putByte(' ');
            plat_putByte('t');
            plat_putByte('y');
            plat_putByte('p');
            plat_putByte('e');
            plat_putByte(' ');
            plat_putByte('0');
            plat_putByte('x');
            int_printHexByte(OBJ_GET_TYPE(pobj));
            plat_putByte(' ');
            plat_putByte('@');
            plat_putByte(' ');
            plat_putByte('0');
            plat_putByte('x');
            _int_printHex((intptr_t)pobj);
            retval = plat_putByte('>');
            break;

        default:
            /* Otherwise raise a TypeError */
            PM_RAISE(retval, PM_RET_EX_TYPE);
            break;
    }
    return retval;
}
#endif /* HAVE_PRINT */


#ifdef HAVE_BACKTICK
PmReturn_t
obj_repr(pPmObj_t pobj, pPmObj_t *r_pstr)
{
    uint8_t tBuffer[32];
    uint8_t bytesWritten = 0;
    PmReturn_t retval = PM_RET_OK;
    uint8_t const *pcstr = (uint8_t *)tBuffer;;

    C_ASSERT(pobj != C_NULL);

    switch (OBJ_GET_TYPE(pobj))
    {
        case OBJ_TYPE_INT:
            bytesWritten = snprintf((char *)&tBuffer, sizeof(tBuffer), "%li",
                                    (long)((pPmInt_t)pobj)->val);
            retval = string_new(&pcstr, r_pstr);
            break;

#ifdef HAVE_FLOAT
        case OBJ_TYPE_FLT:
            bytesWritten = snprintf((char *)&tBuffer, sizeof(tBuffer), "%f",
                                    ((pPmFloat_t)pobj)->val);
            retval = string_new(&pcstr, r_pstr);
            break;
#endif /* HAVE_FLOAT */

        default:
            /* Otherwise raise a TypeError */
            PM_RAISE(retval, PM_RET_EX_TYPE);
            break;
    }

    /* Sanity check */
    C_ASSERT(bytesWritten < sizeof(tBuffer));

    return retval;
}
#endif /* HAVE_BACKTICK */
