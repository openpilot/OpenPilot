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
#  @copybrief list

## @package list
#  @brief Provides PyMite's list module.
#
# Notes:
# - index(l, o) does not offer start and stop arguments.


def append(l, o):
    """__NATIVE__
    pPmObj_t pl;
    PmReturn_t retval = PM_RET_OK;

    /* Raise TypeError if it's not a list or wrong number of args, */
    pl = NATIVE_GET_LOCAL(0);
    if ((OBJ_GET_TYPE(pl) != OBJ_TYPE_LST) || (NATIVE_GET_NUM_ARGS() != 2))
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* Append the object to the list */
    retval = list_append(pl, NATIVE_GET_LOCAL(1));

    NATIVE_SET_TOS(PM_NONE);

    return retval;
    """
    pass


def count(l, v):
    c = 0
    for o in l:
        if o == v:
            c = c + 1
    return c


def extend(l, s):
    for i in s:
        append(l, i)


def index(l, o):
    """__NATIVE__
    pPmObj_t pl;
    pPmObj_t po;
    pPmObj_t pi;
    PmReturn_t retval = PM_RET_OK;
    uint16_t i;

    /* Raise TypeError if it's not a list or wrong number of args, */
    pl = NATIVE_GET_LOCAL(0);
    if ((OBJ_GET_TYPE(pl) != OBJ_TYPE_LST) || (NATIVE_GET_NUM_ARGS() != 2))
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* Get the index of the object in the list */
    po = NATIVE_GET_LOCAL(1);
    retval = list_index(pl, po, &i);

    if (retval == PM_RET_EX_VAL)
    {
        PM_RAISE(retval, PM_RET_EX_VAL);
        return retval;
    }

    int_new((int32_t)i, &pi);
    NATIVE_SET_TOS(pi);

    return retval;
    """
    pass


def insert(l, i, o):
    """__NATIVE__
    pPmObj_t pl;
    pPmObj_t pi;
    pPmObj_t po;
    PmReturn_t retval = PM_RET_OK;
    uint16_t i;

    /*
     * Raise TypeError if wrong number of args, first arg is not a list, or
     * second arg is not an int
     */
    pl = NATIVE_GET_LOCAL(0);
    pi = NATIVE_GET_LOCAL(1);
    po = NATIVE_GET_LOCAL(2);
    if ((NATIVE_GET_NUM_ARGS() != 3)
        || (OBJ_GET_TYPE(pl) != OBJ_TYPE_LST)
        || (OBJ_GET_TYPE(pi) != OBJ_TYPE_INT) )
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* Insert the object before the given index */
    i = (uint16_t)((pPmInt_t)pi)->val;
    retval = list_insert(pl, i, po);

    if (retval != PM_RET_OK)
    {
        PM_RAISE(retval, PM_RET_EX_SYS);
    }

    NATIVE_SET_TOS(PM_NONE);

    return retval;
    """
    pass


def pop(l, i):
    """__NATIVE__
    pPmObj_t pl;
    pPmObj_t pi;
    pPmObj_t po;
    PmReturn_t retval = PM_RET_OK;
    int16_t i;

    /*
     * Raise TypeError if first arg is not a list o second arg is not an int
     * or there are the wrong number of arguments
     */
    pl = NATIVE_GET_LOCAL(0);
    if (OBJ_GET_TYPE(pl) != OBJ_TYPE_LST)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    pi = NATIVE_GET_LOCAL(1);
    if (NATIVE_GET_NUM_ARGS() == 2)
    {
        if (OBJ_GET_TYPE(pi) != OBJ_TYPE_INT)
        {
            PM_RAISE(retval, PM_RET_EX_TYPE);
            return retval;
        }
        i = (uint16_t)((pPmInt_t)pi)->val;
    }
    else
    {
        i = -1;
    }
    if ((NATIVE_GET_NUM_ARGS() < 1) || (NATIVE_GET_NUM_ARGS() > 2))
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* Get the object at the given index */
    retval = list_getItem(pl, i, &po);
    PM_RETURN_IF_ERROR(retval);

    /* Return the object to the caller */
    NATIVE_SET_TOS(po);

    /* Remove the object from the given index */
    retval = list_delItem(pl, i);
    PM_RETURN_IF_ERROR(retval);

    return retval;
    """
    pass


def remove(l, v):
    """__NATIVE__
    pPmObj_t pl;
    pPmObj_t pv;
    PmReturn_t retval = PM_RET_OK;

    /* Raise TypeError if it's not a list or wrong number of args, */
    pl = NATIVE_GET_LOCAL(0);
    if ((OBJ_GET_TYPE(pl) != OBJ_TYPE_LST) || (NATIVE_GET_NUM_ARGS() != 2))
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* Remove the value from the list */
    pv = NATIVE_GET_LOCAL(1);
    retval = list_remove(pl, pv);
    if (retval != PM_RET_OK)
    {
        PM_RAISE(retval, retval);
    }

    NATIVE_SET_TOS(PM_NONE);

    return retval;
    """
    pass




# TODO:
# L.reverse() -- reverse *IN PLACE*
# L.sort(cmp=None, key=None, reverse=False) -- stable sort *IN PLACE*;
#    cmp(x, y) -> -1, 0, 1

# :mode=c:
