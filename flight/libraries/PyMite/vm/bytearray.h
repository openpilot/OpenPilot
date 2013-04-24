/*
# This file is Copyright 2010 Dean Hall.
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


#ifndef __BYTEARRAY_H__
#define __BYTEARRAY_H__

/**
 * \file
 * \brief Bytearray Object Type
 *
 * Bytearray object type header.
 */


/**
 * Bytes container
 *
 * Holds actual byte payload
 */
typedef struct PmBytes_s
{
    /** Object descriptor */
    PmObjDesc_t od;

    /** Physical number of bytes in the C array (below) */
    int16_t length;

    /** C array of bytes */
    uint8_t val[1];
} PmBytes_t,
 *pPmBytes_t;


/**
 * Bytearray obj
 *
 * Mutable ordered sequence of bytes.  Contains ptr to chunk of bytes.
 */
typedef struct PmBytearray_s
{
    /** Object descriptor */
    PmObjDesc_t od;

    /** Bytearray length; logical number of bytes */
    int16_t length;

    /** Ptr to bytes container (may hold more bytes than length) */
    pPmBytes_t val;
} PmBytearray_t,
 *pPmBytearray_t;

 
PmReturn_t bytearray_new(pPmObj_t pobj, pPmObj_t *r_pobj);
PmReturn_t bytearray_getItem(pPmObj_t pobj, int16_t index, pPmObj_t *r_pobj);
PmReturn_t bytearray_setItem(pPmObj_t pba, int16_t index, pPmObj_t pobj);
PmReturn_t bytearray_print(pPmObj_t pobj);

#endif /* __BYTEARRAY_H__ */
