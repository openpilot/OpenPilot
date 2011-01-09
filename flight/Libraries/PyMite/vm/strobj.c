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
#define __FILE_ID__ 0x12


/**
 * \file
 * \brief String Object Type
 *
 * String object type opeartions.
 */

#include "pm.h"


#if USE_STRING_CACHE
/** String obj cachche: a list of all string objects. */
static pPmString_t pstrcache = C_NULL;
#endif /* USE_STRING_CACHE */


/*
 * If USE_STRING_CACHE is defined nonzero, the string cache
 * will be searched for an existing String object.
 * If not found, a new object is created and inserted
 * into the cache.
 */
PmReturn_t
string_create(PmMemSpace_t memspace, uint8_t const **paddr, int16_t len,
              int16_t n, pPmObj_t *r_pstring)
{
    PmReturn_t retval = PM_RET_OK;
    pPmString_t pstr = C_NULL;
    uint8_t *pdst = C_NULL;
    uint8_t const *psrc = C_NULL;

#if USE_STRING_CACHE
    pPmString_t pcacheentry = C_NULL;
#endif /* USE_STRING_CACHE */
    uint8_t *pchunk;

    /* If loading from an image, get length from the image */
    if (len < 0)
    {
        len = mem_getWord(memspace, paddr);
    }

    /* If loading from a C string, get its strlen (first null) */
    else if (len == 0)
    {
        len = sli_strlen((char const *)*paddr);
    }

    /* Get space for String obj */
    retval = heap_getChunk(sizeof(PmString_t) + len * n, &pchunk);
    PM_RETURN_IF_ERROR(retval);
    pstr = (pPmString_t)pchunk;

    /* Fill the string obj */
    OBJ_SET_TYPE(pstr, OBJ_TYPE_STR);
    pstr->length = len * n;

    /* Copy C-string into String obj */
    pdst = (uint8_t *)&(pstr->val);
    while (--n >= 0)
    {
        psrc = *paddr;
        mem_copy(memspace, &pdst, &psrc, len);
    }

    /* Be sure paddr points to one byte past the end of the source string */
    *paddr = psrc;

    /* Zero-pad end of string */
    for (; pdst < (uint8_t *)pstr + OBJ_GET_SIZE(pstr); pdst++)
    {
        *pdst = 0;
    }

#if USE_STRING_CACHE
    /* Check for twin string in cache */
    for (pcacheentry = pstrcache;
         pcacheentry != C_NULL; pcacheentry = pcacheentry->next)
    {
        /* If string already exists */
        if (string_compare(pcacheentry, pstr) == C_SAME)
        {
            /* Free the string */
            retval = heap_freeChunk((pPmObj_t)pstr);

            /* Return ptr to old */
            *r_pstring = (pPmObj_t)pcacheentry;
            return retval;
        }
    }

    /* Insert string obj into cache */
    pstr->next = pstrcache;
    pstrcache = pstr;

#endif /* USE_STRING_CACHE */

    *r_pstring = (pPmObj_t)pstr;
    return PM_RET_OK;
}


PmReturn_t
string_newFromChar(uint8_t const c, pPmObj_t *r_pstring)
{
    PmReturn_t retval;
    uint8_t cstr[2];
    uint8_t const *pcstr;

    cstr[0] = c;
    cstr[1] = '\0';
    pcstr = cstr;

    retval = string_new(&pcstr, r_pstring);

    /* If c was a null character, force the length to 1 */
    if (c == '\0')
    {
        ((pPmString_t)*r_pstring)->length = 1;
    }

    return retval;
}


int8_t
string_compare(pPmString_t pstr1, pPmString_t pstr2)
{
    /* Return false if lengths are not equal */
    if (pstr1->length != pstr2->length)
    {
        return C_DIFFER;
    }

    /* Compare the strings' contents */
    return sli_strncmp((char const *)&(pstr1->val),
                       (char const *)&(pstr2->val),
                       pstr1->length) == 0 ? C_SAME : C_DIFFER;
}


#ifdef HAVE_PRINT
PmReturn_t
string_printFormattedBytes(uint8_t *pb, uint8_t is_escaped, uint16_t n)
{
    uint16_t i;
    uint8_t ch;
    uint8_t nibble;
    PmReturn_t retval = PM_RET_OK;

    if (is_escaped)
    {
        retval = plat_putByte('\'');
        PM_RETURN_IF_ERROR(retval);
    }

    for (i = 0; i < n; i++)
    {
        ch = pb[i];
        if (is_escaped && (ch == '\\'))
        {
            /* Output an additional backslash to escape it. */
            retval = plat_putByte('\\');
            PM_RETURN_IF_ERROR(retval);
        }

        /* Print the hex escape code of non-printable characters */
        if (is_escaped
            && ((ch < (uint8_t)32) || (ch >= (uint8_t)128) || (ch == '\'')))
        {
            plat_putByte('\\');
            plat_putByte('x');

            nibble = (ch >> (uint8_t)4) + '0';
            if (nibble > '9')
                nibble += ('a' - '0' - (uint8_t)10);
            plat_putByte(nibble);

            nibble = (ch & (uint8_t)0x0F) + '0';
            if (nibble > '9')
                nibble += ('a' - '0' - (uint8_t)10);
            plat_putByte(nibble);
        }
        else
        {
            /* Simply output character */
            retval = plat_putByte(ch);
            PM_RETURN_IF_ERROR(retval);
        }
    }
    if (is_escaped)
    {
        retval = plat_putByte('\'');
    }

    return retval;
}


PmReturn_t
string_print(pPmObj_t pstr, uint8_t is_escaped)
{
    PmReturn_t retval = PM_RET_OK;

    C_ASSERT(pstr != C_NULL);

    /* Ensure string obj */
    if (OBJ_GET_TYPE(pstr) != OBJ_TYPE_STR)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    retval = string_printFormattedBytes(&(((pPmString_t)pstr)->val[0]),
                                        is_escaped,
                                        ((pPmString_t)pstr)->length);

    return retval;
}
#endif /* HAVE_PRINT */


PmReturn_t
string_cacheInit(void)
{
#if USE_STRING_CACHE
    pstrcache = C_NULL;
#endif
    return PM_RET_OK;
}


PmReturn_t
string_getCache(pPmString_t **r_ppstrcache)
{
#if USE_STRING_CACHE
    *r_ppstrcache = &pstrcache;
#else
    *r_ppstrcache = C_NULL;
#endif
    return PM_RET_OK;
}


PmReturn_t
string_concat(pPmString_t pstr1, pPmString_t pstr2, pPmObj_t *r_pstring)
{
    PmReturn_t retval = PM_RET_OK;
    pPmString_t pstr = C_NULL;
    uint8_t *pdst = C_NULL;
    uint8_t const *psrc = C_NULL;
#if USE_STRING_CACHE
    pPmString_t pcacheentry = C_NULL;
#endif /* USE_STRING_CACHE */
    uint8_t *pchunk;
    uint16_t len;

    /* Create the String obj */
    len = pstr1->length + pstr2->length;
    retval = heap_getChunk(sizeof(PmString_t) + len, &pchunk);
    PM_RETURN_IF_ERROR(retval);
    pstr = (pPmString_t)pchunk;
    OBJ_SET_TYPE(pstr, OBJ_TYPE_STR);
    pstr->length = len;

    /* Concatenate C-strings into String obj and apply null terminator */
    pdst = (uint8_t *)&(pstr->val);
    psrc = (uint8_t const *)&(pstr1->val);
    mem_copy(MEMSPACE_RAM, &pdst, &psrc, pstr1->length);
    psrc = (uint8_t const *)&(pstr2->val);
    mem_copy(MEMSPACE_RAM, &pdst, &psrc, pstr2->length);
    *pdst = '\0';

#if USE_STRING_CACHE
    /* Check for twin string in cache */
    for (pcacheentry = pstrcache;
         pcacheentry != C_NULL; pcacheentry = pcacheentry->next)
    {
        /* If string already exists */
        if (string_compare(pcacheentry, pstr) == C_SAME)
        {
            /* Free the string */
            retval = heap_freeChunk((pPmObj_t)pstr);

            /* Return ptr to old */
            *r_pstring = (pPmObj_t)pcacheentry;
            return retval;
        }
    }

    /* Insert string obj into cache */
    pstr->next = pstrcache;
    pstrcache = pstr;
#endif /* USE_STRING_CACHE */

    *r_pstring = (pPmObj_t)pstr;
    return PM_RET_OK;
}


#ifdef HAVE_STRING_FORMAT

#define SIZEOF_FMTDBUF 42
#define SIZEOF_SMALLFMT 8

PmReturn_t
string_format(pPmString_t pstr, pPmObj_t parg, pPmObj_t *r_pstring)
{
    PmReturn_t retval;
    uint16_t strsize = 0;
    uint16_t strindex;
    uint8_t *fmtcstr;
    uint8_t smallfmtcstr[SIZEOF_SMALLFMT];
    uint8_t fmtdbuf[SIZEOF_FMTDBUF];
    uint8_t i;
    uint8_t j;
    uint8_t argtupleindex = 0;
    pPmObj_t pobj;
    int snprintretval;
    uint8_t expectedargcount = 0;
    pPmString_t pnewstr;
    uint8_t *pchunk;
#if USE_STRING_CACHE
    pPmString_t pcacheentry = C_NULL;
#endif /* USE_STRING_CACHE */

    /* Get the first arg */
    pobj = parg;

    /* Calculate the size of the resulting string */
    fmtcstr = pstr->val;
    for (i = 0; i < pstr->length; i++)
    {
        /* Count non-format chars */
        if (fmtcstr[i] != '%') { strsize++; continue; }

        /* If double percents, count one percent */
        if (fmtcstr[++i] == '%') { strsize++; continue; }

        /* Get arg from the tuple */
        if (OBJ_GET_TYPE(parg) == OBJ_TYPE_TUP)
        {
            pobj = ((pPmTuple_t)parg)->val[argtupleindex++];
        }

        snprintretval = -1;

        /* Format one arg to get its length */
        smallfmtcstr[0] = '%';
        for(j = 1; (i < pstr->length) && (j < SIZEOF_SMALLFMT); i++)
        {
            smallfmtcstr[j] = fmtcstr[i];
            j++;

            if ((fmtcstr[i] == 'd')
                || (fmtcstr[i] == 'x')
                || (fmtcstr[i] == 'X'))
            {
                if (OBJ_GET_TYPE(pobj) != OBJ_TYPE_INT)
                {
                    PM_RAISE(retval, PM_RET_EX_TYPE);
                    return retval;
                }
                smallfmtcstr[j] = '\0';
                snprintretval = snprintf((char *)fmtdbuf, SIZEOF_FMTDBUF,
                    (char *)smallfmtcstr, ((pPmInt_t)pobj)->val);
                break;
            }

#ifdef HAVE_FLOAT
            else if (fmtcstr[i] == 'f')
            {
                if (OBJ_GET_TYPE(pobj) != OBJ_TYPE_FLT)
                {
                    PM_RAISE(retval, PM_RET_EX_TYPE);
                    return retval;
                }
                smallfmtcstr[j] = '\0';
                snprintretval = snprintf((char *)fmtdbuf, SIZEOF_FMTDBUF,
                    (char *)smallfmtcstr, ((pPmFloat_t)pobj)->val);
                break;
            }
#endif /* HAVE_FLOAT */

            else if (fmtcstr[i] == 's')
            {
                if (OBJ_GET_TYPE(pobj) != OBJ_TYPE_STR)
                {
                    PM_RAISE(retval, PM_RET_EX_TYPE);
                    return retval;
                }
                smallfmtcstr[j] = '\0';
                snprintretval = snprintf((char *)fmtdbuf, SIZEOF_FMTDBUF,
                    (char *)smallfmtcstr, ((pPmString_t)pobj)->val);
                break;
            }
        }

        /* Raise ValueError if the format string was bad */
        if (snprintretval < 0)
        {
            PM_RAISE(retval, PM_RET_EX_VAL);
            return retval;
        }

        expectedargcount++;
        strsize += snprintretval;
    }

    /* TypeError wrong number args */
    if (((OBJ_GET_TYPE(parg) != OBJ_TYPE_TUP) && (expectedargcount != 1))
        || ((OBJ_GET_TYPE(parg) == OBJ_TYPE_TUP)
            && (expectedargcount != ((pPmTuple_t)parg)->length)))
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* Allocate and initialize String obj */
    retval = heap_getChunk(sizeof(PmString_t) + strsize, &pchunk);
    PM_RETURN_IF_ERROR(retval);
    pnewstr = (pPmString_t)pchunk;
    OBJ_SET_TYPE(pnewstr, OBJ_TYPE_STR);
    pnewstr->length = strsize;


    /* Fill contents of String obj */
    strindex = 0;
    argtupleindex = 0;
    pobj = parg;

    for (i = 0; i < pstr->length; i++)
    {
        /* Copy non-format chars */
        if (fmtcstr[i] != '%')
        {
            pnewstr->val[strindex++] = fmtcstr[i];
            continue;
        }

        /* If double percents, copy one percent */
        if (fmtcstr[++i] == '%')
        {
            pnewstr->val[strindex++] = '%';
            continue;
        }

        /* Get arg from the tuple */
        if (OBJ_GET_TYPE(parg) == OBJ_TYPE_TUP)
        {
            pobj = ((pPmTuple_t)parg)->val[argtupleindex++];
        }

        snprintretval = -1;

        /* Format one arg to get its length */
        smallfmtcstr[0] = '%';
        for(j = 1; (i < pstr->length) && (j < SIZEOF_SMALLFMT); i++)
        {
            smallfmtcstr[j] = fmtcstr[i];
            j++;

            if ((fmtcstr[i] == 'd')
                || (fmtcstr[i] == 'x')
                || (fmtcstr[i] == 'X'))
            {
                smallfmtcstr[j] = '\0';
                snprintretval = snprintf((char *)fmtdbuf, SIZEOF_FMTDBUF,
                    (char *)smallfmtcstr, ((pPmInt_t)pobj)->val);
                break;
            }

#ifdef HAVE_FLOAT
            else if (fmtcstr[i] == 'f')
            {
                smallfmtcstr[j] = '\0';
                snprintretval = snprintf((char *)fmtdbuf, SIZEOF_FMTDBUF,
                    (char *)smallfmtcstr, ((pPmFloat_t)pobj)->val);
                break;
            }
#endif /* HAVE_FLOAT */

            else if (fmtcstr[i] == 's')
            {
                smallfmtcstr[j] = '\0';
                snprintretval = snprintf((char *)fmtdbuf, SIZEOF_FMTDBUF,
                    (char *)smallfmtcstr, ((pPmString_t)pobj)->val);
                break;
            }
        }

        /* Copy formatted C string into new string object */
        for (j = 0; j < snprintretval; j++)
        {
            pnewstr->val[strindex++] = fmtdbuf[j];
        }
    }
    pnewstr->val[strindex] = '\0';

#if USE_STRING_CACHE
    /* Check for twin string in cache */
    for (pcacheentry = pstrcache;
         pcacheentry != C_NULL; pcacheentry = pcacheentry->next)
    {
        /* If string already exists */
        if (string_compare(pcacheentry, pnewstr) == C_SAME)
        {
            /* Free the string */
            retval = heap_freeChunk((pPmObj_t)pnewstr);

            /* Return ptr to old */
            *r_pstring = (pPmObj_t)pcacheentry;
            return retval;
        }
    }

    /* Insert string obj into cache */
    pnewstr->next = pstrcache;
    pstrcache = pnewstr;

#endif /* USE_STRING_CACHE */

    *r_pstring = (pPmObj_t)pnewstr;
    return PM_RET_OK;
}
#endif /* HAVE_STRING_FORMAT */
