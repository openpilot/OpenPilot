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

## @package string
#  @brief Provides PyMite's string module.
#


"""__NATIVE__
#include <stdlib.h>
#include <string.h>
"""


digits = "0123456789"
hexdigits = "0123456789abcdefABCDEF"
letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"


#
# Returns the integer represented by the string a [in base b].
# Optional int arg, b, may be 0 or 2 through 36; otherwise it is a ValueError.
#
def atoi(a, b):
    """__NATIVE__
    pPmObj_t pa;
    pPmObj_t pb;
    char const *pc;
    char *pend;
    long i;
    int8_t base;
    pPmObj_t pi;
    PmReturn_t retval = PM_RET_OK;

    /* Raise TypeError if it's not a string or wrong number of args, */
    pa = NATIVE_GET_LOCAL(0);
    if ((OBJ_GET_TYPE(pa) != OBJ_TYPE_STR) || (NATIVE_GET_NUM_ARGS() < 1)
        || (NATIVE_GET_NUM_ARGS() > 2))
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* Get the base, if it exists; otherwise assume 10 */
    base = 10;
    if (NATIVE_GET_NUM_ARGS() == 2)
    {
        pb = NATIVE_GET_LOCAL(1);

        /* Raise a TypeError if 2nd arg is not an int */
        if (OBJ_GET_TYPE(pb) != OBJ_TYPE_INT)
        {
            PM_RAISE(retval, PM_RET_EX_TYPE);
            return retval;
        }

        base = ((pPmInt_t)pb)->val;

        /* Raise ValueError if base is out of range */
        if ((base < 0) || (base == 1) || (base > 36))
        {
            PM_RAISE(retval, PM_RET_EX_VAL);
            return retval;
        }
    }

    /* Perform conversion */
    pend = C_NULL;
    pc = (char const *)&(((pPmString_t)pa)->val);
    i = strtol(pc, &pend, base);

    /* Raise ValueError if there was a conversion error */
    if (*pend != C_NULL)
    {
        PM_RAISE(retval, PM_RET_EX_VAL);
        return retval;
    }

    /* Create an int object to hold the result of the conversion */
    retval = int_new(i, &pi);

    NATIVE_SET_TOS(pi);

    return retval;
    """
    pass


#
# Returns the number of occurrences of substring s2 in string s1.
# WARNING: Does not match Python's behavior if s1 contains a null character.
#
def count(s1, s2):
    """__NATIVE__
    pPmObj_t ps1;
    pPmObj_t ps2;
    uint8_t *pc1;
    uint8_t *pc2;
    uint8_t *pscan;
    uint8_t *pmatch;
    uint8_t pc2c0;
    uint16_t pc1len;
    uint16_t pc2len;
    uint16_t n;
    uint16_t remaining;
    uint16_t cmp;
    pPmObj_t pn;
    PmReturn_t retval = PM_RET_OK;

    /* Raise TypeError if it's not a string or wrong number of args, */
    ps1 = NATIVE_GET_LOCAL(0);
    ps2 = NATIVE_GET_LOCAL(1);
    if ((OBJ_GET_TYPE(ps1) != OBJ_TYPE_STR) || (NATIVE_GET_NUM_ARGS() != 2)
        || (OBJ_GET_TYPE(ps2) != OBJ_TYPE_STR))
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    pc1 = ((pPmString_t)ps1)->val;
    pc2 = ((pPmString_t)ps2)->val;
    pc1len = ((pPmString_t)ps1)->length;
    pc2len = ((pPmString_t)ps2)->length;
    n = 0;

    /* Handle some quick special cases (order of if-clauses is important) */
    if (pc2len == 0)
    {
        n = pc1len + 1;
    }
    else if (pc1len == 0)
    {
        n = 0;
    }

    /* Count the number of matches */
    else
    {
        n = 0;
        remaining = pc1len;
        pscan = pc1;
        pc2c0 = pc2[0];
        while (pscan <= (pc1 + (pc1len - pc2len)))
        {
            /* Find the next possible start */
            pmatch = (uint8_t *)memchr(pscan, pc2c0, remaining);
            if (pmatch == C_NULL) break;
            remaining -= (pmatch - pscan);
            pscan = pmatch;

            /* If it matches, increase the count, else try the next char */
            cmp = memcmp(pscan, pc2, pc2len);
            if (cmp == 0)
            {
                n++;
                pscan += pc2len;
                remaining -= pc2len;
            }
            else
            {
                pscan++;
                remaining--;
            }
        }
    }

    retval = int_new(n, &pn);

    NATIVE_SET_TOS(pn);

    return retval;
    """
    pass


#
# Returns the lowest index in s1 where substring s2 is found or -1 on failure.
# WARNING: Does not accept optional start,end arguments.
#
def find(s1, s2):
    """__NATIVE__
    pPmObj_t ps1;
    pPmObj_t ps2;
    uint8_t *pc1;
    uint8_t *pc2;
    uint8_t *pmatch;
    uint8_t pc1len;
    uint8_t pc2len;
    int32_t n;
    pPmObj_t pn;
    PmReturn_t retval = PM_RET_OK;

    /* Raise TypeError if it's not a string or wrong number of args, */
    ps1 = NATIVE_GET_LOCAL(0);
    ps2 = NATIVE_GET_LOCAL(1);
    if ((OBJ_GET_TYPE(ps1) != OBJ_TYPE_STR) || (NATIVE_GET_NUM_ARGS() != 2)
        || (OBJ_GET_TYPE(ps2) != OBJ_TYPE_STR))
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    pc1 = ((pPmString_t)ps1)->val;
    pc2 = ((pPmString_t)ps2)->val;
    pc1len = ((pPmString_t)ps1)->length;
    pc2len = ((pPmString_t)ps2)->length;
    n = -1;

    /* Handle a quick special case */
    if (pc2len == 0)
    {
        n = 0;
    }

    /* Try to find the index of the substring */
    else
    {
        /* Find the next possible start */
        pmatch = (uint8_t *)memchr(pc1, pc2[0], pc1len);
        if (pmatch != C_NULL)
        {
            /* If it matches, calculate the index */
            if (memcmp(pmatch, pc2, pc2len) == 0)
            {
                n = pmatch - pc1;
            }
        }
    }

    retval = int_new(n, &pn);

    NATIVE_SET_TOS(pn);

    return retval;
    """
    pass


def join(s, sep=' '):
    len_s = len(s)
    if len_s == 0:
        return ''
    rs = s[0]
    i = 1
    while i < len_s:
        rs = rs + sep + s[i]
        i += 1
    return rs


# :mode=c:
