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

/*
 * The heap consists of ranges (free or allocated) separated and
 * bounded by markers.
 *
 * For maximum space efficiency, the default is to use 4-byte
 * 'compact' markers, which limits the heap to a maxium of 128KiB.
 * For larger heaps define HEAP_SUPPORT_LARGE, which doubles markers
 * to 8 bytes each, but allows heaps up to 2^^33 bytes in size.
 *
 * Each marker contains two structures, one describing the previous
 * region and one describing the next.  Thus, markers form a
 * doubly-linked list chaining each region together.
 *
 * Each region is described by two identical structures, providing
 * a measure of referential integrity that can be used to detect
 * overflows out of the region without the use of separate magic
 * numbers.
 *
 * The region descriptor size includes the size of the marker at its
 * head.  This means that zero is not a legal marker value.
 *
 * Free regions are always coalesced, and a pointer is kept to the
 * most recently-created free region to accelerate allocation in the
 * common case where a large number of free objects are allocated
 * early.
 *
 * The heap is bounded by markers pointing to zero-sized allocated
 * ranges, so they can never be merged.
 */
#ifdef HEAP_SUPPORT_LARGE

struct region_descriptor {
    uint32_t    size:31;    /* size of the region (including marker) in multiples of the marker size */
    uint32_t    free:1;     /* if nonzero, region is free */
};
static const uint32_t       max_free = 0x7fffffff;

#else /* !HEAP_SUPPORT_LARGE */

struct region_descriptor {
    uint16_t    size:15;    /* size of the region (including marker) in multiples of the marker size */
    uint16_t    free:1;     /* if nonzero, region is free */
};
static const uint32_t       max_free = 0x7fff;

#endif /* HEAP_SUPPORT_LARGE */

/**
 * The marker placed between regions.
 *
 * Allocations are aligned and rounded to the size of this structure.
 */
struct marker {
    struct region_descriptor    prev;
    struct region_descriptor    next;
};

typedef struct marker       *marker_t;

/* heap handle (boundaries) */
typedef struct {
	marker_t     heap_base;
	marker_t     heap_limit;
	uint32_t     heap_free;
	marker_t     free_hint;      /* likely free region, or heap_base if no free region hint */
} heap_handle_t;

/**
 * Initialise the heap.
 *
 * @param   base        The lower boundary of the heap.
 * @param   limit       The upper boundary of the heap.
 */
extern void msheap_init(heap_handle_t *heap, void *base, void *limit);

/**
 * Allocate memory from the heap.
 *
 * @param   size        The number of bytes required (more may be allocated).
 */
extern void *msheap_alloc(heap_handle_t *heap, uint32_t size);

/**
 * Free memory back to the heap.
 *
 * @param   ptr         Pointer being freed to the heap.
 */
extern void msheap_free(heap_handle_t *heap, void *ptr);

/**
 * Validate the heap.
 *
 * @return              Zero if the heap integrity checks pass, nonzero
 *                      otherwise.
 */
extern int  msheap_check(heap_handle_t *heap);

/**
 * Walk the heap.
 *
 * @param   callback    Called for each allocation in the heap.
 *                      The ptr argument gives the allocated address or
 *                      the free region address, size is the region size 
 *                      in bytes and free is nonzero if the region is free.
 */
extern void msheap_walk(heap_handle_t *heap, void (* callback)(void *ptr, uint32_t size, int free));

/**
 * Return the amount of free space in the heap.
 *
 * @return              The total number of bytes available for allocation.
 */
extern uint32_t msheap_free_space(heap_handle_t *heap);

/**
 * Extend the heap.
 *
 * @param   size        The size of the extension in bytes.
 */
extern void msheap_extend(heap_handle_t *heap, uint32_t size);
