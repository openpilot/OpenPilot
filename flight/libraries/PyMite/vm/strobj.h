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


#ifndef __STRING_H__
#define __STRING_H__


/**
 * \file
 * \brief String Object Type
 *
 * String object type header.
 */


/** Set to nonzero to enable string cache.  DO NOT REMOVE THE DEFINITION. */
#define USE_STRING_CACHE 1


/**
 * Loads a string from image
 *
 * @param ms memoryspace paddr points to
 * @param paddr address in memoryspace of source string
 * @param r_pstring Return by reference; a new string object
 * @return Return status
 */
#define string_loadFromImg(ms, paddr, r_pstring) \
    string_create((ms), (paddr), (int16_t)-1, (int16_t)1, (r_pstring))

/**
 * Creates String object from character array in RAM
 *
 * @param paddr pointer to address of source string
 * @param r_pstring Return arg; addr of ptr to string
 */
#define string_new(paddr, r_pstring) \
    string_create(MEMSPACE_RAM, (uint8_t const **)(paddr), 0, (int16_t)1, (r_pstring))

/**
 * Creates String object from character array in RAM which may contain
 * embedded null characters.
 *
 * @param paddr pointer to address of source string
 * @param len length of source string
 * @param r_pstring Return arg; addr of ptr to string
 */
#define string_newWithLen(paddr, len, r_pstring) \
    string_create(MEMSPACE_RAM, (uint8_t const **)(paddr), (len), (int16_t)1, \
                  (r_pstring))

/**
 * Creates String object by replicating an existing C string, n times
 *
 * @param paddr pointer to address of source string
 * @param n number of times to replicate the source string
 * @param r_pstring Return arg; addr of ptr to string
 */
#define string_replicate(paddr, n, r_pstring) \
    string_create(MEMSPACE_RAM, (paddr), (uint8_t)0, (n), (r_pstring))

/***************************************************************
 * Types
 **************************************************************/

/**
 * String obj
 *
 * Null terminated array of chars.
 */
typedef struct PmString_s
{
    /** Object descriptor */
    PmObjDesc_t od;

    /** Length of string */
    uint16_t length;

#if USE_STRING_CACHE
    /** Ptr to next string in cache */
    struct PmString_s *next;
#endif                          /* USE_STRING_CACHE */

    /**
     * Null-term char array
     *
     * Use length 1 here so that string-alloc function can use
     * "sizeof(PmString_t) + len" and there will be room for the null-term
     */
    uint8_t val[1];
} PmString_t,
 *pPmString_t;


/***************************************************************
 * Prototypes
 **************************************************************/

/**
 * Creates a new String obj.
 * If len is less than zero, load from a String image.
 * If len is zero, copy from a C string (which has a null terminator)
 * If len is positive, copy as many chars as given in the len argument
 *      A string image has the following structure:
 *          -type:      int8 - OBJ_TYPE_STRING
 *          -length:    uint16 - number of bytes in the string
 *          -val:       uint8[] - array of chars with null term
 *
 * Returns by reference a ptr to String obj.
 *
 * Obtain space for String from the heap.
 * Copy string from memspace.
 * Leave contents of paddr pointing one byte past end of str.
 *
 * THE PROGRAMMER SHOULD NOT CALL THIS FUNCTION DIRECTLY.
 * Instead, use one of the two macros string_loadFromImg()
 * or string_new().
 *
 * @param   memspace memory space where *paddr points
 * @param   paddr ptr to ptr to null term character array or image.
 * @param   len length of the C character array
 *          (use -1 for string images, 0 for C strings)
 * @param   n Number of times to replicate the given string argument
 * @param   r_pstring Return by reference; ptr to String obj
 * @return  Return status
 */
PmReturn_t string_create(PmMemSpace_t memspace, uint8_t const **paddr,
                         int16_t len, int16_t n, pPmObj_t *r_pstring);

/**
 * Creates a new String object from a single character.
 *
 * @param   c The character to become the string
 * @param   r_pstring Return by reference; ptr to String obj
 * @return  Return status
 */
PmReturn_t string_newFromChar(uint8_t const c, pPmObj_t *r_pstring);

/**
 * Compares two String objects for equality.
 *
 * @param   pstr1 Ptr to first string
 * @param   pstr2 Ptr to second string
 * @return  C_SAME if the strings are equivalent, C_DIFFER otherwise
 */
int8_t string_compare(pPmString_t pstr1, pPmString_t pstr2);

#ifdef HAVE_PRINT
/**
 * Sends out a string object bytewise. Escaping and framing is configurable
 * via is_escaped.
 *
 * @param pstr Ptr to string object
 * @param is_escaped If 0, print out string as is. Otherwise escape unprintable
 *                   characters and surround string with single quotes.
 * @return Return status
 */
PmReturn_t string_print(pPmObj_t pstr, uint8_t is_escaped);
#endif /* HAVE_PRINT */

/**
 * Clears the string cache if one exists.
 * Called by heap_init()
 *
 * @return Return status
 */
PmReturn_t string_cacheInit(void);


/** Returns a pointer to the base of the string cache */
PmReturn_t string_getCache(pPmString_t **r_ppstrcache);

/**
 * Returns a new string object that is the concatenation
 * of the two given strings.
 *
 * @param pstr1 First source string
 * @param pstr2 Second source string
 * @param r_pstring Return arg; ptr to new string object
 * @return Return status
 */
PmReturn_t
string_concat(pPmString_t pstr1, pPmString_t pstr2, pPmObj_t *r_pstring);

#ifdef HAVE_STRING_FORMAT
/**
 * Returns a new string object that is created from the given format string
 * and the argument(s).
 *
 * @param pstr Format string object
 * @param parg Single argument or tuple of arguments
 * @param r_pstring Return arg; ptr to new string object
 * @return Return status
 */
PmReturn_t string_format(pPmString_t pstr, pPmObj_t parg, pPmObj_t *r_pstring);

/**
 * Prints n bytes, formatting them if is_escaped is true
 *
 * @param pb Pointer to C bytes
 * @param is_escaped Boolean true if string is to be escaped
 * @param n Number of bytes to print
 * @return Return status
 */
PmReturn_t string_printFormattedBytes(uint8_t *pb,
                                      uint8_t is_escaped,
                                      uint16_t n);
#endif /* HAVE_STRING_FORMAT */

#endif /* __STRING_H__ */
