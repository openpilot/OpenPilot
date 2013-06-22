/* -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*-
 * MSHEAP
 * 
 * A compact, low-overhead heap implementation with configurable
 * integrity checking.
 *
 * Copyright 2011 Michael Smith. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    1. Redistributions of source code must retain the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdint.h>

/**
 * Initialise the heap.
 *
 * @param   base        The lower boundary of the heap.
 * @param   limit       The upper boundary of the heap.
 */
extern void msheap_init(void *base, void *limit);

/**
 * Allocate memory from the heap.
 *
 * @param   size        The number of bytes required (more may be allocated).
 */
extern void *msheap_alloc(uint32_t size);

/**
 * Free memory back to the heap.
 *
 * @param   ptr         Pointer being freed to the heap.
 */
extern void msheap_free(void *ptr);

/**
 * Validate the heap.
 *
 * @return              Zero if the heap integrity checks pass, nonzero
 *                      otherwise.
 */
extern int  msheap_check(void);

/**
 * Walk the heap.
 *
 * @param   callback    Called for each allocation in the heap.
 *                      The ptr argument gives the allocated address or
 *                      the free region address, size is the region size 
 *                      in bytes and free is nonzero if the region is free.
 */
extern void msheap_walk(void (* callback)(void *ptr, uint32_t size, int free));

/**
 * Return the amount of free space in the heap.
 *
 * @return              The total number of bytes available for allocation.
 */
extern uint32_t msheap_free_space(void);

/**
 * Extend the heap.
 *
 * @param   size        The size of the extension in bytes.
 */
extern void msheap_extend(uint32_t size);
