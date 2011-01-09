# This file is Copyright 2003, 2006, 2007, 2009, 2010 Dean Hall.
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

## @package sys
#  @brief Provides PyMite's system module, sys
#
#  USAGE
#  -----
#
#  import sys
#

#### TODO
# modules = None #set ptr to dict w/native func
# platform string or device id, rand
# ver = "0.1"            # XXX compile date & platform?
# Example: sys.version = '2.4.1 (#1, Feb 26 2006, 16:26:36) \n[GCC 4.0.0 20041026 (Apple Computer, Inc. build 4061)]'


maxint = 0x7FFFFFFF     # 2147483647


def exit(val):
    """__NATIVE__
    pPmObj_t pval = C_NULL;
    PmReturn_t retval;

    /* If no arg given, assume return 0 */
    if (NATIVE_GET_NUM_ARGS() == 0)
    {
        NATIVE_SET_TOS(PM_ZERO);
    }

    /* If 1 arg given, put it on stack */
    else if (NATIVE_GET_NUM_ARGS() == 1)
    {
        pval = NATIVE_GET_LOCAL(0);
        NATIVE_SET_TOS(pval);
    }

    /* If wrong number of args, raise TypeError */
    else
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* Raise the SystemExit exception */
    PM_RAISE(retval, PM_RET_EX_EXIT);
    return retval;
    """
    pass


#
# Runs the Garbage Collector
#
def gc():
    """__NATIVE__
    PmReturn_t retval = PM_RET_OK;
#ifdef HAVE_GC
    /* If wrong number of args, raise TypeError */
    if (NATIVE_GET_NUM_ARGS() != 0)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    retval = heap_gcRun();
#endif
    NATIVE_SET_TOS(PM_NONE);

    return retval;
    """
    pass


#
# Gets a byte from the platform's default I/O
# Returns the byte in the LSB of the returned integer
#
def getb():
    """__NATIVE__
    uint8_t b;
    pPmObj_t pb;
    PmReturn_t retval;

    /* If wrong number of args, raise TypeError */
    if (NATIVE_GET_NUM_ARGS() != 0)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    retval = plat_getByte(&b);
    PM_RETURN_IF_ERROR(retval);

    retval = int_new((int32_t)b, &pb);
    NATIVE_SET_TOS(pb);
    return retval;
    """
    pass


#
# Returns a tuple containing the amout of heap available and the maximum
#
def heap():
    """__NATIVE__
    PmReturn_t retval;
    pPmObj_t pavail;
    pPmObj_t pmax;
    pPmObj_t ptup;
    uint8_t objid;

    /* If wrong number of args, raise TypeError */
    if (NATIVE_GET_NUM_ARGS() != 0)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* Allocate a tuple to store the return values */
    retval = tuple_new(2, &ptup);
    PM_RETURN_IF_ERROR(retval);

    /* Get the maximum heap size */
    heap_gcPushTempRoot(ptup, &objid);
    retval = int_new(PM_HEAP_SIZE, &pmax);
    if (retval != PM_RET_OK)
    {
        heap_gcPopTempRoot(objid);
        return retval;
    }

    /* Allocate an int to hold the amount of heap available */
    retval = int_new(heap_getAvail() - sizeof(PmInt_t), &pavail);
    heap_gcPopTempRoot(objid);
    PM_RETURN_IF_ERROR(retval);

    /* Put the two heap values in the tuple */
    ((pPmTuple_t)ptup)->val[0] = pavail;
    ((pPmTuple_t)ptup)->val[1] = pmax;

    /* Return the tuple on the stack */
    NATIVE_SET_TOS(ptup);

    return retval;
    """
    pass


#
# Sends the LSB of the integer out the platform's default I/O
#
def putb(b):
    """__NATIVE__
    uint8_t b;
    pPmObj_t pb;
    PmReturn_t retval;

    pb = NATIVE_GET_LOCAL(0);

    /* If wrong number of args, raise TypeError */
    if (NATIVE_GET_NUM_ARGS() != 1)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* If arg is not an int, raise TypeError */
    if (OBJ_GET_TYPE(pb) != OBJ_TYPE_INT)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    b = ((pPmInt_t)pb)->val & 0xFF;
    retval = plat_putByte(b);
    NATIVE_SET_TOS(PM_NONE);
    return retval;
    """
    pass


#
# Runs the given function in a thread sharing the current global namespace
#
def runInThread(f):
    """__NATIVE__
    PmReturn_t retval;
    pPmObj_t pf;

    /* If wrong number of args, raise TypeError */
    if (NATIVE_GET_NUM_ARGS() != 1)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* If arg is not a function, raise TypeError */
    pf = NATIVE_GET_LOCAL(0);
    if (OBJ_GET_TYPE(pf) != OBJ_TYPE_FXN)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    retval = interp_addThread((pPmFunc_t)pf);
    NATIVE_SET_TOS(PM_NONE);
    return retval;
    """
    pass


#
# Returns the number of milliseconds since the PyMite VM was initialized
#
def time():
    """__NATIVE__
    uint32_t t;
    pPmObj_t pt;
    PmReturn_t retval;

    /* If wrong number of args, raise TypeError */
    if (NATIVE_GET_NUM_ARGS() != 0)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* Get the system time (milliseconds since init) */
    retval = plat_getMsTicks(&t);
    PM_RETURN_IF_ERROR(retval);

    /*
     * Raise ValueError if there is an overflow
     * (plat_getMsTicks is unsigned; int is signed)
     */
    if ((int32_t)t < 0)
    {
        PM_RAISE(retval, PM_RET_EX_VAL);
        return retval;
    }

    /* Return an int object with the time value */
    retval = int_new((int32_t)t, &pt);
    NATIVE_SET_TOS(pt);
    return retval;
    """
    pass


#
# Waits in a busy loop for the given number of milliseconds
#
def wait(ms):
    t = time() + ms
    while time() < t:
        pass


# :mode=c:
