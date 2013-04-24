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

#include "msheap.h"

/*
 * Integrity checking.
 *
 * The HEAP_CHECK_LEVEL define determines the level of heap and
 * argument integrity checking that will be performed.
 *
 * Higher checking levels include all the checks from lower levels.
 *
 * 0 : Pointers passed to msheap_free are checked; this will
 *     detect overflowing of the freed region and overflows from
 *     adjacent regions into the freed region.
 *     msheap_panic() is called if an overflow is detected.
 *
 * 1 : Heap elements are checked during msheap_alloc(); this will
 *     detect overflowing of any region that is seen during the
 *     allocation.
 *     msheap_panic() is called if an overflow is detected.
 *
 * 2 : msheap_panic() is called with more informative diagnostics
 *     including the test that failed.
 *
 * 3 : The entire heap is checked on every call to msheap_alloc()
 *     and msheap_free().  This will detect overflowing of any
 *     region.
 *     Every argument is checked before use.
 *     Regions are checked after operations.
 *
 * -1: No integrity checking of any sort is performed.
 *
 * HEAP_CHECK_LEVEL defaults to 2 if DEBUG is defined, or 0
 * otherwise.
 */
#ifndef HEAP_CHECK_LEVEL
# ifdef DEBUG
#  define HEAP_CHECK_LEVEL  2
# else
#  define HEAP_CHECK_LEVEL  0
# endif
#endif

/**
 * Utility debug/checking macro.
 *
 * @param   _expr       Expression to evaluate.  Calls msheap_panic if the 
 *                      evaluation returns false.
 * @param   _str        String to pass to msheap_panic.
 * @return              Zero, but only if _expr evaluates true.
 */
#define __ASSERT(_expr, _str)                   \
    ({                                          \
        int _a __attribute__((unused)) = 0;     \
        if (!(_expr))                           \
            msheap_panic(_str);                 \
        _a;                                     \
    })

/** canonical macro weirdness */
#define __STR(_x)   # _x

/* at level 2+, add the expression string and line number */
#if HEAP_CHECK_LEVEL >= 2
# define __ASSERT_FMT(_expr, _line, _str)   __ASSERT(_expr, "heap assertion failed @" __STR(_line) ": " _str)
#else
# define __ASSERT_FMT(_expr, _line, _str)   __ASSERT(_expr, "heap assertion failed")
#endif

/** 
 * Assert if HEAP_CHECK_LEVEL >= _level and _expr evaluates false
 *
 * @param   _level      The lowest HEAP_CHECK_LEVEL at which this assertion should be tested.
 * @param   _expr       The expression to test.
 */
#define ASSERT(_level, _expr)       (void)((HEAP_CHECK_LEVEL < _level) || __ASSERT_FMT(_expr, __LINE__, #_expr))

/**
 * Assert if HEAP_CHECK_LEVEL >= _level and _expr evaluates false, return test result otherwise.
 *
 * @param   _level      The lowest HEAP_CHECK_LEVEL at which to assert.
 * @param   _expr       The expression to test.
 * @return              1 if _expr evaluates false, 0 otherwise.
 */
#define ASSERT_TEST(_level, _expr)              \
    ({                                          \
        ASSERT(_level, _expr);                  \
        (_expr) ? 0 : 1;                        \
    })


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
static const uintptr_t      marker_size = sizeof(struct marker);

/* heap boundaries */
static marker_t     heap_base;
static marker_t     heap_limit;
static uint32_t     heap_free;
static marker_t     free_hint;      /* likely free region, or heap_base if no free region hint */

/* rounding macros for powers of 2 */
#define round_down(_val, _boundary) ((_val) & ~(_boundary - 1))
#define round_up(_val, _boundary)   round_down((_val) + (_boundary) - 1, _boundary)

/* default panic handler */
void msheap_panic(const char *reason) __attribute__((weak, noreturn));

static int  region_check(marker_t marker);
static void split_region(marker_t marker, uint32_t size);
static void merge_region(marker_t marker);

/**
 * Initialise the heap.
 *
 * @param   base        The lower boundary of the heap.
 * @param   limit       The upper boundary of the heap.
 */
void
msheap_init(void *base, void *limit)
{
    heap_base = (marker_t)round_up((uintptr_t)base, marker_size);
    heap_limit = (marker_t)round_down((uintptr_t)limit, marker_size) - 1;

    ASSERT(3, heap_base);               /* must not be NULL */
    ASSERT(3, heap_limit);              /* must not be NULL */
    ASSERT(3, heap_limit > heap_base);  /* limit must be above base */

    /* Initial size of the free region (includes the heap_base marker) */
    heap_free = heap_limit - heap_base;
    ASSERT(0, heap_free <= max_free);   /* heap must not be too large */
    ASSERT(3, heap_free > 1);           /* heap must be at least 1 marker in size */

    /*
     * Initialise the base and limit markers.
     */
    heap_base->prev.size = 0;
    heap_base->prev.free = 0;
    heap_base->next.size = heap_free;
    heap_base->next.free = 1;
    heap_limit->prev.size = heap_free;
    heap_limit->prev.free = 1;
    heap_limit->next.size = 0;
    heap_limit->next.free = 0;

    free_hint = heap_base;              /* a good place to start ... */

    region_check(heap_base);
    region_check(heap_limit);
}

void *
msheap_alloc(uint32_t size)
{
    marker_t    cursor;
    marker_t    best;

    ASSERT(3, msheap_check());

    /* convert the passed-in size to the number of marker-size units we need to allocate */
    size += marker_size;
    size = round_up(size, marker_size);
    size /= marker_size;

    /* cannot possibly satisfy this allocation */
    if (size > heap_free)
        return 0;

    /* simple single-pass best-fit search */
restart:
    cursor = free_hint;
    best = 0;
    while (cursor != heap_limit) {

        ASSERT(1, region_check(cursor));

        /* if the region is free and large enough */
        if ((cursor->next.free) && (cursor->next.size >= size)) {

            /* if we have no candidate, or the new one is smaller, take it */
            if (!best || (cursor->next.size < best->next.size))
                best = cursor;
        }

        cursor += cursor->next.size;
    }

    if (!best) {
        /* 
         * If we were working from the hint and found nothing, reset
         * the hint and try again
         */
        if (free_hint != heap_base) { 
            free_hint = heap_base;
            goto restart; 
        }

        /* no space */
        return 0;
    }

    /* split the free region to make space */
    split_region(best, size);

    /* update free space counter */
    heap_free -= size;

    /* and return a pointer to the allocated region */
    return (void *)(best + 1);
}

void
msheap_free(void *ptr)
{
    marker_t    marker;

    marker = (marker_t)ptr - 1;

    ASSERT(0, region_check(marker));
    ASSERT(3, msheap_check());

    /* this region is free, mark it accordingly */
    marker->next.free = 1;
    (marker + marker->next.size)->prev.free = 1;

    /* account for space we are freeing */
    heap_free += marker->next.size;

    /* possibly merge this region and the following */
    merge_region(marker);

    /* possibly merge this region and the preceeding */
    if (marker->prev.free) {
        marker -= marker->prev.size;
        merge_region(marker);
    }

    /*
     * Marker now points to the new free region, so update
     * the free hint if this has opened space earlier in the heap.
     */
    if (marker < free_hint)
        free_hint = marker;
}

int
msheap_check(void)
{
    marker_t    cursor;
    uint32_t    free_space = 0;

    cursor = heap_base;                             /* start at the base of the heap */

    for (;;) {
        if (ASSERT_TEST(2, region_check(cursor)))   /* check the current region */
            return 0;
        if (cursor->next.free)                      /* if the region is free */
            free_space += cursor->next.size;        /* count it as free space */
        if (cursor == heap_limit)                   /* if this was the last region, stop */
            break;
        cursor += cursor->next.size;                /* next region */
    }

    if (ASSERT_TEST(2, region_check(free_hint)))
        return 0;
    if (ASSERT_TEST(2, free_space == heap_free))
        return 0;

    return 1;
}

void
msheap_walk(void (* callback)(void *ptr, uint32_t size, int free))
{
    marker_t    cursor;

    cursor = heap_base;
    for (;;) {
        callback(cursor + 1, cursor->next.size * marker_size, cursor->next.free);
        if (cursor == heap_limit)
            break;
        cursor += cursor->next.size;
    }
}

uint32_t
msheap_free_space(void)
{
    return heap_free * marker_size;
}

void
msheap_extend(uint32_t size)
{
    marker_t    new_free;

    /* convert to marker-sized units (and implicitly round down) */
    size /= marker_size;
    if (size < 1)
        return;

    /*
     * We can either extend a free region immediately prior to
     * the heap limit, or we can turn the heap limit marker
     * into the marker for a free region.
     */
    if (heap_limit->prev.free) {
        new_free = heap_limit - heap_limit->prev.size;
    } else {
        new_free = heap_limit;
    }

    /* update new free region */
    new_free->next.size += size;
    new_free->next.free = 1;

    /* new end marker */
    heap_limit = new_free + new_free->next.size;
    heap_limit->prev.size = new_free->next.size;
    heap_limit->prev.free = 1;
    heap_limit->next.size = 0;
    heap_limit->next.free = 0;

    ASSERT(3, msheap_check());
}

/**
 * Local heap panic implementation.
 *
 * Just sits and spins - should normally be overridden by the wrapper.
 *
 * @param   reason      The reason we are panicking.
 */
void
msheap_panic(const char *reason)
{
    for (;;)
        ;
}

/**
 * Check that a region is sane.
 *
 * If HEAP_CHECK_LEVEL is >= 2, assert at the point where the test fails, otherwise
 * expect that we are being called from inside an ASSERT wrapper at an appropriate
 * but lower level.
 *
 * @param   marker      The region to test.
 * @return              0 if the region fails checking, 1 otherwise.
 */
static int
region_check(marker_t marker)
{
    marker_t    other;

    if (ASSERT_TEST(2, marker) |                            /* not NULL */
        ASSERT_TEST(2, !((uintptr_t)marker % marker_size)) | /* properly aligned */
        ASSERT_TEST(2, marker >= heap_base) |               /* within the heap */
        ASSERT_TEST(2, marker <= heap_limit))
        return 0;

    /* validate link to next marker & return link from that marker */
    if (marker->next.size > 0) {
        other = marker + marker->next.size;

        if (ASSERT_TEST(2, other > marker) |                        /* must be after */
            ASSERT_TEST(2, other <= heap_limit) |                   /* must be inside the heap */
            ASSERT_TEST(2, marker->next.size == other->prev.size) | /* sizes must match */
            ASSERT_TEST(2, marker->next.free == other->prev.free))  /* free state must match */
            return 0;
    } else {
        if (ASSERT_TEST(2, marker == heap_limit))                   /* or it's the end of the heap */
            return 0;
    }

    /* validate link to previous marker & return link from that marker */
    if (marker->prev.size > 0) {
        other = marker - marker->prev.size;

        if (ASSERT_TEST(2, other < marker) |                        /* must be before */
            ASSERT_TEST(2, other >= heap_base) |                    /* must be inside the heap */
            ASSERT_TEST(2, marker->prev.size == other->next.size) | /* sizes must match */
            ASSERT_TEST(2, marker->prev.free == other->next.free))  /* free state must match */
            return 0;
    } else {
        if (ASSERT_TEST(2, marker == heap_base))                    /* or it's the end of the heap */
            return 0;
    }

    /* must never be two free regions adjacent */
    if (ASSERT_TEST(2, !(marker->prev.free && marker->next.free)))
        return 0;

    return 1;
}

/**
 * Split a free region into two and allocate the first portion.
 *
 * @param   marker      Marker at the head of the region to be split.
 * @param   size        Size of the portion to be allocated.
 */
static void
split_region(marker_t marker, uint32_t size)
{
    marker_t    split, tail;

    ASSERT(1, marker->next.free);           /* must be splitting a free region */
    ASSERT(1, !marker->prev.free);          /* free region must never follow a free region */
    ASSERT(1, marker->next.size >= size);   /* size must fit in region */

    ASSERT(3, size);                        /* split result must be at least one marker in size */

    tail = marker + marker->next.size;
    ASSERT(1, region_check(tail));          /* validate the following region */

    /* 
     * The split marker is at the end of the allocated region; it may actually
     * be at the end of the previous free region as well.
     */
    split = marker + size;
    
    /* describe the now-allocated region */
    split->prev.size = size;
    split->prev.free = 0;

    /* if there is a real split, then describe the free region */
    if (split != tail) {        
        split->next.size = marker->next.size - size;
        split->next.free = 1;
        tail->prev.size = split->next.size;
        tail->prev.free = 1;

        /*
         * Update the allocation speedup hint to
         * point to the new free region if we just used it.
         */
        if (free_hint == marker)
            free_hint = split;
    } else {

        /*
         * If we just allocated all of what the free hint
         * pointed to, reset it to the base of the heap.
         */
        if (free_hint == marker)
            free_hint = heap_base;
    }

    /* and update the allocated region */
    marker->next.size = size;
    marker->next.free = 0;

    ASSERT(3, region_check(marker));
    ASSERT(3, region_check(split));
    ASSERT(3, region_check(tail));
}

/**
 * Merge a free region with the following region, if possible.
 *
 * @param   marker  Marker preceeding the region to be merged.
 */
static void
merge_region(marker_t marker)
{
    marker_t    other;

    /* 
     * note - cannot region_check(marker) here as we are
     * actively fixing adjacent free regions.
     */

    other = marker + marker->next.size;

    /* if this region and the next region are both free, merge */
    if (marker->next.free && other->next.free) {

        /* update region size */
        marker->next.size += other->next.size;

        /* update the marker following the end of the merged regions */
        other = marker + marker->next.size;
        other->prev.size = marker->next.size;
    }
}

