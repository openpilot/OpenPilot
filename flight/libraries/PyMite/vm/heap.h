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


#ifndef __HEAP_H__
#define __HEAP_H__


/**
 * \file
 * \brief VM Heap
 *
 * VM heap header.
 */


/**
 * The threshold of heap.avail under which the interpreter will run the GC
 * just before starting a native session.
 */
#define HEAP_GC_NF_THRESHOLD (512)


#ifdef __DEBUG__
#define DEBUG_PRINT_HEAP_AVAIL(s) \
    do { uint16_t n; heap_getAvail(&n); printf(s "heap avail = %d\n", n); } \
    while (0)
#else
#define DEBUG_PRINT_HEAP_AVAIL(s)
#endif


/**
 * Initializes the heap for use.
 *
 * @return  nothing.
 */
PmReturn_t heap_init(void);

/**
 * Returns a free chunk from the heap.
 *
 * The chunk will be at least the requested size.
 * The actual size can be found in the return chunk's od.od_size.
 *
 * @param   requestedsize Requested size of the chunk in bytes.
 * @param   r_pchunk Addr of ptr to chunk (return).
 * @return  Return code
 */
PmReturn_t heap_getChunk(uint16_t requestedsize, uint8_t **r_pchunk);

/**
 * Places the chunk back in the heap.
 *
 * @param   ptr Pointer to object to free.
 */
PmReturn_t heap_freeChunk(pPmObj_t ptr);

/** @return  Return number of bytes available in the heap */
#if PM_HEAP_SIZE > 65535
uint32_t
#else
uint16_t
#endif
  heap_getAvail(void);

#ifdef HAVE_GC
/**
 * Runs the mark-sweep garbage collector
 *
 * @return  Return code
 */
PmReturn_t heap_gcRun(void);

/**
 * Enables (if true) or disables automatic garbage collection
 *
 * @param   bool Value to enable or disable auto GC
 * @return  Return code
 */
PmReturn_t heap_gcSetAuto(uint8_t auto_gc);

#endif /* HAVE_GC */

/**
 * Pushes an object onto the temporary roots stack if there is room
 * to protect the objects from a potential garbage collection
 *
 * @param pobj Object to push onto the roots stack
 * @param r_objid By reference; ID to use when popping the object from the stack
 */
void heap_gcPushTempRoot(pPmObj_t pobj, uint8_t *r_objid);

/**
 * Pops from the temporary roots stack all objects upto and including the one
 * denoted by the given ID 
 *
 * @param objid ID of object to pop
 */
void heap_gcPopTempRoot(uint8_t objid);

#endif /* __HEAP_H__ */
