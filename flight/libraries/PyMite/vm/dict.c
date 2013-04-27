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
#define __FILE_ID__ 0x02


/**
 * \file
 * \brief Dict Object Type
 *
 * Dict object type operations.
 */


#include "pm.h"


PmReturn_t
dict_new(pPmObj_t *r_pdict)
{
    PmReturn_t retval = PM_RET_OK;
    pPmDict_t pdict = C_NULL;
    uint8_t *pchunk;
    
    /* Allocate a dict */
    retval = heap_getChunk(sizeof(PmDict_t), &pchunk);
    PM_RETURN_IF_ERROR(retval);

    /* Init dict fields */
    pdict = (pPmDict_t)pchunk;
    OBJ_SET_TYPE(pdict, OBJ_TYPE_DIC);
    pdict->length = 0;
    pdict->d_keys = C_NULL;
    pdict->d_vals = C_NULL;

    *r_pdict = (pPmObj_t)pchunk;
    return retval;
}


PmReturn_t
dict_clear(pPmObj_t pdict)
{
    PmReturn_t retval = PM_RET_OK;

    C_ASSERT(pdict != C_NULL);

    /* Raise TypeError if arg is not a dict */
    if (OBJ_GET_TYPE(pdict) != OBJ_TYPE_DIC)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* clear length */
    ((pPmDict_t)pdict)->length = 0;

    /* Free the keys and values seglists if needed */
    if (((pPmDict_t)pdict)->d_keys != C_NULL)
    {
        PM_RETURN_IF_ERROR(seglist_clear(((pPmDict_t)pdict)->d_keys));
        PM_RETURN_IF_ERROR(heap_freeChunk((pPmObj_t)
                                          ((pPmDict_t)pdict)->d_keys));
        ((pPmDict_t)pdict)->d_keys = C_NULL;
    }
    if (((pPmDict_t)pdict)->d_vals != C_NULL)
    {
        PM_RETURN_IF_ERROR(seglist_clear(((pPmDict_t)pdict)->d_vals));
        retval = heap_freeChunk((pPmObj_t)((pPmDict_t)pdict)->d_vals);
        ((pPmDict_t)pdict)->d_vals = C_NULL;
    }
    return retval;
}


/*
 * Sets a value in the dict using the given key.
 *
 * Scans dict for the key.  If key val found, replace old
 * with new val.  If no key found, add key/val pair to dict.
 */
PmReturn_t
dict_setItem(pPmObj_t pdict, pPmObj_t pkey, pPmObj_t pval)
{
    PmReturn_t retval = PM_RET_OK;
    int16_t indx;

    C_ASSERT(pdict != C_NULL);
    C_ASSERT(pkey != C_NULL);
    C_ASSERT(pval != C_NULL);

    /* If it's not a dict, raise TypeError */
    if (OBJ_GET_TYPE(pdict) != OBJ_TYPE_DIC)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* #112: Force Dict keys to be of hashable type */
    /* If key is not hashable, raise TypeError */
    if (OBJ_GET_TYPE(pkey) > OBJ_TYPE_HASHABLE_MAX)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* #147: Change boolean keys to integers */
    if (pkey == PM_TRUE)
    {
        pkey = PM_ONE;
    }
    else if (pkey == PM_FALSE)
    {
        pkey = PM_ZERO;
    }

    /*
     * #115: If this is the first key/value pair to be added to the Dict,
     * allocate the key and value seglists that hold those items
     */
    if (((pPmDict_t)pdict)->length == 0)
    {
        retval = seglist_new(&((pPmDict_t)pdict)->d_keys);
        PM_RETURN_IF_ERROR(retval);
        retval = seglist_new(&((pPmDict_t)pdict)->d_vals);
        PM_RETURN_IF_ERROR(retval);
    }
    else
    {
        /* Check for matching key */
        indx = 0;
        retval = seglist_findEqual(((pPmDict_t)pdict)->d_keys, pkey, &indx);

        /* If found a matching key, replace val obj */
        if (retval == PM_RET_OK)
        {
            retval = seglist_setItem(((pPmDict_t)pdict)->d_vals, pval, indx);
            return retval;
        }
    }

    /* Otherwise, insert the key,val pair */
    retval = seglist_insertItem(((pPmDict_t)pdict)->d_keys, pkey, 0);
    PM_RETURN_IF_ERROR(retval);
    retval = seglist_insertItem(((pPmDict_t)pdict)->d_vals, pval, 0);
    ((pPmDict_t)pdict)->length++;

    return retval;
}


PmReturn_t
dict_getItem(pPmObj_t pdict, pPmObj_t pkey, pPmObj_t *r_pobj)
{
    PmReturn_t retval = PM_RET_OK;
    int16_t indx = 0;

/*    C_ASSERT(pdict != C_NULL);*/

    /* if it's not a dict, raise TypeError */
    if (OBJ_GET_TYPE(pdict) != OBJ_TYPE_DIC)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* if dict is empty, raise KeyError */
    if (((pPmDict_t)pdict)->length <= 0)
    {
        PM_RAISE(retval, PM_RET_EX_KEY);
        return retval;
    }

    /* #147: Change boolean keys to integers */
    if (pkey == PM_TRUE)
    {
        pkey = PM_ONE;
    }
    else if (pkey == PM_FALSE)
    {
        pkey = PM_ZERO;
    }

    /* check for matching key */
    retval = seglist_findEqual(((pPmDict_t)pdict)->d_keys, pkey, &indx);
    /* if key not found, raise KeyError */
    if (retval == PM_RET_NO)
    {
        PM_RAISE(retval, PM_RET_EX_KEY);
    }
    /* return any other error */
    PM_RETURN_IF_ERROR(retval);

    /* key was found, get obj from vals */
    retval = seglist_getItem(((pPmDict_t)pdict)->d_vals, indx, r_pobj);
    return retval;
}


#ifdef HAVE_DEL
PmReturn_t
dict_delItem(pPmObj_t pdict, pPmObj_t pkey)
{
    PmReturn_t retval = PM_RET_OK;
    int16_t indx = 0;

    C_ASSERT(pdict != C_NULL);

    /* Check for matching key */
    retval = seglist_findEqual(((pPmDict_t)pdict)->d_keys, pkey, &indx);

    /* Raise KeyError if key is not found */
    if (retval == PM_RET_NO)
    {
        PM_RAISE(retval, PM_RET_EX_KEY);
    }

    /* Return any other error */
    PM_RETURN_IF_ERROR(retval);

    /* Remove the key and value */
    retval = seglist_removeItem(((pPmDict_t)pdict)->d_keys, indx);
    PM_RETURN_IF_ERROR(retval);
    retval = seglist_removeItem(((pPmDict_t)pdict)->d_vals, indx);

    /* Reduce the item count */
    ((pPmDict_t)pdict)->length--;

    return retval;
}
#endif /* HAVE_DEL */


#ifdef HAVE_PRINT
PmReturn_t
dict_print(pPmObj_t pdict)
{
    PmReturn_t retval = PM_RET_OK;
    int16_t index;
    pSeglist_t keys,
      vals;
    pPmObj_t pobj1;

    C_ASSERT(pdict != C_NULL);

    /* if it's not a dict, raise TypeError */
    if (OBJ_GET_TYPE(pdict) != OBJ_TYPE_DIC)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    plat_putByte('{');

    keys = ((pPmDict_t)pdict)->d_keys;
    vals = ((pPmDict_t)pdict)->d_vals;

    /* if dict is empty, raise KeyError */
    for (index = 0; index < ((pPmDict_t)pdict)->length; index++)
    {
        if (index != 0)
        {
            plat_putByte(',');
            plat_putByte(' ');
        }
        retval = seglist_getItem(keys, index, &pobj1);
        PM_RETURN_IF_ERROR(retval);
        retval = obj_print(pobj1, C_FALSE, C_TRUE);
        PM_RETURN_IF_ERROR(retval);

        plat_putByte(':');
        retval = seglist_getItem(vals, index, &pobj1);
        PM_RETURN_IF_ERROR(retval);
        retval = obj_print(pobj1, C_FALSE, C_TRUE);
        PM_RETURN_IF_ERROR(retval);
    }

    return plat_putByte('}');
}
#endif /* HAVE_PRINT */

PmReturn_t
dict_update(pPmObj_t pdestdict, pPmObj_t psourcedict)
{
    PmReturn_t retval = PM_RET_OK;
    int16_t i;
    pPmObj_t pkey;
    pPmObj_t pval;

    C_ASSERT(pdestdict != C_NULL);
    C_ASSERT(psourcedict != C_NULL);

    /* If it's not a dict, raise TypeError */
    if (OBJ_GET_TYPE(pdestdict) != OBJ_TYPE_DIC)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* If it's not a dict, raise TypeError */
    if (OBJ_GET_TYPE(psourcedict) != OBJ_TYPE_DIC)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* Iterate over the add-on dict */
    for (i = 0; i < ((pPmDict_t)psourcedict)->length; i++)
    {
        /* Get the key,val from the add-on dict */
        retval = seglist_getItem(((pPmDict_t)psourcedict)->d_keys, i, &pkey);
        PM_RETURN_IF_ERROR(retval);
        retval = seglist_getItem(((pPmDict_t)psourcedict)->d_vals, i, &pval);
        PM_RETURN_IF_ERROR(retval);

        /* Set the key,val to the destination dict */
        retval = dict_setItem(pdestdict, pkey, pval);
        PM_RETURN_IF_ERROR(retval);
    }

    return retval;
}
