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
#define __FILE_ID__ 0x09


/**
 * \file
 * \brief VM Interpreter
 *
 * VM interpreter operations.
 */


#include "pm.h"


PmReturn_t
interpret(const uint8_t returnOnNoThreads)
{
    PmReturn_t retval = PM_RET_OK;
    pPmObj_t pobj1 = C_NULL;
    pPmObj_t pobj2 = C_NULL;
    pPmObj_t pobj3 = C_NULL;
    int16_t t16 = 0;
    int8_t t8 = 0;
    uint8_t bc;
    uint8_t objid, objid2;

    /* Activate a thread the first time */
    retval = interp_reschedule();
    PM_RETURN_IF_ERROR(retval);

    /* Interpret loop */
    for (;;)
    {
        if (gVmGlobal.pthread == C_NULL)
        {
            if (returnOnNoThreads)
            {
                /* User chose to return on no threads left */
                return retval;
            }

            /*
             * Without a frame there is nothing to execute, so reschedule
             * (possibly activating a recently added thread).
             */
            retval = interp_reschedule();
            PM_BREAK_IF_ERROR(retval);
            continue;
        }

        /* Reschedule threads if flag is true? */
        if (gVmGlobal.reschedule)
        {
            retval = interp_reschedule();
            PM_BREAK_IF_ERROR(retval);
        }

        /* Get byte; the func post-incrs PM_IP */
        bc = mem_getByte(PM_FP->fo_memspace, &PM_IP);
        switch (bc)
        {
            case POP_TOP:
                pobj1 = PM_POP();
                continue;

            case ROT_TWO:
                pobj1 = TOS;
                TOS = TOS1;
                TOS1 = pobj1;
                continue;

            case ROT_THREE:
                pobj1 = TOS;
                TOS = TOS1;
                TOS1 = TOS2;
                TOS2 = pobj1;
                continue;

            case DUP_TOP:
                pobj1 = TOS;
                PM_PUSH(pobj1);
                continue;

            case ROT_FOUR:
                pobj1 = TOS;
                TOS = TOS1;
                TOS1 = TOS2;
                TOS2 = TOS3;
                TOS3 = pobj1;
                continue;

            case NOP:
                continue;

            case UNARY_POSITIVE:
                /* Raise TypeError if TOS is not an int */
                if ((OBJ_GET_TYPE(TOS) != OBJ_TYPE_INT)
#ifdef HAVE_FLOAT
                    && (OBJ_GET_TYPE(TOS) != OBJ_TYPE_FLT)
#endif /* HAVE_FLOAT */
                    )
                {
                    PM_RAISE(retval, PM_RET_EX_TYPE);
                    break;
                }

                /* When TOS is an int, this is a no-op */
                continue;

            case UNARY_NEGATIVE:
#ifdef HAVE_FLOAT
                if (OBJ_GET_TYPE(TOS) == OBJ_TYPE_FLT)
                {
                    retval = float_negative(TOS, &pobj2);
                }
                else
#endif /* HAVE_FLOAT */
                {
                    retval = int_negative(TOS, &pobj2);
                }
                PM_BREAK_IF_ERROR(retval);
                TOS = pobj2;
                continue;

            case UNARY_NOT:
                pobj1 = PM_POP();
                if (obj_isFalse(pobj1))
                {
                    PM_PUSH(PM_TRUE);
                }
                else
                {
                    PM_PUSH(PM_FALSE);
                }
                continue;

#ifdef HAVE_BACKTICK
            /* #244 Add support for the backtick operation (UNARY_CONVERT) */
            case UNARY_CONVERT:
                retval = obj_repr(TOS, &pobj3);
                PM_BREAK_IF_ERROR(retval);
                TOS = pobj3;
                continue;
#endif /* HAVE_BACKTICK */

            case UNARY_INVERT:
                /* Raise TypeError if it's not an int */
                if (OBJ_GET_TYPE(TOS) != OBJ_TYPE_INT)
                {
                    PM_RAISE(retval, PM_RET_EX_TYPE);
                    break;
                }

                /* Otherwise perform bit-wise complement */
                retval = int_bitInvert(TOS, &pobj2);
                PM_BREAK_IF_ERROR(retval);
                TOS = pobj2;
                continue;

            case LIST_APPEND:
                /* list_append will raise a TypeError if TOS1 is not a list */
                retval = list_append(TOS1, TOS);
                PM_SP -= 2;
                continue;

            case BINARY_POWER:
            case INPLACE_POWER:

#ifdef HAVE_FLOAT
                if ((OBJ_GET_TYPE(TOS) == OBJ_TYPE_FLT)
                    || (OBJ_GET_TYPE(TOS1) == OBJ_TYPE_FLT))
                {
                    /* Calculate float power */
                    retval = float_op(TOS1, TOS, &pobj3, 'P');
                    PM_BREAK_IF_ERROR(retval);
                    PM_SP--;
                    TOS = pobj3;
                    continue;
                }
#endif /* HAVE_FLOAT */

                /* Calculate integer power */
                retval = int_pow(TOS1, TOS, &pobj3);
                PM_BREAK_IF_ERROR(retval);

                /* Set return value */
                PM_SP--;
                TOS = pobj3;
                continue;

            case GET_ITER:
#ifdef HAVE_GENERATORS
                /* Raise TypeError if TOS is an instance, but not iterable */
                if (OBJ_GET_TYPE(TOS) == OBJ_TYPE_CLI)
                {
                    retval = class_getAttr(TOS, PM_NEXT_STR, &pobj1);
                    if (retval != PM_RET_OK)
                    {
                        PM_RAISE(retval, PM_RET_EX_TYPE);
                        break;
                    }
                }
                else
#endif /* HAVE_GENERATORS */
                {
                    /* Convert sequence to sequence-iterator */
                    retval = seqiter_new(TOS, &pobj1);
                    PM_BREAK_IF_ERROR(retval);

                    /* Put sequence-iterator on top of stack */
                    TOS = pobj1;
                }
                continue;

            case BINARY_MULTIPLY:
            case INPLACE_MULTIPLY:
                /* If both objs are ints, perform the op */
                if ((OBJ_GET_TYPE(TOS) == OBJ_TYPE_INT)
                    && (OBJ_GET_TYPE(TOS1) == OBJ_TYPE_INT))
                {
                    retval = int_new(((pPmInt_t)TOS1)->val *
                                     ((pPmInt_t)TOS)->val, &pobj3);
                    PM_BREAK_IF_ERROR(retval);
                    PM_SP--;
                    TOS = pobj3;
                    continue;
                }

#ifdef HAVE_FLOAT
                else if ((OBJ_GET_TYPE(TOS) == OBJ_TYPE_FLT)
                         || (OBJ_GET_TYPE(TOS1) == OBJ_TYPE_FLT))
                {
                    retval = float_op(TOS1, TOS, &pobj3, '*');
                    PM_BREAK_IF_ERROR(retval);
                    PM_SP--;
                    TOS = pobj3;
                    continue;
                }
#endif /* HAVE_FLOAT */

#ifdef HAVE_REPLICATION
                /* If it's a list replication operation */
                else if ((OBJ_GET_TYPE(TOS) == OBJ_TYPE_INT)
                         && (OBJ_GET_TYPE(TOS1) == OBJ_TYPE_LST))
                {
                    t16 = (int16_t)((pPmInt_t)TOS)->val;
                    if (t16 < 0)
                    {
                        t16 = 0;
                    }

                    retval = list_replicate(TOS1, t16, &pobj3);
                    PM_BREAK_IF_ERROR(retval);
                    PM_SP--;
                    TOS = pobj3;
                    continue;
                }

                /* If it's a tuple replication operation */
                else if ((OBJ_GET_TYPE(TOS) == OBJ_TYPE_INT)
                         && (OBJ_GET_TYPE(TOS1) == OBJ_TYPE_TUP))
                {
                    t16 = (int16_t)((pPmInt_t)TOS)->val;
                    if (t16 < 0)
                    {
                        t16 = 0;
                    }

                    retval = tuple_replicate(TOS1, t16, &pobj3);
                    PM_BREAK_IF_ERROR(retval);
                    PM_SP--;
                    TOS = pobj3;
                    continue;
                }

                /* If it's a string replication operation */
                else if ((OBJ_GET_TYPE(TOS) == OBJ_TYPE_INT)
                         && (OBJ_GET_TYPE(TOS1) == OBJ_TYPE_STR))
                {
                    t16 = (int16_t)((pPmInt_t)TOS)->val;
                    if (t16 < 0)
                    {
                        t16 = 0;
                    }

                    pobj2 = TOS1;
                    pobj2 = (pPmObj_t)&((pPmString_t)pobj2)->val;
                    retval = string_replicate(
                        (uint8_t const **)(uint8_t *)&pobj2, t16, &pobj3);
                    PM_BREAK_IF_ERROR(retval);
                    PM_SP--;
                    TOS = pobj3;
                    continue;
                }
#endif /* HAVE_REPLICATION */

                /* Otherwise raise a TypeError */
                PM_RAISE(retval, PM_RET_EX_TYPE);
                break;

            case BINARY_DIVIDE:
            case INPLACE_DIVIDE:
            case BINARY_FLOOR_DIVIDE:
            case INPLACE_FLOOR_DIVIDE:

#ifdef HAVE_FLOAT
                if ((OBJ_GET_TYPE(TOS) == OBJ_TYPE_FLT)
                    || (OBJ_GET_TYPE(TOS1) == OBJ_TYPE_FLT))
                {
                    retval = float_op(TOS1, TOS, &pobj3, '/');
                    PM_BREAK_IF_ERROR(retval);
                    PM_SP--;
                    TOS = pobj3;
                    continue;
                }
#endif /* HAVE_FLOAT */

                /* Raise TypeError if args aren't ints */
                if ((OBJ_GET_TYPE(TOS) != OBJ_TYPE_INT)
                    || (OBJ_GET_TYPE(TOS1) != OBJ_TYPE_INT))
                {
                    PM_RAISE(retval, PM_RET_EX_TYPE);
                    break;
                }

                /* Raise ZeroDivisionError if denominator is zero */
                if (((pPmInt_t)TOS)->val == 0)
                {
                    PM_RAISE(retval, PM_RET_EX_ZDIV);
                    break;
                }

                /* Otherwise perform operation */
                retval = int_new(((pPmInt_t)TOS1)->val /
                                 ((pPmInt_t)TOS)->val, &pobj3);
                PM_BREAK_IF_ERROR(retval);
                PM_SP--;
                TOS = pobj3;
                continue;

            case BINARY_MODULO:
            case INPLACE_MODULO:

#ifdef HAVE_STRING_FORMAT
                /* If it's a string, perform string format */
                if (OBJ_GET_TYPE(TOS1) == OBJ_TYPE_STR)
                {
                    retval = string_format((pPmString_t)TOS1, TOS, &pobj3);
                    PM_BREAK_IF_ERROR(retval);
                    PM_SP--;
                    TOS = pobj3;
                    continue;
                }
#endif /* HAVE_STRING_FORMAT */

#ifdef HAVE_FLOAT
                if ((OBJ_GET_TYPE(TOS) == OBJ_TYPE_FLT)
                    || (OBJ_GET_TYPE(TOS1) == OBJ_TYPE_FLT))
                {
                    retval = float_op(TOS1, TOS, &pobj3, '%');
                    PM_BREAK_IF_ERROR(retval);
                    PM_SP--;
                    TOS = pobj3;
                    continue;
                }
#endif /* HAVE_FLOAT */

                /* Raise TypeError if args aren't ints */
                if ((OBJ_GET_TYPE(TOS) != OBJ_TYPE_INT)
                    || (OBJ_GET_TYPE(TOS1) != OBJ_TYPE_INT))
                {
                    PM_RAISE(retval, PM_RET_EX_TYPE);
                    break;
                }

                /* Raise ZeroDivisionError if denominator is zero */
                if (((pPmInt_t)TOS)->val == 0)
                {
                    PM_RAISE(retval, PM_RET_EX_ZDIV);
                    break;
                }

                /* Otherwise perform operation */
                retval = int_new(((pPmInt_t)TOS1)->val %
                                 ((pPmInt_t)TOS)->val, &pobj3);
                PM_BREAK_IF_ERROR(retval);
                PM_SP--;
                TOS = pobj3;
                continue;

            case STORE_MAP:
                /* #213: Add support for Python 2.6 bytecodes */
                C_ASSERT(OBJ_GET_TYPE(TOS2) == OBJ_TYPE_DIC);
                retval = dict_setItem(TOS2, TOS, TOS1);
                PM_BREAK_IF_ERROR(retval);
                PM_SP -= 2;
                continue;

            case BINARY_ADD:
            case INPLACE_ADD:

#ifdef HAVE_FLOAT
                if ((OBJ_GET_TYPE(TOS) == OBJ_TYPE_FLT)
                    || (OBJ_GET_TYPE(TOS1) == OBJ_TYPE_FLT))
                {
                    retval = float_op(TOS1, TOS, &pobj3, '+');
                    PM_BREAK_IF_ERROR(retval);
                    PM_SP--;
                    TOS = pobj3;
                    continue;
                }
#endif /* HAVE_FLOAT */

                /* If both objs are ints, perform the op */
                if ((OBJ_GET_TYPE(TOS) == OBJ_TYPE_INT)
                    && (OBJ_GET_TYPE(TOS1) == OBJ_TYPE_INT))
                {
                    retval = int_new(((pPmInt_t)TOS1)->val +
                                     ((pPmInt_t)TOS)->val, &pobj3);
                    PM_BREAK_IF_ERROR(retval);
                    PM_SP--;
                    TOS = pobj3;
                    continue;
                }

                /* #242: If both objs are strings, perform concatenation */
                if ((OBJ_GET_TYPE(TOS) == OBJ_TYPE_STR)
                    && (OBJ_GET_TYPE(TOS1) == OBJ_TYPE_STR))
                {
                    retval = string_concat((pPmString_t)TOS1,
                                           (pPmString_t)TOS,
                                           &pobj3);
                    PM_BREAK_IF_ERROR(retval);
                    PM_SP--;
                    TOS = pobj3;
                    continue;
                }

                /* Otherwise raise a TypeError */
                PM_RAISE(retval, PM_RET_EX_TYPE);
                break;

            case BINARY_SUBTRACT:
            case INPLACE_SUBTRACT:

#ifdef HAVE_FLOAT
                if ((OBJ_GET_TYPE(TOS) == OBJ_TYPE_FLT)
                    || (OBJ_GET_TYPE(TOS1) == OBJ_TYPE_FLT))
                {
                    retval = float_op(TOS1, TOS, &pobj3, '-');
                    PM_BREAK_IF_ERROR(retval);
                    PM_SP--;
                    TOS = pobj3;
                    continue;
                }
#endif /* HAVE_FLOAT */

                /* If both objs are ints, perform the op */
                if ((OBJ_GET_TYPE(TOS) == OBJ_TYPE_INT)
                    && (OBJ_GET_TYPE(TOS1) == OBJ_TYPE_INT))
                {
                    retval = int_new(((pPmInt_t)TOS1)->val -
                                     ((pPmInt_t)TOS)->val, &pobj3);
                    PM_BREAK_IF_ERROR(retval);
                    PM_SP--;
                    TOS = pobj3;
                    continue;
                }

                /* Otherwise raise a TypeError */
                PM_RAISE(retval, PM_RET_EX_TYPE);
                break;

            case BINARY_SUBSCR:
                /* Implements TOS = TOS1[TOS]. */

                if (OBJ_GET_TYPE(TOS1) == OBJ_TYPE_DIC)
                {
                    retval = dict_getItem(TOS1, TOS, &pobj3);
                }
                else
                {
                    /* Raise a TypeError if index is not an Integer or Bool */
                    if ((OBJ_GET_TYPE(TOS) != OBJ_TYPE_INT)
                        && (OBJ_GET_TYPE(TOS) != OBJ_TYPE_BOOL))
                    {
                        PM_RAISE(retval, PM_RET_EX_TYPE);
                        break;
                    }

                    pobj1 = TOS1;
#ifdef HAVE_BYTEARRAY
                    /* If object is an instance, get the thing it contains */
                    if (OBJ_GET_TYPE(pobj1) == OBJ_TYPE_CLI)
                    {
                        retval = dict_getItem((pPmObj_t)((pPmInstance_t)pobj1)->cli_attrs,
                                              PM_NONE,
                                              &pobj2);
                        PM_RETURN_IF_ERROR(retval);
                        pobj1 = pobj2;
                    }
#endif /* HAVE_BYTEARRAY */

                    /* Ensure the index doesn't overflow */
                    C_ASSERT(((pPmInt_t)TOS)->val <= 0x0000FFFF);
                    t16 = (int16_t)((pPmInt_t)TOS)->val;

                    retval = seq_getSubscript(pobj1, t16, &pobj3);
                }
                PM_BREAK_IF_ERROR(retval);
                PM_SP--;
                TOS = pobj3;
                continue;

#ifdef HAVE_FLOAT
            /* #213: Add support for Python 2.6 bytecodes */
            case BINARY_TRUE_DIVIDE:
            case INPLACE_TRUE_DIVIDE:

                /* Perform division; float_op() checks for types and zero-div */
                retval = float_op(TOS1, TOS, &pobj3, '/');
                PM_BREAK_IF_ERROR(retval);
                PM_SP--;
                TOS = pobj3;
                continue;
#endif /* HAVE_FLOAT */

            case SLICE_0:
                /* Implements TOS = TOS[:], push a copy of the sequence */

                /* Create a copy if it is a list */
                if (OBJ_GET_TYPE(TOS) == OBJ_TYPE_LST)
                {
                    retval = list_copy(TOS, &pobj2);
                    PM_BREAK_IF_ERROR(retval);

                    TOS = pobj2;
                }

                /* If TOS is an immutable sequence leave it (no op) */

                /* Raise a TypeError for types that can not be sliced */
                else if ((OBJ_GET_TYPE(TOS) != OBJ_TYPE_STR)
                         && (OBJ_GET_TYPE(TOS) != OBJ_TYPE_TUP))
                {
                    PM_RAISE(retval, PM_RET_EX_TYPE);
                    break;
                }
                continue;

            case STORE_SUBSCR:
                /* Implements TOS1[TOS] = TOS2 */

                /* If it's a list */
                if (OBJ_GET_TYPE(TOS1) == OBJ_TYPE_LST)
                {
                    /* Ensure subscr is an int or bool */
                    if ((OBJ_GET_TYPE(TOS) != OBJ_TYPE_INT)
                        && (OBJ_GET_TYPE(TOS) != OBJ_TYPE_BOOL))
                    {
                        PM_RAISE(retval, PM_RET_EX_TYPE);
                        break;
                    }

                    /* Set the list item */
                    retval = list_setItem(TOS1,
                                          (int16_t)(((pPmInt_t)TOS)->val),
                                          TOS2);
                    PM_BREAK_IF_ERROR(retval);
                    PM_SP -= 3;
                    continue;
                }

                /* If it's a dict */
                if (OBJ_GET_TYPE(TOS1) == OBJ_TYPE_DIC)
                {
                    /* Set the dict item */
                    retval = dict_setItem(TOS1, TOS, TOS2);
                    PM_BREAK_IF_ERROR(retval);
                    PM_SP -= 3;
                    continue;
                }

#ifdef HAVE_BYTEARRAY
                /* If object is an instance, get the thing it contains */
                if (OBJ_GET_TYPE(TOS1) == OBJ_TYPE_CLI)
                {
                    retval = dict_getItem((pPmObj_t)((pPmInstance_t)TOS1)->cli_attrs,
                                          PM_NONE,
                                          &pobj2);

                    /* Raise TypeError if instance isn't a ByteArray */
                    if ((retval == PM_RET_EX_KEY)
                        || (OBJ_GET_TYPE(pobj2) != OBJ_TYPE_BYA))
                    {
                        PM_RAISE(retval, PM_RET_EX_TYPE);
                        break;
                    }
                    PM_BREAK_IF_ERROR(retval);

                    /* Ensure subscr is an int or bool */
                    if ((OBJ_GET_TYPE(TOS) != OBJ_TYPE_INT)
                        && (OBJ_GET_TYPE(TOS) != OBJ_TYPE_BOOL))
                    {
                        PM_RAISE(retval, PM_RET_EX_TYPE);
                        break;
                    }

                    retval = bytearray_setItem(pobj2,
                                               (int16_t)(((pPmInt_t)TOS)->val),
                                               TOS2);
                    PM_BREAK_IF_ERROR(retval);
                    PM_SP -= 3;
                    continue;
                }
#endif /* HAVE_BYTEARRAY */

                /* TypeError for all else */
                PM_RAISE(retval, PM_RET_EX_TYPE);
                break;

#ifdef HAVE_DEL
            case DELETE_SUBSCR:

                if ((OBJ_GET_TYPE(TOS1) == OBJ_TYPE_LST)
                    && (OBJ_GET_TYPE(TOS) == OBJ_TYPE_INT))
                {
                    retval = list_delItem(TOS1,
                                          (int16_t)((pPmInt_t)TOS)->val);
                }

                else if ((OBJ_GET_TYPE(TOS1) == OBJ_TYPE_DIC)
                         && (OBJ_GET_TYPE(TOS) <= OBJ_TYPE_HASHABLE_MAX))
                {
                    retval = dict_delItem(TOS1, TOS);
                }

                /* Raise TypeError if obj is not a list or dict */
                else
                {
                    PM_RAISE(retval, PM_RET_EX_TYPE);
                }

                PM_BREAK_IF_ERROR(retval);
                PM_SP -= 2;
                continue;
#endif /* HAVE_DEL */

            case BINARY_LSHIFT:
            case INPLACE_LSHIFT:
                /* If both objs are ints, perform the op */
                if ((OBJ_GET_TYPE(TOS) == OBJ_TYPE_INT)
                    && (OBJ_GET_TYPE(TOS1) == OBJ_TYPE_INT))
                {
                    retval = int_new(((pPmInt_t)TOS1)->val <<
                                     ((pPmInt_t)TOS)->val, &pobj3);
                    PM_BREAK_IF_ERROR(retval);
                    PM_SP--;
                    TOS = pobj3;
                    continue;
                }

                /* Otherwise raise a TypeError */
                PM_RAISE(retval, PM_RET_EX_TYPE);
                break;

            case BINARY_RSHIFT:
            case INPLACE_RSHIFT:
                /* If both objs are ints, perform the op */
                if ((OBJ_GET_TYPE(TOS) == OBJ_TYPE_INT)
                    && (OBJ_GET_TYPE(TOS1) == OBJ_TYPE_INT))
                {
                    retval = int_new(((pPmInt_t)TOS1)->val >>
                                     ((pPmInt_t)TOS)->val, &pobj3);
                    PM_BREAK_IF_ERROR(retval);
                    PM_SP--;
                    TOS = pobj3;
                    continue;
                }

                /* Otherwise raise a TypeError */
                PM_RAISE(retval, PM_RET_EX_TYPE);
                break;

            case BINARY_AND:
            case INPLACE_AND:
                /* If both objs are ints, perform the op */
                if ((OBJ_GET_TYPE(TOS) == OBJ_TYPE_INT)
                    && (OBJ_GET_TYPE(TOS1) == OBJ_TYPE_INT))
                {
                    retval = int_new(((pPmInt_t)TOS1)->val &
                                     ((pPmInt_t)TOS)->val, &pobj3);
                    PM_BREAK_IF_ERROR(retval);
                    PM_SP--;
                    TOS = pobj3;
                    continue;
                }

                /* Otherwise raise a TypeError */
                PM_RAISE(retval, PM_RET_EX_TYPE);
                break;

            case BINARY_XOR:
            case INPLACE_XOR:
                /* If both objs are ints, perform the op */
                if ((OBJ_GET_TYPE(TOS) == OBJ_TYPE_INT)
                    && (OBJ_GET_TYPE(TOS1) == OBJ_TYPE_INT))
                {
                    retval = int_new(((pPmInt_t)TOS1)->val ^
                                     ((pPmInt_t)TOS)->val, &pobj3);
                    PM_BREAK_IF_ERROR(retval);
                    PM_SP--;
                    TOS = pobj3;
                    continue;
                }

                /* Otherwise raise a TypeError */
                PM_RAISE(retval, PM_RET_EX_TYPE);
                break;

            case BINARY_OR:
            case INPLACE_OR:
                /* If both objs are ints, perform the op */
                if ((OBJ_GET_TYPE(TOS) == OBJ_TYPE_INT)
                    && (OBJ_GET_TYPE(TOS1) == OBJ_TYPE_INT))
                {
                    retval = int_new(((pPmInt_t)TOS1)->val |
                                     ((pPmInt_t)TOS)->val, &pobj3);
                    PM_BREAK_IF_ERROR(retval);
                    PM_SP--;
                    TOS = pobj3;
                    continue;
                }

                /* Otherwise raise a TypeError */
                PM_RAISE(retval, PM_RET_EX_TYPE);
                break;

#ifdef HAVE_PRINT
            case PRINT_EXPR:
                /* Print interactive expression */
                /* Fallthrough */

            case PRINT_ITEM:
                if (gVmGlobal.needSoftSpace && (bc == PRINT_ITEM))
                {
                    retval = plat_putByte(' ');
                    PM_BREAK_IF_ERROR(retval);
                }
                gVmGlobal.needSoftSpace = C_TRUE;

                /* Print out topmost stack element */
                retval = obj_print(TOS, (uint8_t)(bc == PRINT_EXPR), C_FALSE);
                PM_BREAK_IF_ERROR(retval);
                PM_SP--;
                if (bc != PRINT_EXPR)
                {
                    continue;
                }
                /* If PRINT_EXPR, Fallthrough to print a newline */

            case PRINT_NEWLINE:
                gVmGlobal.needSoftSpace = C_FALSE;
                if (gVmGlobal.somethingPrinted)
                {
                    retval = plat_putByte('\n');
                    gVmGlobal.somethingPrinted = C_FALSE;
                }
                PM_BREAK_IF_ERROR(retval);
                continue;
#endif /* HAVE_PRINT */

            case BREAK_LOOP:
            {
                pPmBlock_t pb1 = PM_FP->fo_blockstack;

                /* Ensure there's a block */
                C_ASSERT(pb1 != C_NULL);

                /* Delete blocks until first loop block */
                while ((pb1->b_type != B_LOOP) && (pb1->next != C_NULL))
                {
                    pobj2 = (pPmObj_t)pb1;
                    pb1 = pb1->next;
                    retval = heap_freeChunk(pobj2);
                    PM_BREAK_IF_ERROR(retval);
                }

                /* Test again outside while loop */
                PM_BREAK_IF_ERROR(retval);

                /* Restore PM_SP */
                PM_SP = pb1->b_sp;

                /* Goto handler */
                PM_IP = pb1->b_handler;

                /* Pop and delete this block */
                PM_FP->fo_blockstack = pb1->next;
                retval = heap_freeChunk((pPmObj_t)pb1);
                PM_BREAK_IF_ERROR(retval);
            }
                continue;

            case LOAD_LOCALS:
                /* Pushes local attrs dict of current frame */
                /* WARNING: does not copy fo_locals to attrs */
                PM_PUSH((pPmObj_t)PM_FP->fo_attrs);
                continue;

            case RETURN_VALUE:
                /* Get expiring frame's TOS */
                pobj2 = PM_POP();

#if 0 /*__DEBUG__*/
                /* #251: This safety check is disabled because it breaks ipm */
                /* #109: Check that stack should now be empty */
                /* If this is regular frame (not native and not a generator) */
                if ((PM_FP != (pPmFrame_t)(&gVmGlobal.nativeframe)) &&
                    !(PM_FP->fo_func->f_co->co_flags & CO_GENERATOR))
                {
                    /* An empty stack points one past end of locals */
                    t8 = PM_FP->fo_func->f_co->co_nlocals;
                    C_ASSERT(PM_SP == &(PM_FP->fo_locals[t8]));
                }
#endif /* __DEBUG__ */

                /* Keep ref of expiring frame */
                pobj1 = (pPmObj_t)PM_FP;
                C_ASSERT(OBJ_GET_TYPE(pobj1) == OBJ_TYPE_FRM);

                /* If no previous frame, quit thread */
                if (PM_FP->fo_back == C_NULL)
                {
                    gVmGlobal.pthread->interpctrl = INTERP_CTRL_EXIT;
                    retval = PM_RET_OK;
                    break;
                }

                /* Otherwise return to previous frame */
                PM_FP = PM_FP->fo_back;

#ifdef HAVE_GENERATORS
                /* If returning function was a generator */
                if (((pPmFrame_t)pobj1)->fo_func->f_co->co_flags & CO_GENERATOR)
                {
                    /* Raise a StopIteration exception */
                    PM_RAISE(retval, PM_RET_EX_STOP);
                    break;
                }
#endif /* HAVE_GENERATORS */

#ifdef HAVE_CLASSES
                /*
                 * If returning function was class initializer
                 * do not push a return object
                 */
                if (((pPmFrame_t)pobj1)->fo_isInit)
                {
                    /* Raise TypeError if __init__ did not return None */
                    if (pobj2 != PM_NONE)
                    {
                        PM_RAISE(retval, PM_RET_EX_TYPE);
                        break;
                    }
                }
                else
#endif /* HAVE_CLASSES */

                /*
                 * Push frame's return val, except if the expiring frame
                 * was due to an import statement
                 */
                if (!(((pPmFrame_t)pobj1)->fo_isImport))
                {
                    PM_PUSH(pobj2);
                }

                /* Deallocate expired frame */
                PM_BREAK_IF_ERROR(heap_freeChunk(pobj1));
                continue;

#ifdef HAVE_IMPORTS
            case IMPORT_STAR:
                /* #102: Implement the remaining IMPORT_ bytecodes */
                /* Expect a module on the top of the stack */
                C_ASSERT(OBJ_GET_TYPE(TOS) == OBJ_TYPE_MOD);

                /* Update PM_FP's attrs with those of the module on the stack */
                retval = dict_update((pPmObj_t)PM_FP->fo_attrs,
                                     (pPmObj_t)((pPmFunc_t)TOS)->f_attrs);
                PM_BREAK_IF_ERROR(retval);
                PM_SP--;
                continue;
#endif /* HAVE_IMPORTS */

#ifdef HAVE_GENERATORS
            case YIELD_VALUE:
                /* #207: Add support for the yield keyword */
                /* Get expiring frame's TOS */
                pobj1 = PM_POP();

                /* Raise TypeError if __init__ did not return None */
                /* (Yield means this is a generator) */
                if ((PM_FP)->fo_isInit)
                {
                    PM_RAISE(retval, PM_RET_EX_TYPE);
                    break;
                }

                /* Return to previous frame */
                PM_FP = PM_FP->fo_back;

                /* Push yield value onto caller's TOS */
                PM_PUSH(pobj1);
                continue;
#endif /* HAVE_GENERATORS */

            case POP_BLOCK:
                /* Get ptr to top block */
                pobj1 = (pPmObj_t)PM_FP->fo_blockstack;

                /* If there's no block, raise SystemError */
                C_ASSERT(pobj1 != C_NULL);

                /* Pop block */
                PM_FP->fo_blockstack = PM_FP->fo_blockstack->next;

                /* Set stack to previous level, jump to code outside block */
                PM_SP = ((pPmBlock_t)pobj1)->b_sp;
                PM_IP = ((pPmBlock_t)pobj1)->b_handler;

                PM_BREAK_IF_ERROR(heap_freeChunk(pobj1));
                continue;

#ifdef HAVE_CLASSES
            case BUILD_CLASS:
                /* Create and push new class */
                retval = class_new(TOS, TOS1, TOS2, &pobj2);
                PM_BREAK_IF_ERROR(retval);
                PM_SP -= 2;
                TOS = pobj2;
                continue;
#endif /* HAVE_CLASSES */


            /***************************************************
             * All bytecodes after 90 (0x5A) have a 2-byte arg
             * that needs to be swallowed using GET_ARG().
             **************************************************/

            case STORE_NAME:
                /* Get name index */
                t16 = GET_ARG();

                /* Get key */
                pobj2 = PM_FP->fo_func->f_co->co_names->val[t16];

                /* Set key=val in current frame's attrs dict */
                retval = dict_setItem((pPmObj_t)PM_FP->fo_attrs, pobj2, TOS);
                PM_BREAK_IF_ERROR(retval);
                PM_SP--;
                continue;

#ifdef HAVE_DEL
            case DELETE_NAME:
                /* Get name index */
                t16 = GET_ARG();

                /* Get key */
                pobj2 = PM_FP->fo_func->f_co->co_names->val[t16];

                /* Remove key,val pair from current frame's attrs dict */
                retval = dict_delItem((pPmObj_t)PM_FP->fo_attrs, pobj2);
                PM_BREAK_IF_ERROR(retval);
                continue;
#endif /* HAVE_DEL */

            case UNPACK_SEQUENCE:
                /* Get ptr to sequence */
                pobj1 = PM_POP();

#ifdef HAVE_BYTEARRAY
                /* If object is an instance, get the thing it contains */
                if (OBJ_GET_TYPE(pobj1) == OBJ_TYPE_CLI)
                {
                    retval = dict_getItem((pPmObj_t)((pPmInstance_t)pobj1)->cli_attrs,
                                          PM_NONE,
                                          &pobj2);
                    PM_RETURN_IF_ERROR(retval);
                    pobj1 = pobj2;
                }
#endif /* HAVE_BYTEARRAY */

                /*
                 * Get the length of the sequence; this will
                 * raise TypeError if obj is not a sequence.
                 *
                 * #59: Unpacking to a Dict shall not be supported
                 */
                retval = seq_getLength(pobj1, &t16);
                if (retval != PM_RET_OK)
                {
                    GET_ARG();
                    break;
                }

                /* Raise ValueError if seq length does not match num args */
                if (t16 != GET_ARG())
                {
                    PM_RAISE(retval, PM_RET_EX_VAL);
                    break;
                }

                /* Push sequence's objs onto stack */
                for (; --t16 >= 0;)
                {
                    retval = seq_getSubscript(pobj1, t16, &pobj2);
                    PM_BREAK_IF_ERROR(retval);
                    PM_PUSH(pobj2);
                }

                /* Test again outside the for loop */
                PM_BREAK_IF_ERROR(retval);
                continue;

            case FOR_ITER:
                t16 = GET_ARG();

#ifdef HAVE_GENERATORS
                /* If TOS is an instance, call next method */
                if (OBJ_GET_TYPE(TOS) == OBJ_TYPE_CLI)
                {
                    /* Get the next() func */
                    retval = class_getAttr(TOS, PM_NEXT_STR, &pobj1);
                    PM_BREAK_IF_ERROR(retval);

                    /* Push the func and instance as an arg */
                    pobj2 = TOS;
                    PM_PUSH(pobj1);
                    PM_PUSH(pobj2);
                    t16 = 1;

                    /* Ensure pobj1 is the func */
                    goto CALL_FUNC_FOR_ITER;
                }
                else
#endif /* HAVE_GENERATORS */
                {
                    /* Get the next item in the sequence iterator */
                    retval = seqiter_getNext(TOS, &pobj2);
                }

                /* Catch StopIteration early: pop iterator and break loop */
                if (retval == PM_RET_EX_STOP)
                {
                    PM_SP--;
                    retval = PM_RET_OK;
                    PM_IP += t16;
                    continue;
                }
                PM_BREAK_IF_ERROR(retval);

                /* Push the next item onto the stack */
                PM_PUSH(pobj2);
                continue;

            case STORE_ATTR:
                /* TOS.name = TOS1 */
                /* Get names index */
                t16 = GET_ARG();

                /* Get attrs dict from obj */
                if ((OBJ_GET_TYPE(TOS) == OBJ_TYPE_FXN)
                    || (OBJ_GET_TYPE(TOS) == OBJ_TYPE_MOD))
                {
                    pobj2 = (pPmObj_t)((pPmFunc_t)TOS)->f_attrs;
                }

#ifdef HAVE_CLASSES
                else if (OBJ_GET_TYPE(TOS) == OBJ_TYPE_CLO)
                {
                    pobj2 = (pPmObj_t)((pPmClass_t)TOS)->cl_attrs;
                }
                else if (OBJ_GET_TYPE(TOS) == OBJ_TYPE_CLI)
                {
                    pobj2 = (pPmObj_t)((pPmInstance_t)TOS)->cli_attrs;
                }
                else if (OBJ_GET_TYPE(TOS) == OBJ_TYPE_MTH)
                {
                    pobj2 = (pPmObj_t)((pPmMethod_t)TOS)->m_attrs;
                }
#endif /* HAVE_CLASSES */

                /* Other types result in an AttributeError */
                else
                {
                    PM_RAISE(retval, PM_RET_EX_ATTR);
                    break;
                }

                /* If attrs is not a dict, raise SystemError */
                if (OBJ_GET_TYPE(pobj2) != OBJ_TYPE_DIC)
                {
                    PM_RAISE(retval, PM_RET_EX_SYS);
                    break;
                }

                /* Get name/key obj */
                pobj3 = PM_FP->fo_func->f_co->co_names->val[t16];

                /* Set key=val in obj's dict */
                retval = dict_setItem(pobj2, pobj3, TOS1);
                PM_BREAK_IF_ERROR(retval);
                PM_SP -= 2;
                continue;

#ifdef HAVE_DEL
            case DELETE_ATTR:
                /* del TOS.name */
                /* Get names index */
                t16 = GET_ARG();

                /* Get attrs dict from obj */
                if ((OBJ_GET_TYPE(TOS) == OBJ_TYPE_FXN)
                    || (OBJ_GET_TYPE(TOS) == OBJ_TYPE_MOD))
                {
                    pobj2 = (pPmObj_t)((pPmFunc_t)TOS)->f_attrs;
                }

#ifdef HAVE_CLASSES
                else if (OBJ_GET_TYPE(TOS) == OBJ_TYPE_CLO)
                {
                    pobj2 = (pPmObj_t)((pPmClass_t)TOS)->cl_attrs;
                }
                else if (OBJ_GET_TYPE(TOS) == OBJ_TYPE_CLI)
                {
                    pobj2 = (pPmObj_t)((pPmInstance_t)TOS)->cli_attrs;
                }
                else if (OBJ_GET_TYPE(TOS) == OBJ_TYPE_MTH)
                {
                    pobj2 = (pPmObj_t)((pPmMethod_t)TOS)->m_attrs;
                }
#endif /* HAVE_CLASSES */

                /* Other types result in an AttributeError */
                else
                {
                    PM_RAISE(retval, PM_RET_EX_ATTR);
                    break;
                }

                /* If attrs is not a dict, raise SystemError */
                if (OBJ_GET_TYPE(pobj2) != OBJ_TYPE_DIC)
                {
                    PM_RAISE(retval, PM_RET_EX_SYS);
                    break;
                }

                /* Get name/key obj */
                pobj3 = PM_FP->fo_func->f_co->co_names->val[t16];

                /* Remove key,val from obj's dict */
                retval = dict_delItem(pobj2, pobj3);

                /* Raise an AttributeError if key is not found */
                if (retval == PM_RET_EX_KEY)
                {
                    PM_RAISE(retval, PM_RET_EX_ATTR);
                }

                PM_BREAK_IF_ERROR(retval);
                PM_SP--;
                continue;
#endif /* HAVE_DEL */

            case STORE_GLOBAL:
                /* Get name index */
                t16 = GET_ARG();

                /* Get key */
                pobj2 = PM_FP->fo_func->f_co->co_names->val[t16];

                /* Set key=val in global dict */
                retval = dict_setItem((pPmObj_t)PM_FP->fo_globals, pobj2, TOS);
                PM_BREAK_IF_ERROR(retval);
                PM_SP--;
                continue;

#ifdef HAVE_DEL
            case DELETE_GLOBAL:
                /* Get name index */
                t16 = GET_ARG();

                /* Get key */
                pobj2 = PM_FP->fo_func->f_co->co_names->val[t16];

                /* Remove key,val from globals */
                retval = dict_delItem((pPmObj_t)PM_FP->fo_globals, pobj2);
                PM_BREAK_IF_ERROR(retval);
                continue;
#endif /* HAVE_DEL */

            case DUP_TOPX:
                t16 = GET_ARG();
                C_ASSERT(t16 <= 3);

                pobj1 = TOS;
                pobj2 = TOS1;
                pobj3 = TOS2;
                if (t16 >= 3)
                    PM_PUSH(pobj3);
                if (t16 >= 2)
                    PM_PUSH(pobj2);
                if (t16 >= 1)
                    PM_PUSH(pobj1);
                continue;

            case LOAD_CONST:
                /* Get const's index in CO */
                t16 = GET_ARG();

                /* Push const on stack */
                PM_PUSH(PM_FP->fo_func->f_co->co_consts->val[t16]);
                continue;

            case LOAD_NAME:
                /* Get name index */
                t16 = GET_ARG();

                /* Get name from names tuple */
                pobj1 = PM_FP->fo_func->f_co->co_names->val[t16];

                /* Get value from frame's attrs dict */
                retval = dict_getItem((pPmObj_t)PM_FP->fo_attrs, pobj1, &pobj2);
                if (retval == PM_RET_EX_KEY)
                {
                    /* Get val from globals */
                    retval = dict_getItem((pPmObj_t)PM_FP->fo_globals,
                                          pobj1, &pobj2);

                    /* Check for name in the builtins module if it is loaded */
                    if ((retval == PM_RET_EX_KEY) && (PM_PBUILTINS != C_NULL))
                    {
                        /* Get val from builtins */
                        retval = dict_getItem(PM_PBUILTINS, pobj1, &pobj2);
                        if (retval == PM_RET_EX_KEY)
                        {
                            /* Name not defined, raise NameError */
                            PM_RAISE(retval, PM_RET_EX_NAME);
                            break;
                        }
                    }
                }
                PM_BREAK_IF_ERROR(retval);
                PM_PUSH(pobj2);
                continue;

            case BUILD_TUPLE:
                /* Get num items */
                t16 = GET_ARG();
                retval = tuple_new(t16, &pobj1);
                PM_BREAK_IF_ERROR(retval);

                /* Fill tuple with ptrs to objs */
                for (; --t16 >= 0;)
                {
                    ((pPmTuple_t)pobj1)->val[t16] = PM_POP();
                }
                PM_PUSH(pobj1);
                continue;

            case BUILD_LIST:
                t16 = GET_ARG();
                retval = list_new(&pobj1);
                PM_BREAK_IF_ERROR(retval);
                for (; --t16 >= 0;)
                {
                    /* Insert obj into list */
                    heap_gcPushTempRoot(pobj1, &objid);
                    retval = list_insert(pobj1, 0, TOS);
                    heap_gcPopTempRoot(objid);
                    PM_BREAK_IF_ERROR(retval);
                    PM_SP--;
                }
                /* Test again outside for loop */
                PM_BREAK_IF_ERROR(retval);

                /* push list onto stack */
                PM_PUSH(pobj1);
                continue;

            case BUILD_MAP:
                /* Argument is ignored */
                t16 = GET_ARG();
                retval = dict_new(&pobj1);
                PM_BREAK_IF_ERROR(retval);
                PM_PUSH(pobj1);
                continue;

            case LOAD_ATTR:
                /* Implements TOS.attr */
                t16 = GET_ARG();

                /* Get attrs dict from obj */
                if ((OBJ_GET_TYPE(TOS) == OBJ_TYPE_FXN) ||
                    (OBJ_GET_TYPE(TOS) == OBJ_TYPE_MOD))
                {
                    pobj1 = (pPmObj_t)((pPmFunc_t)TOS)->f_attrs;
                }

#ifdef HAVE_CLASSES
                else if (OBJ_GET_TYPE(TOS) == OBJ_TYPE_CLO)
                {
                    pobj1 = (pPmObj_t)((pPmClass_t)TOS)->cl_attrs;
                }
                else if (OBJ_GET_TYPE(TOS) == OBJ_TYPE_CLI)
                {
                    pobj1 = (pPmObj_t)((pPmInstance_t)TOS)->cli_attrs;
                }
                else if (OBJ_GET_TYPE(TOS) == OBJ_TYPE_MTH)
                {
                    pobj1 = (pPmObj_t)((pPmMethod_t)TOS)->m_attrs;
                }
#endif /* HAVE_CLASSES */

                /* Other types result in an AttributeError */
                else
                {
                    PM_RAISE(retval, PM_RET_EX_ATTR);
                    break;
                }

                /* If attrs is not a dict, raise SystemError */
                if (OBJ_GET_TYPE(pobj1) != OBJ_TYPE_DIC)
                {
                    PM_RAISE(retval, PM_RET_EX_SYS);
                    break;
                }

                /* Get name */
                pobj2 = PM_FP->fo_func->f_co->co_names->val[t16];

                /* Get attr with given name */
                retval = dict_getItem(pobj1, pobj2, &pobj3);

#ifdef HAVE_CLASSES
                /*
                 * If attr is not found and object is a class or instance,
                 * try to get the attribute from the class attrs or parent(s)
                 */
                if ((retval == PM_RET_EX_KEY) &&
                    ((OBJ_GET_TYPE(TOS) == OBJ_TYPE_CLO)
                        || (OBJ_GET_TYPE(TOS) == OBJ_TYPE_CLI)))
                {
                    retval = class_getAttr(TOS, pobj2, &pobj3);
                }
#endif /* HAVE_CLASSES */

                /* Raise an AttributeError if key is not found */
                if (retval == PM_RET_EX_KEY)
                {
                    PM_RAISE(retval, PM_RET_EX_ATTR);
                }
                PM_BREAK_IF_ERROR(retval);

#ifdef HAVE_CLASSES
                /* If obj is an instance and attr is a func, create method */
                if ((OBJ_GET_TYPE(TOS) == OBJ_TYPE_CLI) &&
                    (OBJ_GET_TYPE(pobj3) == OBJ_TYPE_FXN))
                {
                    pobj2 = pobj3;
                    retval = class_method(TOS, pobj2, &pobj3);
                    PM_BREAK_IF_ERROR(retval);
                }
#endif /* HAVE_CLASSES */

                /* Put attr on the stack */
                TOS = pobj3;
                continue;

            case COMPARE_OP:
                retval = PM_RET_OK;
                t16 = GET_ARG();

#ifdef HAVE_FLOAT
                if ((OBJ_GET_TYPE(TOS) == OBJ_TYPE_FLT)
                    || (OBJ_GET_TYPE(TOS1) == OBJ_TYPE_FLT))
                {
                    retval = float_compare(TOS1, TOS, &pobj3, (PmCompare_t)t16);
                    PM_SP--;
                    TOS = pobj3;
                    continue;
                }
#endif /* HAVE_FLOAT */

                /* Handle all integer-to-integer (or bool) comparisons */
                if (((OBJ_GET_TYPE(TOS) == OBJ_TYPE_INT)
                     || (OBJ_GET_TYPE(TOS) == OBJ_TYPE_BOOL))
                    && ((OBJ_GET_TYPE(TOS1) == OBJ_TYPE_INT)
                        || (OBJ_GET_TYPE(TOS1) == OBJ_TYPE_BOOL)))
                {
                    int32_t a = ((pPmInt_t)TOS1)->val;
                    int32_t b = ((pPmInt_t)TOS)->val;

                    switch (t16)
                    {
                        /* *INDENT-OFF* */
                        case COMP_LT: t8 = (int8_t)(a <  b); break;
                        case COMP_LE: t8 = (int8_t)(a <= b); break;
                        case COMP_EQ: t8 = (int8_t)(a == b); break;
                        case COMP_NE: t8 = (int8_t)(a != b); break;
                        case COMP_GT: t8 = (int8_t)(a >  b); break;
                        case COMP_GE: t8 = (int8_t)(a >= b); break;
                        case COMP_IS: t8 = (int8_t)(TOS == TOS1); break;
                        case COMP_IS_NOT: t8 = (int8_t)(TOS != TOS1);break;
                        case COMP_IN:
                        case COMP_NOT_IN:
                            PM_RAISE(retval, PM_RET_EX_TYPE);
                            break;

                        default:
                            /* Other compares are not yet supported */
                            PM_RAISE(retval, PM_RET_EX_SYS);
                            break;
                        /* *INDENT-ON* */
                    }
                    PM_BREAK_IF_ERROR(retval);
                    pobj3 = (t8) ? PM_TRUE : PM_FALSE;
                }

                /* Handle non-integer comparisons */
                else
                {
                    retval = PM_RET_OK;
                    switch (t16)
                    {
                        case COMP_EQ:
                        case COMP_NE:
                            /* Handle equality for non-int types */
                            pobj3 = PM_FALSE;
                            t8 = obj_compare(TOS, TOS1);
                            if (((t8 == C_SAME) && (t16 == COMP_EQ))
                                || ((t8 == C_DIFFER) && (t16 == COMP_NE)))
                            {
                                pobj3 = PM_TRUE;
                            }
                            break;

                        case COMP_IN:
                        case COMP_NOT_IN:
                            /* Handle membership comparisons */
                            pobj3 = PM_FALSE;
                            retval = obj_isIn(TOS, TOS1);
                            if (retval == PM_RET_OK)
                            {
                                if (t16 == COMP_IN)
                                {
                                    pobj3 = PM_TRUE;
                                }
                            }
                            else if (retval == PM_RET_NO)
                            {
                                retval = PM_RET_OK;
                                if (t16 == COMP_NOT_IN)
                                {
                                    pobj3 = PM_TRUE;
                                }
                            }
                            break;

                        default:
                            /* Other comparisons are not implemented */
                            PM_RAISE(retval, PM_RET_EX_SYS);
                            break;
                    }
                    PM_BREAK_IF_ERROR(retval);
                }
                PM_SP--;
                TOS = pobj3;
                continue;

            case IMPORT_NAME:
                /* Get name index */
                t16 = GET_ARG();

                /* Get name String obj */
                pobj1 = PM_FP->fo_func->f_co->co_names->val[t16];

                /* Pop unused None object */
                PM_SP--;

                /* Ensure "level" is -1; no support for relative import yet */
                C_ASSERT(obj_compare(TOS, PM_NEGONE) == C_SAME);

                /* #110: Prevent importing previously-loaded module */
                /* If the named module is in globals, put it on the stack */
                retval =
                    dict_getItem((pPmObj_t)PM_FP->fo_globals, pobj1, &pobj2);
                if ((retval == PM_RET_OK)
                    && (OBJ_GET_TYPE(pobj2) == OBJ_TYPE_MOD))
                {
                    TOS = pobj2;
                    continue;
                }

                /* Load module from image */
                retval = mod_import(pobj1, &pobj2);
                PM_BREAK_IF_ERROR(retval);

                /* Put Module on top of stack */
                TOS = pobj2;

                /* Code after here is a duplicate of CALL_FUNCTION */
                /* Make frame object to interpret the module's root code */
                heap_gcPushTempRoot(pobj2, &objid);
                retval = frame_new(pobj2, &pobj3);
                heap_gcPopTempRoot(objid);
                PM_BREAK_IF_ERROR(retval);

                /* No arguments to pass */

                /* Keep ref to current frame */
                ((pPmFrame_t)pobj3)->fo_back = PM_FP;

                /* Handle to have None popped on return */
                ((pPmFrame_t)pobj3)->fo_isImport = (uint8_t)1;

                /* Set new frame */
                PM_FP = (pPmFrame_t)pobj3;
                continue;

#ifdef HAVE_IMPORTS
            case IMPORT_FROM:
                /* #102: Implement the remaining IMPORT_ bytecodes */
                /* Expect the module on the top of the stack */
                C_ASSERT(OBJ_GET_TYPE(TOS) == OBJ_TYPE_MOD);
                pobj1 = TOS;

                /* Get the name of the object to import */
                t16 = GET_ARG();
                pobj2 = PM_FP->fo_func->f_co->co_names->val[t16];

                /* Get the object from the module's attributes */
                retval = dict_getItem((pPmObj_t)((pPmFunc_t)pobj1)->f_attrs,
                                      pobj2, &pobj3);
                PM_BREAK_IF_ERROR(retval);

                /* Push the object onto the top of the stack */
                PM_PUSH(pobj3);
                continue;
#endif /* HAVE_IMPORTS */

            case JUMP_FORWARD:
                t16 = GET_ARG();
                PM_IP += t16;
                continue;

            case JUMP_IF_FALSE:
                t16 = GET_ARG();
                if (obj_isFalse(TOS))
                {
                    PM_IP += t16;
                }
                continue;

            case JUMP_IF_TRUE:
                t16 = GET_ARG();
                if (!obj_isFalse(TOS))
                {
                    PM_IP += t16;
                }
                continue;

            case JUMP_ABSOLUTE:
            case CONTINUE_LOOP:
                /* Get target offset (bytes) */
                t16 = GET_ARG();

                /* Jump to base_ip + arg */
                PM_IP = PM_FP->fo_func->f_co->co_codeaddr + t16;
                continue;

            case LOAD_GLOBAL:
                /* Get name */
                t16 = GET_ARG();
                pobj1 = PM_FP->fo_func->f_co->co_names->val[t16];

                /* Try globals first */
                retval = dict_getItem((pPmObj_t)PM_FP->fo_globals,
                                      pobj1, &pobj2);

                /* If that didn't work, try builtins */
                if (retval == PM_RET_EX_KEY)
                {
                    retval = dict_getItem(PM_PBUILTINS, pobj1, &pobj2);

                    /* No such global, raise NameError */
                    if (retval == PM_RET_EX_KEY)
                    {
                        PM_RAISE(retval, PM_RET_EX_NAME);
                        break;
                    }
                }
                PM_BREAK_IF_ERROR(retval);
                PM_PUSH(pobj2);
                continue;

            case SETUP_LOOP:
            {
                uint8_t *pchunk;

                /* Get block span (bytes) */
                t16 = GET_ARG();

                /* Create block */
                retval = heap_getChunk(sizeof(PmBlock_t), &pchunk);
                PM_BREAK_IF_ERROR(retval);
                pobj1 = (pPmObj_t)pchunk;
                OBJ_SET_TYPE(pobj1, OBJ_TYPE_BLK);

                /* Store current stack pointer */
                ((pPmBlock_t)pobj1)->b_sp = PM_SP;

                /* Default handler is to exit block/loop */
                ((pPmBlock_t)pobj1)->b_handler = PM_IP + t16;
                ((pPmBlock_t)pobj1)->b_type = B_LOOP;

                /* Insert block into blockstack */
                ((pPmBlock_t)pobj1)->next = PM_FP->fo_blockstack;
                PM_FP->fo_blockstack = (pPmBlock_t)pobj1;
                continue;
            }

            case LOAD_FAST:
                t16 = GET_ARG();
                PM_PUSH(PM_FP->fo_locals[t16]);
                continue;

            case STORE_FAST:
                t16 = GET_ARG();
                PM_FP->fo_locals[t16] = PM_POP();
                continue;

#ifdef HAVE_DEL
            case DELETE_FAST:
                t16 = GET_ARG();
                PM_FP->fo_locals[t16] = PM_NONE;
                continue;
#endif /* HAVE_DEL */

#ifdef HAVE_ASSERT
            case RAISE_VARARGS:
                t16 = GET_ARG();

                /* Only supports taking 1 arg for now */
                if (t16 != 1)
                {
                    PM_RAISE(retval, PM_RET_EX_SYS);
                    break;
                }

                /* Load Exception class from builtins */
                retval = dict_getItem(PM_PBUILTINS, PM_EXCEPTION_STR, &pobj2);
                if (retval != PM_RET_OK)
                {
                    PM_RAISE(retval, PM_RET_EX_SYS);
                    break;
                }

                /* Raise TypeError if TOS is not an instance of Exception */
                pobj1 = TOS;
                if ((OBJ_GET_TYPE(pobj1) != OBJ_TYPE_CLO)
                    || !class_isSubclass(pobj1, pobj2))
                {
                    PM_RAISE(retval, PM_RET_EX_TYPE);
                    break;
                }

                /* Push the traceback, parameter and exception object */
                TOS = PM_NONE;
                PM_PUSH(PM_NONE);
                PM_PUSH(pobj1);

                /* Get the exception's code attr */
                retval = dict_getItem((pPmObj_t)((pPmClass_t)pobj1)->cl_attrs,
                                      PM_CODE_STR, &pobj2);
                PM_BREAK_IF_ERROR(retval);

                /* Raise exception by breaking with retval set to code */
                PM_RAISE(retval, (PmReturn_t)(((pPmInt_t)pobj2)->val & 0xFF));
                break;
#endif /* HAVE_ASSERT */

            case CALL_FUNCTION:
                /* Get num args */
                t16 = GET_ARG();

                /* Ensure no keyword args */
                if ((t16 & (uint16_t)0xFF00) != 0)
                {
                    PM_RAISE(retval, PM_RET_EX_SYS);
                    break;
                }

                /* Get the callable */
                pobj1 = STACK(t16);

                /* Useless push to get temp-roots stack level used in cleanup */
                heap_gcPushTempRoot(pobj1, &objid);

                C_DEBUG_PRINT(VERBOSITY_LOW,
                    "interpret(), CALL_FUNCTION on <obj type=%d @ %p>\n",
                    OBJ_GET_TYPE(pobj1), pobj1);

#ifdef HAVE_GENERATORS
                /* If the callable is a generator function (can't be native) */
                if ((OBJ_GET_TYPE(pobj1) == OBJ_TYPE_FXN)
                    && (OBJ_GET_TYPE(((pPmFunc_t)pobj1)->f_co) == OBJ_TYPE_COB)
                    && (((pPmFunc_t)pobj1)->f_co->co_flags & CO_GENERATOR))
                {
#ifdef HAVE_DEFAULTARGS
                    /* Num required args := argcount - num default args */
                    t8 = ((pPmFunc_t)pobj1)->f_co->co_argcount;
                    if (((pPmFunc_t)pobj1)->f_defaultargs != C_NULL)
                    {
                        t8 -= ((pPmTuple_t)((pPmFunc_t)pobj1)->f_defaultargs)->
                            length;
                    }

                    /*
                     * Raise a TypeError if num args passed
                     * is more than allowed or less than required
                     */
                    if (((t16 & ((uint8_t)0xFF))
                         > ((pPmFunc_t)pobj1)->f_co->co_argcount)
                        || ((t16 & ((uint8_t)0xFF)) < t8))
#else
                    if ((t16 & ((uint8_t)0xFF)) !=
                        ((pPmFunc_t)pobj1)->f_co->co_argcount)
#endif /* HAVE_DEFAULTARGS */
                    {
                        PM_RAISE(retval, PM_RET_EX_TYPE);
                        break;
                    }

                    /* Collect the function and arguments into a tuple */
                    retval = tuple_new(t16 + 1, &pobj2);
                    heap_gcPushTempRoot(pobj2, &objid2);
                    PM_GOTO_IF_ERROR(retval, CALL_FUNC_CLEANUP);
                    sli_memcpy((uint8_t *)&((pPmTuple_t)pobj2)->val,
                               (uint8_t *)&STACK(t16),
                               (t16 + 1) * sizeof(pPmObj_t));

                    /* Remove old args, push func/args tuple as one arg */
                    PM_SP -= t16;
                    PM_PUSH(pobj2);
                    t16 = 1;

                    /* Set pobj1 and stack to create an instance of Generator */
                    retval = dict_getItem(PM_PBUILTINS, PM_GENERATOR_STR,
                                          &pobj1);
                    C_ASSERT(retval == PM_RET_OK);
                    STACK(t16) = pobj1;
                }
#endif /* HAVE_GENERATORS */

#ifdef HAVE_CLASSES
                /* If the callable is a class, create an instance of it */
                if (OBJ_GET_TYPE(pobj1) == OBJ_TYPE_CLO)
                {
                    /* This marks that the original callable was a class */
                    bc = 0;

                    /* Replace class with new instance */
                    retval = class_instantiate(pobj1, &pobj2);
                    heap_gcPushTempRoot(pobj2, &objid2);
                    STACK(t16) = pobj2;

                    /* If __init__ does not exist */
                    pobj3 = C_NULL;
                    retval = class_getAttr(pobj1, PM_INIT_STR, &pobj3);
                    if (retval == PM_RET_EX_KEY)
                    {
                        /* Raise TypeError if there are args */
                        if (t16 > 0)
                        {
                            PM_RAISE(retval, PM_RET_EX_TYPE);
                            goto CALL_FUNC_CLEANUP;
                        }

                        /* Otherwise, continue with instance */
                        heap_gcPopTempRoot(objid);
                        continue;
                    }
                    else if (retval != PM_RET_OK)
                    {
                        PM_GOTO_IF_ERROR(retval, CALL_FUNC_CLEANUP);
                    }

                    /* Slide the arguments up 1 slot in the stack */
                    PM_SP++;
                    for (t8 = 0; t8 < t16; t8++)
                    {
                        STACK(t8) = STACK(t8 + 1);
                    }

                    /* Convert __init__ to method, insert it as the callable */
                    retval = class_method(pobj2, pobj3, &pobj1);
                    PM_GOTO_IF_ERROR(retval, CALL_FUNC_CLEANUP);
                    heap_gcPushTempRoot(pobj2, &objid2);
                    STACK(t16) = pobj1;
                    /* Fall through to call the method */
                }

                if (OBJ_GET_TYPE(pobj1) == OBJ_TYPE_MTH)
                {
                    /* Set the method's func to be the callable */
                    STACK(t16) = (pPmObj_t)((pPmMethod_t)pobj1)->m_func;

                    /* Slide the arguments up 1 slot in the stack */
                    PM_SP++;
                    for (t8 = 0; t8 < t16; t8++)
                    {
                        STACK(t8) = STACK(t8 + 1);
                    }

                    /* Insert instance as "self" arg to the method */
                    STACK(t16++) = (pPmObj_t)((pPmMethod_t)pobj1)->m_instance;

                    /* Refresh the callable */
                    pobj1 = (pPmObj_t)((pPmMethod_t)pobj1)->m_func;
                }
#endif /* HAVE_CLASSES */

#ifdef HAVE_GENERATORS
CALL_FUNC_FOR_ITER:
#endif /* HAVE_GENERATORS */
                /* Raise a TypeError if object is not callable */
                if (OBJ_GET_TYPE(pobj1) != OBJ_TYPE_FXN)
                {
                    PM_RAISE(retval, PM_RET_EX_TYPE);
                    goto CALL_FUNC_CLEANUP;
                }

                /* If it is a regular func (not native) */
                if (OBJ_GET_TYPE(((pPmFunc_t)pobj1)->f_co) == OBJ_TYPE_COB)
                {
                    /*
                     * #132 Raise TypeError if num args does not match the
                     * code object's expected argcount
                     */

#ifdef HAVE_DEFAULTARGS
                    /* Num required args := argcount - num default args */
                    t8 = ((pPmFunc_t)pobj1)->f_co->co_argcount;
                    if (((pPmFunc_t)pobj1)->f_defaultargs != C_NULL)
                    {
                        t8 -= ((pPmTuple_t)((pPmFunc_t)pobj1)->f_defaultargs)->
                            length;
                    }

                    /*
                     * Raise a TypeError if num args passed
                     * is more than allowed or less than required
                     */
                    if (((t16 & ((uint8_t)0xFF))
                         > ((pPmFunc_t)pobj1)->f_co->co_argcount)
                        || ((t16 & ((uint8_t)0xFF)) < t8))
#else
                    if ((t16 & ((uint8_t)0xFF)) !=
                        ((pPmFunc_t)pobj1)->f_co->co_argcount)
#endif /* HAVE_DEFAULTARGS */
                    {
                        PM_RAISE(retval, PM_RET_EX_TYPE);
                        break;
                    }

                    /* Make frame object to run the func object */
                    retval = frame_new(pobj1, &pobj2);
                    heap_gcPushTempRoot(pobj2, &objid2);
                    PM_GOTO_IF_ERROR(retval, CALL_FUNC_CLEANUP);

#ifdef HAVE_CLASSES
                    /*
                     * If the original callable was a class, indicate that
                     * the frame is running the initializer so that
                     * its return object is checked for None and ignored.
                     */
                    if (bc == 0)
                    {
                        ((pPmFrame_t)pobj2)->fo_isInit = C_TRUE;
                    }
#endif /* HAVE_CLASSES */

#ifdef HAVE_DEFAULTARGS
                    /* If this func has default arguments, put them in place */
                    if (((pPmFunc_t)pobj1)->f_defaultargs != C_NULL)
                    {
                        int8_t i = 0;

                        /* Copy default args into the new frame's locals */
                        for ( /* t8 set above */ ;
                             t8 < ((pPmFunc_t)pobj1)->f_co->co_argcount; t8++)
                        {
                            ((pPmFrame_t)pobj2)->fo_locals[t8] =
                                ((pPmTuple_t)((pPmFunc_t)pobj1)->
                                 f_defaultargs)->val[i++];
                        }
                    }
#endif /* HAVE_DEFAULTARGS */

                    /* Pass args to new frame */
                    while (--t16 >= 0)
                    {
                        /*
                         * Pop args from stack right to left,
                         * since args are pushed left to right,
                         */
                        ((pPmFrame_t)pobj2)->fo_locals[t16] = PM_POP();
                    }

#ifdef HAVE_CLOSURES
                    /* #256: Add support for closures */
                    /* Copy arguments that become cellvars */
                    if (((pPmFunc_t)pobj1)->f_co->co_cellvars != C_NULL)
                    {
                        for (t8 = 0;
                             t8 < ((pPmFunc_t)pobj1)->f_co->co_cellvars->length;
                             t8++)
                        {
                            if (((pPmInt_t)((pPmFunc_t)pobj1)->
                                f_co->co_cellvars->val[t8])->val >= 0)
                            {
                                ((pPmFrame_t)pobj2)->fo_locals[
                                    ((pPmFunc_t)pobj1)->f_co->co_nlocals + t8] =
                                    ((pPmFrame_t)pobj2)->fo_locals[
                                        ((pPmInt_t)(((pPmFunc_t)pobj1)->
                                            f_co->co_cellvars->val[t8]))->val
                                    ];
                            }
                        }
                    }

                    /* Fill frame's freevars with references from closure */
                    for (t8 = 0;
                         t8 < ((pPmFunc_t)pobj1)->f_co->co_nfreevars;
                         t8++)
                    {
                        C_ASSERT(((pPmFunc_t)pobj1)->f_closure != C_NULL);
                        ((pPmFrame_t)pobj2)->fo_locals[
                            ((pPmFunc_t)pobj1)->f_co->co_nlocals
                            + ((((pPmFunc_t)pobj1)->f_co->co_cellvars == C_NULL) ? 0 : ((pPmFunc_t)pobj1)->f_co->co_cellvars->length)
                            + t8] = ((pPmFunc_t)pobj1)->f_closure->val[t8];
                    }
#endif /* HAVE_CLOSURES */

                    /* Pop func obj */
                    pobj3 = PM_POP();

                    /* Keep ref to current frame */
                    ((pPmFrame_t)pobj2)->fo_back = PM_FP;

                    /* Set new frame */
                    PM_FP = (pPmFrame_t)pobj2;
                }

                /* If it's native func */
                else if (OBJ_GET_TYPE(((pPmFunc_t)pobj1)->f_co) ==
                         OBJ_TYPE_NOB)
                {
                    /* Set number of locals (arguments) */
                    gVmGlobal.nativeframe.nf_numlocals = (uint8_t)t16;

                    /* Pop args from stack */
                    while (--t16 >= 0)
                    {
                        gVmGlobal.nativeframe.nf_locals[t16] = PM_POP();
                    }

#ifdef HAVE_GC
                    /* If the heap is low on memory, run the GC */
                    if (heap_getAvail() < HEAP_GC_NF_THRESHOLD)
                    {
                        retval = heap_gcRun();
                        PM_GOTO_IF_ERROR(retval, CALL_FUNC_CLEANUP);
                    }
#endif /* HAVE_GC */

                    /* Pop the function object */
                    PM_SP--;

                    /* Get native function index */
                    pobj2 = (pPmObj_t)((pPmFunc_t)pobj1)->f_co;
                    t16 = ((pPmNo_t)pobj2)->no_funcindx;

                    /* Set flag, so the GC knows a native session is active */
                    gVmGlobal.nativeframe.nf_active = C_TRUE;

                    /*
                     * CALL NATIVE FXN: pass caller's frame and numargs
                     */
                    /* Positive index is a stdlib func */
                    if (t16 >= 0)
                    {
                        retval = std_nat_fxn_table[t16] (&PM_FP);
                    }

                    /* Negative index is a usrlib func */
                    else
                    {
                        retval = usr_nat_fxn_table[-t16] (&PM_FP);
                    }

                    /*
                     * RETURN FROM NATIVE FXN
                     */

                    /* Clear flag, so frame will not be marked by the GC */
                    gVmGlobal.nativeframe.nf_active = C_FALSE;

#ifdef HAVE_CLASSES
                    /* If class's __init__ called, do not push a return obj */
                    if (bc == 0)
                    {
                        /* Raise TypeError if returned obj was not None */
                        if ((retval == PM_RET_OK)
                            && (gVmGlobal.nativeframe.nf_stack != PM_NONE))
                        {
                            PM_RAISE(retval, PM_RET_EX_TYPE);
                            goto CALL_FUNC_CLEANUP;
                        }
                    }
                    else
#endif /* HAVE_CLASSES */

                    /* If the frame pointer was switched, do nothing to TOS */
                    if (retval == PM_RET_FRAME_SWITCH)
                    {
                        retval = PM_RET_OK;
                    }

                    /* Otherwise, return the result from the native function */
                    else
                    {
                        PM_PUSH(gVmGlobal.nativeframe.nf_stack);
                    }
                }
CALL_FUNC_CLEANUP:
                heap_gcPopTempRoot(objid);
                PM_BREAK_IF_ERROR(retval);
                continue;

            case MAKE_FUNCTION:
                /* Get num default args to fxn */
                t16 = GET_ARG();

                /*
                 * The current frame's globals become the function object's
                 * globals.  The current frame is the container object
                 * of this new function object
                 */
                retval = func_new(TOS, (pPmObj_t)PM_FP->fo_globals, &pobj2);
                PM_BREAK_IF_ERROR(retval);

                /* Put any default args in a tuple */
                if (t16 > 0)
                {

#ifdef HAVE_DEFAULTARGS
                    heap_gcPushTempRoot(pobj2, &objid);
                    retval = tuple_new(t16, &pobj3);
                    heap_gcPopTempRoot(objid);
                    PM_BREAK_IF_ERROR(retval);
                    PM_SP--;
                    while (--t16 >= 0)
                    {
                        ((pPmTuple_t)pobj3)->val[t16] = PM_POP();
                    }

                    /* Set func's default args */
                    ((pPmFunc_t)pobj2)->f_defaultargs = (pPmTuple_t)pobj3;
#else
                    /* Default arguments not configured in pmfeatures.h */
                    PM_RAISE(retval, PM_RET_EX_SYS);
                    break;
#endif /* HAVE_DEFAULTARGS */

                }
                else
                {
                    PM_SP--;
                }

                /* Push func obj */
                PM_PUSH(pobj2);
                continue;

#ifdef HAVE_CLOSURES
            case MAKE_CLOSURE:
                /* Get number of default args */
                t16 = GET_ARG();
                retval = func_new(TOS, (pPmObj_t)PM_FP->fo_globals, &pobj2);
                PM_BREAK_IF_ERROR(retval);

                /* Set closure of the new function */
                ((pPmFunc_t)pobj2)->f_closure = (pPmTuple_t)TOS1;
                PM_SP -= 2;

                /* Collect any default arguments into tuple */
                if (t16 > 0)
                {
                    heap_gcPushTempRoot(pobj2, &objid);
                    retval = tuple_new(t16, &pobj3);
                    heap_gcPopTempRoot(objid);
                    PM_BREAK_IF_ERROR(retval);

                    while (--t16 >= 0)
                    {
                        ((pPmTuple_t)pobj3)->val[t16] = PM_POP();
                    }
                    ((pPmFunc_t)pobj2)->f_defaultargs = (pPmTuple_t)pobj3;
                }

                /* Push new func with closure */
                PM_PUSH(pobj2);
                continue;

            case LOAD_CLOSURE:
            case LOAD_DEREF:
                /* Loads the i'th cell of free variable storage onto TOS */
                t16 = GET_ARG();
                pobj1 = PM_FP->fo_locals[PM_FP->fo_func->f_co->co_nlocals + t16];
                if (pobj1 == C_NULL)
                {
                    PM_RAISE(retval, PM_RET_EX_SYS);
                    break;
                }
                PM_PUSH(pobj1);
                continue;

            case STORE_DEREF:
                /* Stores TOS into the i'th cell of free variable storage */
                t16 = GET_ARG();
                PM_FP->fo_locals[PM_FP->fo_func->f_co->co_nlocals + t16] = PM_POP();
                continue;
#endif /* HAVE_CLOSURES */


            default:
                /* SystemError, unknown or unimplemented opcode */
                PM_RAISE(retval, PM_RET_EX_SYS);
                break;
        }

#ifdef HAVE_GENERATORS
        /* If got a StopIteration exception, check for a B_LOOP block */
        if (retval == PM_RET_EX_STOP)
        {
            pobj1 = (pPmObj_t)PM_FP;
            while ((retval == PM_RET_EX_STOP) && (pobj1 != C_NULL))
            {
                pobj2 = (pPmObj_t)((pPmFrame_t)pobj1)->fo_blockstack;
                while ((retval == PM_RET_EX_STOP) && (pobj2 != C_NULL))
                {
                    if (((pPmBlock_t)pobj2)->b_type == B_LOOP)
                    {
                        /* Resume execution where the block handler says */
                        /* Set PM_FP first, so PM_SP and PM_IP are set in the frame */
                        PM_FP = (pPmFrame_t)pobj1;
                        PM_SP = ((pPmBlock_t)pobj2)->b_sp;
                        PM_IP = ((pPmBlock_t)pobj2)->b_handler;
                        ((pPmFrame_t)pobj1)->fo_blockstack =
                            ((pPmFrame_t)pobj1)->fo_blockstack->next;
                        retval = PM_RET_OK;
                        break;
                    }

                    pobj2 = (pPmObj_t)((pPmBlock_t)pobj2)->next;
                }
                pobj1 = (pPmObj_t)((pPmFrame_t)pobj1)->fo_back;
            }
            if (retval == PM_RET_OK)
            {
                continue;
            }
        }
#endif /* HAVE_GENERATORS */

        /*
         * If execution reaches this point, it is because
         * a return value (from above) is not OK or we should exit the thread
         * (return of the function). In any case, remove the
         * current thread and reschedule.
         */
        PM_REPORT_IF_ERROR(retval);

        /* If this is the last thread, return the error code */
        if ((gVmGlobal.threadList->length <= 1) && (retval != PM_RET_OK))
        {
            break;
        }

        retval = list_remove((pPmObj_t)gVmGlobal.threadList,
                             (pPmObj_t)gVmGlobal.pthread);
        gVmGlobal.pthread = C_NULL;
        PM_BREAK_IF_ERROR(retval);

        retval = interp_reschedule();
        PM_BREAK_IF_ERROR(retval);
    }

    return retval;
}


PmReturn_t
interp_reschedule(void)
{
    PmReturn_t retval = PM_RET_OK;
    static uint8_t threadIndex = (uint8_t)0;
    pPmObj_t pobj;

    /* If there are no threads in the runnable list, null the active thread */
    if (gVmGlobal.threadList->length == 0)
    {
        gVmGlobal.pthread = C_NULL;
    }

    /* Otherwise, get the next thread in the list (round robin) */
    else
    {
        if (++threadIndex >= gVmGlobal.threadList->length)
        {
            threadIndex = (uint8_t)0;
        }
        retval = list_getItem((pPmObj_t)gVmGlobal.threadList, threadIndex,
                              &pobj);
        gVmGlobal.pthread = (pPmThread_t)pobj;
        PM_RETURN_IF_ERROR(retval);
    }

    /* Clear flag to indicate a reschedule has occurred */
    interp_setRescheduleFlag(0);
    return retval;
}


PmReturn_t
interp_addThread(pPmFunc_t pfunc)
{
    PmReturn_t retval;
    pPmObj_t pframe;
    pPmObj_t pthread;
    uint8_t objid1, objid2;

    /* Create a frame for the func */
    retval = frame_new((pPmObj_t)pfunc, &pframe);
    PM_RETURN_IF_ERROR(retval);

    /* Create a thread with this new frame */
    heap_gcPushTempRoot(pframe, &objid1);
    retval = thread_new(pframe, &pthread);
    if (retval != PM_RET_OK)
    {
        heap_gcPopTempRoot(objid1);
        return retval;
    }

    /* Add thread to end of list */
    heap_gcPushTempRoot(pthread, &objid2);
    retval = list_append((pPmObj_t)gVmGlobal.threadList, pthread);
    heap_gcPopTempRoot(objid1);
    return retval;
}


void
interp_setRescheduleFlag(uint8_t boolean)
{
    gVmGlobal.reschedule = boolean;
}
