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


#ifndef __PM_EMPTY_PLATFORM_DEFS_H__
#define __PM_EMPTY_PLATFORM_DEFS_H__


/**
 * \file
 * \brief Empty platform-specific definitions
 *
 * This file #defines as blank any undefined platform-specific
 * definitions.
 */

/**
 * Define a processor-specific specifier for use in declaring the heap.
 * If not defined, make it empty.
 * See <code>pmHeap</code> in heap.c for its use, which is:<br>
 * <code>static PmHeap_t pmHeap PM_PLAT_HEAP_ATTR;</code>
 */
#if !defined(PM_PLAT_HEAP_ATTR) || defined(__DOXYGEN__)
#define PM_PLAT_HEAP_ATTR
#endif

#endif /* __PM_EMPTY_PLATFORM_DEFS_H__ */
