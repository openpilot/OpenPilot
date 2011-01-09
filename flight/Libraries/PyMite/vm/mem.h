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


#ifndef __MEM_H__
#define __MEM_H__


/**
 * \file
 * \brief VM Memory
 *
 * VM memory header.
 */


/**
 * Memory Space enum.
 *
 * Defines the different addressable areas of the system.
 */
typedef enum PmMemSpace_e
{
    MEMSPACE_RAM = 0,
    MEMSPACE_PROG,
    MEMSPACE_EEPROM,
    MEMSPACE_SEEPROM,
    MEMSPACE_OTHER0,
    MEMSPACE_OTHER1,
    MEMSPACE_OTHER2,
    MEMSPACE_OTHER3
} PmMemSpace_t, *pPmMemSpace_t;


/**
 * Returns the byte at the given address in memspace.
 *
 * Increments the address (just like getc and read(1))
 * to make image loading work (recursive).
 *
 * @param   memspace memory space/type
 * @param   paddr ptr to address
 * @return  byte from memory.
 *          paddr - points to the next byte
 */
#define mem_getByte(memspace, paddr) plat_memGetByte((memspace), (paddr))

/**
 * Returns the 2-byte word at the given address in memspace.
 *
 * Word obtained in LITTLE ENDIAN order (per Python convention).
 * afterward, addr points one byte past the word.
 *
 * @param   memspace memory space
 * @param   paddr ptr to address
 * @return  word from memory.
 *          addr - points one byte past the word
 */
uint16_t mem_getWord(PmMemSpace_t memspace, uint8_t const **paddr);

/**
 * Returns the 4-byte int at the given address in memspace.
 *
 * Int obtained in LITTLE ENDIAN order (per Python convention).
 * afterward, addr points one byte past the int.
 *
 * @param   memspace memory space
 * @param   paddr ptr to address
 * @return  int from memory.
 *          addr - points one byte past the word
 */
uint32_t mem_getInt(PmMemSpace_t memspace, uint8_t const **paddr);

#ifdef HAVE_FLOAT
/**
 * Returns the 4-byte float at the given address in memspace.
 *
 * Float obtained in LITTLE ENDIAN order (per Python convention).
 * afterward, addr points one byte past the float.
 *
 * @param   memspace memory space
 * @param   paddr ptr to address
 * @return  float from memory.
 *          addr - points one byte past the word
 */
float mem_getFloat(PmMemSpace_t memspace, uint8_t const **paddr);
#endif /* HAVE_FLOAT */

/**
 * Copies count number of bytes from src in memspace to dest in RAM.
 * Leaves dest and src pointing one byte past end of the data.
 *
 * @param   memspace memory space/type of source
 * @param   pdest ptr to destination address
 * @param   psrc  ptr to source address
 * @param   count number of bytes to copy
 * @return  nothing.
 *          src, dest - point 1 past end of data
 * @see     sli_memcpy
 */
void mem_copy(PmMemSpace_t memspace,
              uint8_t **pdest, uint8_t const **psrc, uint16_t count);

/**
 * Returns the number of bytes in the C string pointed to by pstr.
 * Does not modify pstr
 *
 * @param   memspace memory space/type of source
 * @param   pstr  ptr to source C string
 * @return  Number of bytes in the string.
 */
uint16_t mem_getStringLength(PmMemSpace_t memspace,
                             uint8_t const *const pstr);

/**
 * Compares a byte array in RAM to a byte array in the given memory space
 *
 * @param cname Pointer to byte array in RAM
 * @param cnamelen Length of byte array to compare
 * @param memspace Memory space of other byte array
 * @param paddr Pointer to address of other byte array
 * @return PM_RET_OK if all bytes in both arrays match; PM_RET_NO otherwise
 */
PmReturn_t mem_cmpn(uint8_t *cname, uint8_t cnamelen, PmMemSpace_t memspace,
                    uint8_t const **paddr);

#endif /* __MEM_H__ */
