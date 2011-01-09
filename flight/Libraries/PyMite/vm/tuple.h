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


#ifndef __TUPLE_H__
#define __TUPLE_H__


/**
 * \file
 * \brief Tuple Object Type
 *
 * Tuple object type header.
 */

/**
 * Tuple obj
 *
 * Immutable ordered sequence.  Contains array of ptrs to objs.
 */
typedef struct PmTuple_s
{
    /** Object descriptor */
    PmObjDesc_t od;

    /**
     * Length of tuple
     * I don't expect a tuple to ever exceed 255 elements,
     * but if I set this type to int8_t, a 0-element tuple
     * is too small to be allocated.
     */
    int16_t length;

    /** Array of ptrs to objs */
    pPmObj_t val[1];
} PmTuple_t,
 *pPmTuple_t;


#define tuple_copy(src, dest) tuple_replicate((src), 1, (dest))


/**
 * Creates a Tuple by loading a tuple image from memory.
 *
 * Obtain space for tuple from the heap.
 * Load all objs within the tuple img.
 * Leave contents of paddr pointing one byte past end of
 * last obj in tuple.
 *
 * The tuple image has the following structure:
 *      -type:      S8 - OBJ_TYPE_TUPLE
 *      -length     U8 - N number of objects in the tuple.
 *                  N objects follow in the stream.
 *
 * @param   memspace Memory space.
 * @param   paddr Ptr to ptr to tuple in memspace
 * @param   r_ptuple Return by reference; new filled tuple
 * @return  Return status
 */
PmReturn_t tuple_loadFromImg(PmMemSpace_t memspace,
                             uint8_t const **paddr, pPmObj_t *r_ptuple);

/**
 * Allocates space for a new Tuple.  Returns a pointer to the tuple.
 *
 * @param   n the number of elements the tuple will contain
 * @param   r_ptuple Return by ref, ptr to new tuple
 * @return  Return status
 */
PmReturn_t tuple_new(uint16_t n, pPmObj_t *r_ptuple);

/**
 * Replicates a tuple, n number of times to create a new tuple
 *
 * Copies the pointers (not the objects).
 *
 * @param   ptup Ptr to source tuple.
 * @param   n Number of times to replicate the tuple.
 * @param   r_ptuple Return arg; Ptr to new tuple.
 * @return  Return status
 */
PmReturn_t tuple_replicate(pPmObj_t ptup, int16_t n, pPmObj_t *r_ptuple);

/**
 * Gets the object in the tuple at the index.
 *
 * @param   ptup Ptr to tuple obj
 * @param   index Index into tuple
 * @param   r_pobj Return by reference; ptr to item
 * @return  Return status
 */
PmReturn_t tuple_getItem(pPmObj_t ptup, int16_t index, pPmObj_t *r_pobj);

#ifdef HAVE_PRINT
/**
 * Prints out a tuple. Uses obj_print() to print elements.
 *
 * @param pobj Object to print.
 * @return Return status
 */
PmReturn_t tuple_print(pPmObj_t pobj);
#endif /* HAVE_PRINT */

#endif /* __TUPLE_H__ */
