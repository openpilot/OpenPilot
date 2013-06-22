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
#  @copybrief func

## @package func
#  @brief Provides PyMite's func module.


##
# Returns the names tuple of the function/module object
#
def co_names(f):
    """__NATIVE__
    PmReturn_t retval = PM_RET_OK;
    pPmObj_t pfunc;

    /* If wrong number of args, raise TypeError */
    if (NATIVE_GET_NUM_ARGS() != 1)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    pfunc = NATIVE_GET_LOCAL(0);
    NATIVE_SET_TOS((pPmObj_t)((pPmFunc_t)pfunc)->f_co->co_names);

    return retval;
    """
    pass


##
# Returns the constants tuple of the function/module object
#
def co_consts(f):
    """__NATIVE__
    PmReturn_t retval = PM_RET_OK;
    pPmObj_t pfunc;

    /* If wrong number of args, raise TypeError */
    if (NATIVE_GET_NUM_ARGS() != 1)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    pfunc = NATIVE_GET_LOCAL(0);
    NATIVE_SET_TOS((pPmObj_t)((pPmFunc_t)pfunc)->f_co->co_consts);

    return retval;
    """
    pass


# :mode=c:
