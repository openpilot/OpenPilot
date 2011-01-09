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


#ifndef __SLI_H__
#define __SLI_H__


/**
 * \file
 * \brief Standard Library Interface
 *
 * PyMite requires a few functions from a few different
 * standard C libraries (memory, string, etc).
 * If your microcontroller has these libraries,
 * set the constant to 1 for each library available.
 * This will cause a macro to be defined which wraps
 * the function for use by PyMite.
 * Otherwise, leave the constant as 0, and PyMite will
 * use the function defined in sli.c
 * Some of the functions in sli.c will need to be ported
 * to the target system.
 */


/**
 * If the compiler has string.h, set HAVE_STRING to 1;
 * otherwise, leave it 0 and the sli functions will be used.
 */
#define HAVE_STRING_H 0


/*
 * This section creates a macro or a function prototype
 * for each library based on the corresponding constant.
 * For example, if HAVE_STRING_H is defined to non-zero,
 * the system <string.h> file will be included,
 * and a macro "sli_strcmp" will be created to wrap the strcmp()
 * function.  But if HAVE_STRING is zero, the sli_strcmp()
 * prototype will be declared and sli_strcmp() must be
 * implemented in sli.c
 */

#if HAVE_STRING_H

#include <string.h>

#define sli_memcpy(to, from, n) memcpy((to), (from), (n))
#define sli_strcmp(s1, s2)      strcmp((s1),(s2))
#define sli_strlen(s)           strlen(s)
#define sli_strncmp(s1, s2, n)  strncmp((s1),(s2),(n))

#else

/**
 * Copies a block of memory in RAM.
 *
 * @param   to The destination address.
 * @param   from The source address.
 * @param   n The number of bytes to copy.
 * @return  The initial pointer value of the destination
 * @see     mem_copy
 */
void *sli_memcpy(unsigned char *to, unsigned char const *from, unsigned int n);

/**
 * Compares two strings.
 *
 * @param   s1 Ptr to string 1.
 * @param   s2 Ptr to string 2.
 * @return  value that is less then, equal to or greater than 0
 *          depending on whether s1's encoding is
 *          less than, equal to, or greater than s2's.
 */
int sli_strcmp(char const *s1, char const *s2);

/**
 * Obtain string length.
 *
 * @param   s ptr to string.
 * @return  number of bytes in string.
 */
int sli_strlen(char const *s);

/**
 * Compare strings for a specific length.
 *
 * @param   s1 ptr to string 1.
 * @param   s2 ptr to string 2.
 * @param   n number of chars to compare
 * @return  value that is less then, equal to or greater than 0
 *          depending on whether s1's encoding is
 *          less than, equal to, or greater than s2's.
 */
int sli_strncmp(char const *s1, char const *s2, unsigned int n);

#endif /* HAVE_STRING_H */

/**
 * Copy a value repeatedly into a block of memory
 *
 * @param   dest the destination address.
 * @param   val the value.
 * @param   n the number of bytes to copy.
 * @return  Nothing
 * @see     memset
 */
void sli_memset(unsigned char *dest, const char val, unsigned int n);

#endif /* __SLI_H__ */
