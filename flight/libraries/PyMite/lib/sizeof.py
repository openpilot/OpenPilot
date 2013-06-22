# This file is Copyright 2009, 2010 Dean Hall.
#
# This file is part of the Python-on-a-Chip program.
# Python-on-a-Chip is free software: you can redistribute it and/or modify
# it under the terms of the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1.
# 
# Python-on-a-Chip is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# A copy of the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1
# is seen in the file COPYING in this directory.

## @file
#  @copybrief sizeof

## @package sizeof
#  @brief Provides PyMite's sizeof module.
#
# <b>USAGE</b>
#
# \code sizeof.sizeof(obj) \endcode
#
# Prints the size of the given object.  If obj is an integer from 0..31,
# the size of the object type represented by that integer will be returned.

"""__NATIVE__
#include "pm.h"
"""


def sizeof(obj):
    """__NATIVE__
    pPmObj_t pobj;
    pPmObj_t psize;
    int32_t n;
    PmReturn_t retval = PM_RET_OK;
    int32_t static size[] = {
        sizeof(PmObj_t), /* None type */
        sizeof(PmInt_t),
        sizeof(PmFloat_t),
        sizeof(PmString_t),
        sizeof(PmTuple_t),
        sizeof(PmCo_t),
        sizeof(PmFunc_t), /* Module Obj uses func struct */
        sizeof(PmClass_t),
        sizeof(PmFunc_t),
        sizeof(PmClass_t), /* Class instance */
        0, /* CIM */
        0, /* NIM */
        sizeof(PmCo_t), /* NOB */
        sizeof(PmThread_t),
        sizeof(PmClass_t), /* Exception instance */
        sizeof(PmBoolean_t),
        sizeof(PmCodeImgObj_t),
        sizeof(PmList_t),
        sizeof(PmDict_t),
        0,
        0,
        0,
        0,
        0,
        0,
        sizeof(PmFrame_t),
        sizeof(PmBlock_t),
        sizeof(Segment_t),
        sizeof(Seglist_t),
        sizeof(PmSeqIter_t),
        sizeof(PmNativeFrame_t),
    };

    /* If wrong number of args, raise TypeError */
    if (NATIVE_GET_NUM_ARGS() != 1)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    pobj = NATIVE_GET_LOCAL(0);
    if (OBJ_GET_TYPE(pobj) == OBJ_TYPE_INT)
    {
        n = ((pPmInt_t)pobj)->val;
        if ((n >= 0) && (n < 32))
        {
            /* Return the size of the type represented by the integer */
            retval = int_new(size[n], &psize);
        }
        else
        {
            /* Return the size of an integer object */
            retval = int_new(OBJ_GET_SIZE(pobj), &psize);
        }
    }
    else
    {
        /* Return the size of the given non-integer object */
        retval = int_new(OBJ_GET_SIZE(pobj), &psize);
    }

    NATIVE_SET_TOS(psize);
    return retval;
    """
    pass


def print_sizes():
    types = (
        'NON',
        'INT',
        'FLT',
        'STR',
        'TUP',
        'COB',
        'MOD',
        'CLO',
        'FXN',
        'CLI',
        'CIM',
        'NIM',
        'NOB',
        'THR',
        0,
        'BOL',
        'CIO',
        'LST',
        'DIC',
        0, 0, 0, 0, 0, 0,
        'FRM',
        'BLK',
        'SEG',
        'SGL',
        'SQI',
        'NFM',
        0,
    )
    for i in range(32):
        if types[i] != 0:
            print "sizeof(", types[i], ") = ", sizeof(i)
    
#:mode=c:
