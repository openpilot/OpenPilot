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


#ifndef __DICT_H__
#define __DICT_H__


/**
 * \file
 * \brief Dict Object Type
 *
 * Dict object type header.
 */


/**
 * Dict
 *
 * Contains ptr to two seglists,
 * one for keys, the other for values;
 * and a length, the number of key/value pairs.
 */
typedef struct PmDict_s
{
    /** object descriptor */
    PmObjDesc_t od;
    /** number of key,value pairs in the dict */
    int16_t length;
    /** ptr to seglist containing keys */
    pSeglist_t d_keys;
    /** ptr to seglist containing values */
    pSeglist_t d_vals;
} PmDict_t,
 *pPmDict_t;


/**
 * Clears the contents of a dict.
 * after this operation, the dict should in the same state
 * as if it were just created using dict_new().
 *
 * @param   pdict ptr to dict to clear.
 * @return  nothing
 */
PmReturn_t dict_clear(pPmObj_t pdict);

/**
 * Gets the value in the dict using the given key.
 *
 * @param   pdict ptr to dict to search
 * @param   pkey ptr to key obj
 * @param   r_pobj Return; addr of ptr to obj
 * @return  Return status
 */
PmReturn_t dict_getItem(pPmObj_t pdict, pPmObj_t pkey, pPmObj_t *r_pobj);

#ifdef HAVE_DEL
/**
 * Removes a key and value from the dict.
 * Throws TypeError if pdict is not a dict.
 * Throws KeyError if pkey does not exist in pdict.
 *
 * @param   pdict Ptr to dict to search
 * @param   pkey Ptr to key obj
 * @return  Return status
 */
PmReturn_t dict_delItem(pPmObj_t pdict, pPmObj_t pkey);
#endif /* HAVE_DEL */

/**
 * Allocates space for a new Dict.
 * Return a pointer to the dict by reference.
 *
 * @param   r_pdict Return; Addr of ptr to dict
 * @return  Return status
 */
PmReturn_t dict_new(pPmObj_t *r_pdict);

/**
 * Sets a value in the dict using the given key.
 *
 * If the dict already contains a matching key, the value is
 * replaced; otherwise the new key,val pair is inserted
 * at the front of the dict (for fast lookup).
 * In the later case, the length of the dict is incremented.
 *
 * @param   pdict ptr to dict in which (key,val) will go
 * @param   pkey ptr to key obj
 * @param   pval ptr to val obj
 * @return  Return status
 */
PmReturn_t dict_setItem(pPmObj_t pdict, pPmObj_t pkey, pPmObj_t pval);

#ifdef HAVE_PRINT
/**
 * Prints out a dict. Uses obj_print() to print elements.
 *
 * @param pobj Object to print.
 * @return Return status
 */
PmReturn_t dict_print(pPmObj_t pdict);
#endif /* HAVE_PRINT */

/**
 * Updates the destination dict with the key,value pairs from the source dict
 *
 * @param   pdestdict ptr to destination dict in which key,val pairs will go
 * @param   psourcedict ptr to source dict which has all key,val pairs to copy
 * @return  Return status
 */
PmReturn_t dict_update(pPmObj_t pdestdict, pPmObj_t psourcedict);

#endif /* __DICT_H__ */
