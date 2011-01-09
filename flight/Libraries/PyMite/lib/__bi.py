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

## @file
#  @copybrief __bi

## @package __bi
#  @brief Provides PyMite's builtins module, __bi.
#
#  <b>USAGE</b>
#
#  The builtins are loaded by the interpreter.
#  The user SHOULD NOT import this module directly.


def abs(n):
#    return n > 0 and n or -n
    return (n, -n)[n < 0]


def chr(n):
    """__NATIVE__
    pPmObj_t ps;
    pPmObj_t pn;
    int32_t n;
    PmReturn_t retval;

    /* If wrong number of args, raise TypeError */
    if (NATIVE_GET_NUM_ARGS() != 1)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* Raise TypeError if arg is not an int */
    pn = NATIVE_GET_LOCAL(0);
    if (OBJ_GET_TYPE(pn) != OBJ_TYPE_INT)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* Raise ValueError if arg is not int within range(256) */
    n = ((pPmInt_t)pn)->val;
    if ((n < 0) || (n > 255))
    {
        PM_RAISE(retval, PM_RET_EX_VAL);
        return retval;
    }

    /* Create char string from  integer value */
    retval = string_newFromChar((uint8_t)n, &ps);
    NATIVE_SET_TOS(ps);
    return retval;
    """
    pass


def dir(o):
    """__NATIVE__
    PmReturn_t retval = PM_RET_OK;
    pPmObj_t po;
    pPmObj_t pk;
    pPmObj_t pl;
    pSeglist_t psl;
    int16_t i;
    uint8_t objid;

    /* Use globals if no arg given */
    if (NATIVE_GET_NUM_ARGS() == 0)
    {
        /* Get the globals dict */
        po = (pPmObj_t)NATIVE_GET_PFRAME()->fo_globals;
    }

    /* Otherwise use the given arg */
    else if (NATIVE_GET_NUM_ARGS() == 1)
    {
        po = NATIVE_GET_LOCAL(0);

        /* If object is a function or module, use its attrs dict */
        if ((OBJ_GET_TYPE(po) == OBJ_TYPE_FXN)
            || (OBJ_GET_TYPE(po) == OBJ_TYPE_MOD))
        {
            po = (pPmObj_t)((pPmFunc_t)po)->f_attrs;
        }

#ifdef HAVE_CLASSES
        else if (OBJ_GET_TYPE(po) == OBJ_TYPE_CLO)
        {
            po = (pPmObj_t)((pPmClass_t)po)->cl_attrs;
        }
        else if (OBJ_GET_TYPE(po) == OBJ_TYPE_CLI)
        {
            po = (pPmObj_t)((pPmInstance_t)po)->cli_attrs;
        }
        else if (OBJ_GET_TYPE(po) == OBJ_TYPE_MTH)
        {
            po = (pPmObj_t)((pPmMethod_t)po)->m_attrs;
        }
#endif /* HAVE_CLASSES */

        else
        {
            po = C_NULL;
        }
    }

    /* Raise TypeError if wrong number of args */
    else
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    if (po == C_NULL)
    {
        pl = PM_NONE;
    }
    else
    {
        /* Create new list */
        retval = list_new(&pl);
        PM_RETURN_IF_ERROR(retval);

        /* Copy dict's keys to the list */
        psl = ((pPmDict_t)po)->d_keys;
        for (i = 0; i < ((pPmDict_t)po)->length; i++)
        {
            retval = seglist_getItem(psl, i, &pk);
            PM_RETURN_IF_ERROR(retval);
            heap_gcPushTempRoot(pl, &objid);
            retval = list_append(pl, pk);
            heap_gcPopTempRoot(objid);
            PM_RETURN_IF_ERROR(retval);
        }
    }

    NATIVE_SET_TOS(pl);
    return retval;
    """
    pass


#
# Evaluates a given code object (created by Co()).
# Optionally accepts a globals dict as the second parameter
# Optionally accepts a locals dict as the third parameter
#
def eval(co, g, l):
    """__NATIVE__
    PmReturn_t retval;
    pPmObj_t pco;
    pPmObj_t pfunc;
    pPmObj_t pnewframe;
    pPmObj_t pg = C_NULL;
    pPmObj_t pl = C_NULL;
    uint8_t objid;

    /* If wrong number of args, raise TypeError */
    if ((NATIVE_GET_NUM_ARGS() == 0) || (NATIVE_GET_NUM_ARGS() > 3))
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* Raise ValueError if first arg is not a Code Object */
    pco = NATIVE_GET_LOCAL(0);
    if (OBJ_GET_TYPE(pco) != OBJ_TYPE_COB)
    {
        PM_RAISE(retval, PM_RET_EX_VAL);
        return retval;
    }

    /* If 2nd arg exists, raise ValueError if it is not a Dict */
    if (NATIVE_GET_NUM_ARGS() >= 2)
    {
        pg = NATIVE_GET_LOCAL(1);
        if (OBJ_GET_TYPE(pg) != OBJ_TYPE_DIC)
        {
            PM_RAISE(retval, PM_RET_EX_VAL);
            return retval;
        }
    }

    /* If no args are given, use the caller's globals for the function's */
    else
    {
        pg = (pPmObj_t)NATIVE_GET_PFRAME()->fo_globals;
    }

    /* If 3rd arg exists, raise ValueError if it is not a Dict */
    if (NATIVE_GET_NUM_ARGS() >= 3)
    {
        pl = NATIVE_GET_LOCAL(2);
        if (OBJ_GET_TYPE(pl) != OBJ_TYPE_DIC)
        {
            PM_RAISE(retval, PM_RET_EX_VAL);
            return retval;
        }
    }

    /* Create func from code object */
    retval = func_new(pco, pg, &pfunc);
    PM_RETURN_IF_ERROR(retval);

    /* Create frame from module object; globals is set to null */
    heap_gcPushTempRoot(pfunc, &objid);
    retval = frame_new(pfunc, &pnewframe);
    heap_gcPopTempRoot(objid);
    PM_RETURN_IF_ERROR(retval);

    /* TODO: Reclaim pnewframe's attrs dict created in frame_new */
    /*
     * By default use calling frame's attrs as local namespace.
     * This works for ipm because the interactive mode
     * needs a locals namespace that persists across calls to eval()
     */
    ((pPmFrame_t)pnewframe)->fo_attrs = NATIVE_GET_PFRAME()->fo_attrs;

    /* If 2nd arg exists, use it as the global namespace for the new func */
    if (NATIVE_GET_NUM_ARGS() >= 2)
    {
        ((pPmFrame_t)pnewframe)->fo_globals = (pPmDict_t)pg;

        /* If only globals is given, locals defaults to it */
        ((pPmFrame_t)pnewframe)->fo_attrs = (pPmDict_t)pg;
    }

    /* If 3rd arg exists, use it as the local namespace for the new func */
    if (NATIVE_GET_NUM_ARGS() >= 3)
    {
        ((pPmFrame_t)pnewframe)->fo_attrs = (pPmDict_t)pl;
    }

    /*
     * Set the fo_back frame so flow returns to eval()'s caller when completed.
     * Set the frame pointer so the new frame is interpreted immediately
     * after this function returns.
     */
    ((pPmFrame_t)pnewframe)->fo_back = NATIVE_GET_PFRAME();
    NATIVE_GET_PFRAME() = (pPmFrame_t)pnewframe;
    retval = PM_RET_FRAME_SWITCH;

    return retval;
    """
    pass


def filter(f, s):
    return [x for x in s if f(x)]


def globals():
    """__NATIVE__
    pPmObj_t pr = C_NULL;
    PmReturn_t retval;

    /* If wrong number of args, raise TypeError */
    if (NATIVE_GET_NUM_ARGS() != 0)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* Return calling frame's globals dict  on stack*/
    pr = (pPmObj_t)NATIVE_GET_PFRAME()->fo_globals;
    NATIVE_SET_TOS(pr);

    return PM_RET_OK;
    """
    pass


def id(o):
    """__NATIVE__
    PmReturn_t retval;
    pPmObj_t pr = C_NULL;

    /* If wrong number of args, raise TypeError */
    if (NATIVE_GET_NUM_ARGS() != 1)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* Return object's address as an int on the stack */
    retval = int_new((intptr_t)NATIVE_GET_LOCAL(0), &pr);
    NATIVE_SET_TOS(pr);

    return retval;
    """
    pass


# Yields every (i)tem in the (s)equence; requires HAVE_GENERATORS
#def iter(s):
#    for i in s:
#        yield i


def len(s):
    """__NATIVE__
    PmReturn_t retval;
    pPmObj_t ps = C_NULL;
    pPmObj_t pr = C_NULL;

    /* If wrong number of args, raise TypeError */
    if (NATIVE_GET_NUM_ARGS() != 1)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* Get first arg */
    ps = NATIVE_GET_LOCAL(0);

#ifdef HAVE_BYTEARRAY
    /* If object is an instance, get the thing it contains */
    if (OBJ_GET_TYPE(ps) == OBJ_TYPE_CLI)
    {
        retval = dict_getItem((pPmObj_t)((pPmInstance_t)ps)->cli_attrs,
                              PM_NONE,
                              &pr);

        /* If None wasn't in attributes, obj is wrong type for len() */
        if (retval == PM_RET_EX_KEY) retval = PM_RET_EX_TYPE;
        PM_RETURN_IF_ERROR(retval);
        ps = pr;
    }
#endif /* HAVE_BYTEARRAY */

    /* Get the length of the arg based on its type */
    switch (OBJ_GET_TYPE(ps))
    {
        case OBJ_TYPE_STR:
            retval = int_new(((pPmString_t)ps)->length, &pr);
            break;

        case OBJ_TYPE_TUP:
            retval = int_new(((pPmTuple_t)ps)->length, &pr);
            break;

        case OBJ_TYPE_LST:
            retval = int_new(((pPmList_t)ps)->length, &pr);
            break;

        case OBJ_TYPE_DIC:
            retval = int_new(((pPmDict_t)ps)->length, &pr);
            break;

#ifdef HAVE_BYTEARRAY
        case OBJ_TYPE_BYA:
            retval = int_new(((pPmBytearray_t)ps)->length, &pr);
            break;
#endif /* HAVE_BYTEARRAY */

        default:
            /* If not a string or sequence type, raise TypeError */
            PM_RAISE(retval, PM_RET_EX_TYPE);
    }

    NATIVE_SET_TOS(pr);
    return retval;
    """
    pass


def locals():
    """__NATIVE__
    pPmObj_t pr = C_NULL;
    PmReturn_t retval;

    /* If wrong number of args, raise TypeError */
    if (NATIVE_GET_NUM_ARGS() != 0)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* Return calling frame's local attrs dict on the stack */
    pr = (pPmObj_t)NATIVE_GET_PFRAME()->fo_attrs;
    NATIVE_SET_TOS(pr);

    return PM_RET_OK;
    """
    pass


def map(f, s):
    # Allocate the array
    r = [None,] * len(s)

    # Call function f once with each argument in sequence s
    i = 0
    for a in s:
        r[i] = f(a)
        i += 1

    # Return list of results
    return r


def ord(s):
    """__NATIVE__
    pPmObj_t ps;
    pPmObj_t pn;
    int32_t n;
    PmReturn_t retval;

    /* If wrong number of args, raise TypeError */
    if (NATIVE_GET_NUM_ARGS() != 1)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    ps = NATIVE_GET_LOCAL(0);

    /* Raise TypeError if arg is not string of length 1 */
    if ((OBJ_GET_TYPE(ps) != OBJ_TYPE_STR)
        || (((pPmString_t)ps)->length != 1))

    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* Get integer value of character */
    n = ((pPmString_t)ps)->val[0];
    retval = int_new(n, &pn);
    NATIVE_SET_TOS(pn);
    return retval;
    """
    pass


def pow(x, y):
    return x ** y


def range(a, b, c):
    """__NATIVE__
    PmReturn_t retval;
    pPmObj_t pa = C_NULL;
    pPmObj_t pb = C_NULL;
    pPmObj_t pc = C_NULL;
    pPmObj_t pi = C_NULL;
    pPmObj_t pr = C_NULL;
    int16_t i = 0;
    uint8_t objid1, objid2;

    switch (NATIVE_GET_NUM_ARGS())
    {
        case 1:
            pa = PM_ZERO;
            pb = NATIVE_GET_LOCAL(0);
            pc = PM_ONE;
            break;

        case 2:
            pa = NATIVE_GET_LOCAL(0);
            pb = NATIVE_GET_LOCAL(1);
            pc = PM_ONE;
            break;

        case 3:
            pa = NATIVE_GET_LOCAL(0);
            pb = NATIVE_GET_LOCAL(1);
            pc = NATIVE_GET_LOCAL(2);

            /* If 3rd arg is 0, ValueError */
            if (((pPmInt_t)pc)->val == 0)
            {
                PM_RAISE(retval, PM_RET_EX_VAL);
                return retval;
            }
            break;

        default:
            /* If wrong number of args, raise TypeError */
            PM_RAISE(retval, PM_RET_EX_TYPE);
            return retval;
    }

    /* Allocate list */
    retval = list_new(&pr);
    PM_RETURN_IF_ERROR(retval);

    /* Iterate depending on counting direction */
    if (((pPmInt_t)pc)->val > 0)
    {
        for (i = ((pPmInt_t)pa)->val;
             i < ((pPmInt_t)pb)->val;
             i += ((pPmInt_t)pc)->val)
        {
            heap_gcPushTempRoot(pr, &objid1);
            retval = int_new(i, &pi);
            if (retval != PM_RET_OK)
            {
                heap_gcPopTempRoot(objid1);
                return retval;
            }

            heap_gcPushTempRoot(pi, &objid2);
            retval = list_append(pr, pi);
            heap_gcPopTempRoot(objid1);
            PM_RETURN_IF_ERROR(retval);
        }
    }
    else
    {
        for (i = ((pPmInt_t)pa)->val;
             i > ((pPmInt_t)pb)->val;
             i += ((pPmInt_t)pc)->val)
        {
            heap_gcPushTempRoot(pr, &objid1);
            retval = int_new(i, &pi);
            if (retval != PM_RET_OK)
            {
                heap_gcPopTempRoot(objid1);
                return retval;
            }

            heap_gcPushTempRoot(pi, &objid2);
            retval = list_append(pr, pi);
            heap_gcPopTempRoot(objid1);
            PM_RETURN_IF_ERROR(retval);
        }
    }

    /* Return list */
    NATIVE_SET_TOS(pr);
    return retval;
    """
    pass


def sum(s):
    """__NATIVE__
    pPmObj_t ps;
    pPmObj_t pn;
    pPmObj_t po;
    int32_t n;
    uint16_t len;
    uint16_t i;
    PmReturn_t retval;
#ifdef HAVE_FLOAT
    float f;
    uint8_t usefloat = C_FALSE;
#endif /* HAVE_FLOAT */

    /* If wrong number of args, raise TypeError */
    if (NATIVE_GET_NUM_ARGS() != 1)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* Get the length of the sequence */
    ps = NATIVE_GET_LOCAL(0);
    if (OBJ_GET_TYPE(ps) == OBJ_TYPE_TUP)
    {
        len = ((pPmTuple_t)ps)->length;
    }
    else if (OBJ_GET_TYPE(ps) == OBJ_TYPE_LST)
    {
        len = ((pPmList_t)ps)->length;
    }

    /* Raise TypeError if arg is not a sequence */
    else
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* Calculate the sum of the sequence */
    n = 0;
#ifdef HAVE_FLOAT
    f = 0.0;
#endif
    for (i = 0; i < len; i++)
    {
        retval = seq_getSubscript(ps, i, &po);

        if (OBJ_GET_TYPE(po) == OBJ_TYPE_INT)
        {
            /* Add value to sum */
            n += ((pPmInt_t)po)->val;
#ifdef HAVE_FLOAT
            f += (float)((pPmInt_t)po)->val;
#endif /* HAVE_FLOAT */
        }

#ifdef HAVE_FLOAT
        else if (OBJ_GET_TYPE(po) == OBJ_TYPE_FLT)
        {
            /* Add value to sum */
            f += ((pPmFloat_t)po)->val;
            usefloat = C_TRUE;
        }
#endif /* HAVE_FLOAT */

        /* Raise TypeError if item is not an integer */
        else
        {
            PM_RAISE(retval, PM_RET_EX_TYPE);
            return retval;
        }
    }

#ifdef HAVE_FLOAT
    if (usefloat)
    {
        retval = float_new(f, &pn);
    }
    else
#endif /* HAVE_FLOAT */
    {
        retval = int_new(n, &pn);
    }
    NATIVE_SET_TOS(pn);
    return retval;
    """
    pass


def type(o):
    """__NATIVE__
    PmReturn_t retval;
    pPmObj_t po = C_NULL;
    pPmObj_t pr = C_NULL;

    /* If wrong number of args, raise TypeError */
    if (NATIVE_GET_NUM_ARGS() != 1)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* Get arg */
    po = NATIVE_GET_LOCAL(0);

    /* Create int from type enum */
    retval = int_new(OBJ_GET_TYPE(po), &pr);
    NATIVE_SET_TOS(pr);
    return retval;
    """
    pass


# At this time, xrange() works in a for loop, but not in a generator expression
# Yes: for i in xrange(42): pass
# No:  [x for x in xrange(42)]
#def xrange(n):
#    i = 0
#    while i < n:
#        yield i
#        i += 1


#
# Creates a code object from the given image string
#
def Co(i):
    """__NATIVE__
    PmReturn_t retval;
    pPmObj_t pimg;
    pPmObj_t pco;

    /* If wrong number of args, raise TypeError */
    if (NATIVE_GET_NUM_ARGS() != 1)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* Raise ValueError if arg is not a string */
    pimg = NATIVE_GET_LOCAL(0);
    if (OBJ_GET_TYPE(pimg) != OBJ_TYPE_CIO)
    {
        PM_RAISE(retval, PM_RET_EX_VAL);
        return retval;
    }

    /* Create a code object from the image */
    retval = obj_loadFromImgObj(pimg, &pco);
    PM_RETURN_IF_ERROR(retval);

    /* Return the code object */
    NATIVE_SET_TOS(pco);
    return retval;
    """
    pass


# This must be declared before any classes because LOAD_NAME(__name__) is
# part of every class-declaration, so __name__ must exist.
# This is a temporary workaround until __name__ is properly handled.
__name__ = "TBD"


#
# Root object
#
class object():
    pass


#
# Exception classes
#
class Exception(object):
    pass

class AssertionError(Exception):
    pass
AssertionError.code = 0xE4


#
# Generator class - used by the vm for generator-iterators,
# generator-expressions and generator-coroutines
#
# #207: Add support for the yield keyword
#
# WARNING: unsupported methods: throw, close, __del__
#
class Generator(object):

    def __init__(self, fa):
        """__NATIVE__
        PmReturn_t retval;
        pPmObj_t pself;
        pPmObj_t pfa;
        pPmObj_t pfunc;
        pPmObj_t pframe;
        uint8_t i;
        uint8_t objid;

        /* Raise TypeError if wrong number of args */
        if (NATIVE_GET_NUM_ARGS() != 2)
        {
            PM_RAISE(retval, PM_RET_EX_TYPE);
            return retval;
        }

        /* Raise ValueError if first args are not: instance, tuple */
        pself = NATIVE_GET_LOCAL(0);
        pfa = NATIVE_GET_LOCAL(1);
        if (OBJ_GET_TYPE(pself) != OBJ_TYPE_CLI)
        {
            PM_RAISE(retval, PM_RET_EX_VAL);
            return retval;
        }
        if (OBJ_GET_TYPE(pfa) != OBJ_TYPE_TUP)
        {
            PM_RAISE(retval, PM_RET_EX_VAL);
            return retval;
        }

        /* Create a new frame for the function */
        pfunc = ((pPmTuple_t)pfa)->val[0];
        retval = frame_new(pfunc, &pframe);
        PM_RETURN_IF_ERROR(retval);

        /* Copy args into frame's locals */
        for (i = 0; i < ((pPmTuple_t)pfa)->length - 1; i++)
        {
            /* The pfa tuple is (func, [arg0, ... argN]) */
            ((pPmFrame_t)pframe)->fo_locals[i] = ((pPmTuple_t)pfa)->val[i + 1];
        }

        /* Store frame in None attr of instance */
        heap_gcPushTempRoot(pframe, &objid);
        retval = dict_setItem((pPmObj_t)((pPmInstance_t)pself)->cli_attrs,
                              PM_NONE, pframe);
        heap_gcPopTempRoot(objid);

        NATIVE_SET_TOS(PM_NONE);
        return retval;
        """
        pass


    def next(self,):
        return self.send(None)


    def send(self, arg):
        """__NATIVE__
        PmReturn_t retval;
        pPmObj_t pself;
        pPmObj_t parg;
        pPmObj_t pgenframe;

        /* Raise TypeError if wrong number of args */
        if (NATIVE_GET_NUM_ARGS() != 2)
        {
            PM_RAISE(retval, PM_RET_EX_TYPE);
            return retval;
        }

        /* Raise ValueError if first arg is not an instance */
        pself = NATIVE_GET_LOCAL(0);
        parg = NATIVE_GET_LOCAL(1);
        if (OBJ_GET_TYPE(pself) != OBJ_TYPE_CLI)
        {
            PM_RAISE(retval, PM_RET_EX_VAL);
            return retval;
        }

        /* Get the generator's frame */
        retval = dict_getItem((pPmObj_t)((pPmInstance_t)pself)->cli_attrs,
                              PM_NONE, &pgenframe);
        PM_RETURN_IF_ERROR(retval);

        /* Push argument onto generator's frame's stack */
        *(((pPmFrame_t)pgenframe)->fo_sp) = parg;
        ((pPmFrame_t)pgenframe)->fo_sp++;

        /* Set generator's frame's fo_back so yielded value goes to caller */
        ((pPmFrame_t)pgenframe)->fo_back = NATIVE_GET_PFRAME();

        /* Set active frame to run generator */
        NATIVE_GET_PFRAME() = (pPmFrame_t)pgenframe;

        return PM_RET_FRAME_SWITCH;
        """
        pass


# WARNING: Untested.  Awaiting implementation of try/except/finally
#
#    def close(self,):
#        """__NATIVE__
#        PmReturn_t retval;
#        pPmObj_t pself;
#        pPmObj_t pgenframe;
#
#        /* Raise TypeError if wrong number of args */
#        if (NATIVE_GET_NUM_ARGS() != 1)
#        {
#            PM_RAISE(retval, PM_RET_EX_TYPE);
#            return retval;
#        }
#
#        /* Raise ValueError if first arg is not an instance */
#        pself = NATIVE_GET_LOCAL(0);
#        if (OBJ_GET_TYPE(pself) != OBJ_TYPE_CLI)
#        {
#            PM_RAISE(retval, PM_RET_EX_VAL);
#            return retval;
#        }
#
#        /* Get the generator's frame */
#        retval = dict_getItem((pPmObj_t)((pPmInstance_t)pself)->cli_attrs,
#                              PM_NONE, &pgenframe);
#        PM_RETURN_IF_ERROR(retval);
#
#        /* If there was no frame, assume generator already closed, do nothing */
#        if (OBJ_GET_TYPE(pgenframe) != OBJ_TYPE_FRM)
#        {
#            return PM_RET_OK;
#        }
#
#        /* Unbind the frame from the generator instance */
#        retval = dict_setItem((pPmObj_t)((pPmInstance_t)pself)->cli_attrs,
#                              PM_NONE, PM_NONE);
#
#        /* Push argument onto generator's frame's stack */
#        *(((pPmFrame_t)pgenframe)->fo_sp) = PM_NONE;
#        ((pPmFrame_t)pgenframe)->fo_sp++;
#
#        /* Set generator's frame's fo_back so yielded value goes to caller */
#        ((pPmFrame_t)pgenframe)->fo_back = NATIVE_GET_PFRAME();
#
#        /* Set active frame to run generator */
#        NATIVE_GET_PFRAME() = (pPmFrame_t)pgenframe;
#
#        /* Raise a GeneratorExit where the generator function was paused */
#        PM_RAISE(retval, PM_RET_EX_GEN);
#        return retval;
#        """
#        pass


#
# Returns True if called within a module being run as the main; False otherwise
#
def ismain():
    """__NATIVE__

    NATIVE_SET_TOS((NATIVE_GET_PFRAME()->fo_isImport) ? PM_FALSE : PM_TRUE);

    return PM_RET_OK;
    """
    pass


#ifdef HAVE_BYTEARRAY
#class bytearray(object):
#    def __init__(self, o):
#        """__NATIVE__
#        PmReturn_t retval;
#        pPmObj_t pself;
#        pPmObj_t po;
#        pPmObj_t pba;
#        uint8_t objid;
#
#        /* If only the self arg, create zero-length bytearray */
#        if (NATIVE_GET_NUM_ARGS() == 1)
#        {
#            po = PM_ZERO;
#        }
#
#        /* If two args, get the second arg */
#        else if (NATIVE_GET_NUM_ARGS() == 2)
#        {
#            po = NATIVE_GET_LOCAL(1);
#        }
#
#        /* Raise TypeError if wrong number of args */
#        else
#        {
#            PM_RAISE(retval, PM_RET_EX_TYPE);
#            return retval;
#        }
#        pself = NATIVE_GET_LOCAL(0);
#
#        /* Create new bytearray object */
#        retval = bytearray_new(po, &pba);
#        PM_RETURN_IF_ERROR(retval);
#
#        /* Store bytearray in None attr of instance */
#        heap_gcPushTempRoot(pba, &objid);
#        retval = dict_setItem((pPmObj_t)((pPmInstance_t)pself)->cli_attrs,
#                              PM_NONE, pba);
#        heap_gcPopTempRoot(objid);
#
#        NATIVE_SET_TOS(PM_NONE);
#        return retval;
#        """
#        pass
#endif /* HAVE_BYTEARRAY */

#:mode=c:
