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


#undef __FILE_ID__
#define __FILE_ID__ 0x06


/**
 * \file
 * \brief VM Heap
 *
 * VM heap operations.
 * All of PyMite's dynamic memory is obtained from this heap.
 * The heap provides dynamic memory on demand.
 */


#include "pm.h"


/** Checks for heap size definition. */
#ifndef PM_HEAP_SIZE
#warning PM_HEAP_SIZE not defined in src/platform/<yourplatform>/pmfeatures.h
#elif PM_HEAP_SIZE & 3
#error PM_HEAP_SIZE is not a multiple of four
#endif


/** The size of the temporary roots stack */
#define HEAP_NUM_TEMP_ROOTS 24

/**
 * The maximum size a live chunk can be (a live chunk is one that is in use).
 * The live chunk size is limited by the size field in the *object* descriptor.
 * That field is nine bits with two assumed least significant bits (zeros):
 * (0x1FF << 2) == 2044
 */
#define HEAP_MAX_LIVE_CHUNK_SIZE 2044

/**
 * The maximum size a free chunk can be (a free chunk is one that is not in use).
 * The free chunk size is limited by the size field in the *heap* descriptor.
 * That field is fourteen bits with two assumed least significant bits (zeros):
 * (0x3FFF << 2) == 65532
 */
#define HEAP_MAX_FREE_CHUNK_SIZE 65532

/** The minimum size a chunk can be (rounded up to a multiple of 4) */
#define HEAP_MIN_CHUNK_SIZE ((sizeof(PmHeapDesc_t) + 3) & ~3)


/**
 * Gets the GC's mark bit for the object.
 * This MUST NOT be called on objects that are free.
 */
#define OBJ_GET_GCVAL(pobj) ((((pPmObj_t)pobj)->od >> OD_MARK_SHIFT) & 1)

/**
 * Sets the GC's mark bit for the object
 * This MUST NOT be called on objects that are free.
 */
#ifdef HAVE_GC
#define OBJ_SET_GCVAL(pobj, gcval) \
    do \
    { \
        ((pPmObj_t)pobj)->od = (gcval) ? ((pPmObj_t)pobj)->od | OD_MARK_BIT \
                                       : ((pPmObj_t)pobj)->od & ~OD_MARK_BIT;\
    } \
    while (0)
#else
#define OBJ_SET_GCVAL(pobj, gcval)
#endif /* HAVE_GC */


/**
 * The following is a diagram of the heap descriptor at the head of the chunk:
 * @verbatim
 *                MSb          LSb
 *                7 6 5 4 3 2 1 0
 *      pchunk-> +-+-+-+-+-+-+-+-+
 *               |     S[9:2]    |     S := Size of the chunk (2 LSbs dropped)
 *               +-+-+-----------+     F := Chunk free bit (not in use)
 *               |F|R| S[15:10]  |     R := Bit reserved for future use
 *               +-+-+-----------+
 *               |     P(L)      |     P := hd_prev: Pointer to previous node
 *               |     P(H)      |     N := hd_next: Pointer to next node
 *               |     N(L)      |
 *               |     N(H)      |     Theoretical min size == 6
 *               +---------------+     Effective min size == 8
 *               | unused space  |     (12 on 32-bit MCUs)
 *               ...           ...
 *               | end chunk     |
 *               +---------------+
 * @endverbatim
 */
typedef struct PmHeapDesc_s
{
    /** Heap descriptor */
    uint16_t hd;

    /** Ptr to prev heap chunk */
    struct PmHeapDesc_s *prev;

    /** Ptr to next heap chunk */
    struct PmHeapDesc_s *next;
} PmHeapDesc_t,
 *pPmHeapDesc_t;

typedef struct PmHeap_s
{
    /*
     * WARNING: Leave 'base' field at the top of struct to increase chance
     * of alignment when compiler doesn't recognize the aligned attribute
     * which is specific to GCC
     */
    /** Global declaration of heap. */
    uint8_t base[PM_HEAP_SIZE];

    /** Ptr to list of free chunks; sorted smallest to largest. */
    pPmHeapDesc_t pfreelist;

    /** The amount of heap space available in free list */
#if PM_HEAP_SIZE > 65535
    uint32_t avail;
#else
    uint16_t avail;
#endif

#ifdef HAVE_GC
    /** Garbage collection mark value */
    uint8_t gcval;

    /** Boolean to indicate if GC should run automatically */
    uint8_t auto_gc;

    /* #239: Fix GC when 2+ unlinked allocs occur */
    /** Stack of objects to be held as temporary roots */
    pPmObj_t temp_roots[HEAP_NUM_TEMP_ROOTS];

    uint8_t temp_root_index;
#endif                          /* HAVE_GC */

} PmHeap_t,
 *pPmHeap_t;


/** The PyMite heap */
static PmHeap_t pmHeap PM_PLAT_HEAP_ATTR;


#if 0
static void
heap_gcPrintFreelist(void)
{
    pPmHeapDesc_t pchunk = pmHeap.pfreelist;

    printf("DEBUG: pmHeap.avail = %d\n", pmHeap.avail);
    printf("DEBUG: freelist:\n");
    while (pchunk != C_NULL)
    {
        printf("DEBUG:     free chunk (%d bytes) @ 0x%0x\n",
               OBJ_GET_SIZE(pchunk), (int)pchunk);
        pchunk = pchunk->next;
    }
}
#endif


#if 0
/** DEBUG: dumps the heap and roots list to a file */
static void
heap_dump(void)
{
    static int n = 0;
    uint16_t s;
    uint32_t i;
    void *b;
    char filename[32];
    FILE *fp;

    snprintf(filename, 32, "pmheapdump%02d.bin", n++);
    fp = fopen(filename, "wb");

    /* magic : PMDUMP for little endian or PMUDMP for big endian */
    fwrite(&"PM", 1, 2, fp);
    s = 0x5544;
    fwrite(&s, sizeof(uint16_t), 1, fp);
    fwrite(&"MP", 1, 2, fp);

    /* pointer size */
    s = sizeof(intptr_t);
    fwrite(&s, sizeof(uint16_t), 1, fp);

    /* dump version */
    s = 1;
    fwrite(&s, sizeof(uint16_t), 1, fp);    

    /* pmfeatures */
    s = 0;
#ifdef USE_STRING_CACHE
    s |= 1<<0;
#endif
#ifdef HAVE_DEFAULTARGS
    s |= 1<<1;
#endif
#ifdef HAVE_CLOSURES
    s |= 1<<2;
#endif
#ifdef HAVE_CLASSES
    s |= 1<<3;
#endif
    fwrite(&s, sizeof(uint16_t), 1, fp);

    /* size of heap */
    i = PM_HEAP_SIZE;
    fwrite(&i, sizeof(uint32_t), 1, fp);

    /* Write base address of heap */
    b=&pmHeap.base;
    fwrite((void*)(&b), sizeof(intptr_t), 1, fp);

    /* Write contents of heap */
    fwrite(&pmHeap.base, 1, PM_HEAP_SIZE, fp);

    /* Write num roots*/
    i = 10;
    fwrite(&i, sizeof(uint32_t), 1, fp);

    /* Write heap root ptrs */
    fwrite((void *)&gVmGlobal.pnone, sizeof(intptr_t), 1, fp);
    fwrite((void *)&gVmGlobal.pfalse, sizeof(intptr_t), 1, fp);
    fwrite((void *)&gVmGlobal.ptrue, sizeof(intptr_t), 1, fp);
    fwrite((void *)&gVmGlobal.pzero, sizeof(intptr_t), 1, fp);
    fwrite((void *)&gVmGlobal.pone, sizeof(intptr_t), 1, fp);
    fwrite((void *)&gVmGlobal.pnegone, sizeof(intptr_t), 1, fp);
    fwrite((void *)&gVmGlobal.pcodeStr, sizeof(intptr_t), 1, fp);
    fwrite((void *)&gVmGlobal.builtins, sizeof(intptr_t), 1, fp);
    fwrite((void *)&gVmGlobal.nativeframe, sizeof(intptr_t), 1, fp);
    fwrite((void *)&gVmGlobal.threadList, sizeof(intptr_t), 1, fp);
    fclose(fp);
}
#endif


/* Removes the given chunk from the free list; leaves list in sorted order */
static PmReturn_t
heap_unlinkFromFreelist(pPmHeapDesc_t pchunk)
{
    C_ASSERT(pchunk != C_NULL);

    pmHeap.avail -= OBJ_GET_SIZE(pchunk);

    if (pchunk->next != C_NULL)
    {
        pchunk->next->prev = pchunk->prev;
    }

    /* If pchunk was the first chunk in the free list, update the heap ptr */
    if (pchunk->prev == C_NULL)
    {
        pmHeap.pfreelist = pchunk->next;
    }
    else
    {
        pchunk->prev->next = pchunk->next;
    }

    return PM_RET_OK;
}


/* Inserts in order a chunk into the free list.  Caller adjusts heap state */
static PmReturn_t
heap_linkToFreelist(pPmHeapDesc_t pchunk)
{
    uint16_t size;
    pPmHeapDesc_t pscan;

    /* Ensure the object is already free */
    C_ASSERT(OBJ_GET_FREE(pchunk) != 0);

    pmHeap.avail += OBJ_GET_SIZE(pchunk);

    /* If free list is empty, add to head of list */
    if (pmHeap.pfreelist == C_NULL)
    {
        pmHeap.pfreelist = pchunk;
        pchunk->next = C_NULL;
        pchunk->prev = C_NULL;

        return PM_RET_OK;
    }

    /* Scan free list for insertion point */
    pscan = pmHeap.pfreelist;
    size = OBJ_GET_SIZE(pchunk);
    while ((OBJ_GET_SIZE(pscan) < size) && (pscan->next != C_NULL))
    {
        pscan = pscan->next;
    }

    /*
     * Insert chunk after the scan chunk (next is NULL).
     * This is a slightly rare case where the last chunk in the free list
     * is smaller than the chunk being freed.
     */
    if (size > OBJ_GET_SIZE(pscan))
    {
        pchunk->next = pscan->next;
        pscan->next = pchunk;
        pchunk->prev = pscan;
    }

    /* Insert chunk before the scan chunk */
    else
    {
        pchunk->next = pscan;
        pchunk->prev = pscan->prev;

        /* If chunk will be first item in free list */
        if (pscan->prev == C_NULL)
        {
            pmHeap.pfreelist = pchunk;
        }
        else
        {
            pscan->prev->next = pchunk;
        }
        pscan->prev = pchunk;
    }

    return PM_RET_OK;
}


/*
 * Initializes the heap state variables
 */
PmReturn_t
heap_init(void)
{
    pPmHeapDesc_t pchunk;

#if PM_HEAP_SIZE > 65535
    uint32_t hs;
#else
    uint16_t hs;
#endif

#if __DEBUG__
    /* Fill the heap with a non-NULL value to bring out any heap bugs. */
    sli_memset(pmHeap.base, 0xAA, sizeof(pmHeap.base));
#endif

    /* Init heap globals */
    pmHeap.pfreelist = C_NULL;
    pmHeap.avail = 0;
#ifdef HAVE_GC
    pmHeap.gcval = (uint8_t)0;
    pmHeap.temp_root_index = (uint8_t)0;
    heap_gcSetAuto(C_TRUE);
#endif /* HAVE_GC */

    /* Create as many max-sized chunks as possible in the freelist */
    for (pchunk = (pPmHeapDesc_t)pmHeap.base, hs = PM_HEAP_SIZE;
         hs >= HEAP_MAX_FREE_CHUNK_SIZE; hs -= HEAP_MAX_FREE_CHUNK_SIZE)
    {
        OBJ_SET_FREE(pchunk, 1);
        OBJ_SET_SIZE(pchunk, HEAP_MAX_FREE_CHUNK_SIZE);
        heap_linkToFreelist(pchunk);
        pchunk =
            (pPmHeapDesc_t)((uint8_t *)pchunk + HEAP_MAX_FREE_CHUNK_SIZE);
    }

    /* Add any leftover memory to the freelist */
    if (hs >= HEAP_MIN_CHUNK_SIZE)
    {
        /* Round down to a multiple of four */
        hs = hs & ~3;
        OBJ_SET_FREE(pchunk, 1);
        OBJ_SET_SIZE(pchunk, hs);
        heap_linkToFreelist(pchunk);
    }

    C_DEBUG_PRINT(VERBOSITY_LOW, "heap_init(), id=%p, s=%d\n",
                  pmHeap.base, pmHeap.avail);

    string_cacheInit();

    return PM_RET_OK;
}


/**
 * Obtains a chunk of memory from the free list
 *
 * Performs the Best Fit algorithm.
 * Iterates through the freelist to see if a chunk of suitable size exists.
 * Shaves a chunk to perfect size iff the remainder is greater than
 * the minimum chunk size.
 *
 * @param size Requested chunk size
 * @param r_pchunk Return ptr to chunk
 * @return Return status
 */
static PmReturn_t
heap_getChunkImpl(uint16_t size, uint8_t **r_pchunk)
{
    PmReturn_t retval;
    pPmHeapDesc_t pchunk;
    pPmHeapDesc_t premainderChunk;

    C_ASSERT(r_pchunk != C_NULL);

    /* Skip to the first chunk that can hold the requested size */
    pchunk = pmHeap.pfreelist;
    while ((pchunk != C_NULL) && (OBJ_GET_SIZE(pchunk) < size))
    {
        pchunk = pchunk->next;
    }

    /* No chunk of appropriate size was found, raise OutOfMemory exception */
    if (pchunk == C_NULL)
    {
        *r_pchunk = C_NULL;
        PM_RAISE(retval, PM_RET_EX_MEM);
        return retval;
    }

    /* Remove the chunk from the free list */
    retval = heap_unlinkFromFreelist(pchunk);
    PM_RETURN_IF_ERROR(retval);

    /* Check if a chunk should be carved from what is available */
    if (OBJ_GET_SIZE(pchunk) - size >= HEAP_MIN_CHUNK_SIZE)
    {
        /* Create the heap descriptor for the remainder chunk */
        premainderChunk = (pPmHeapDesc_t)((uint8_t *)pchunk + size);
        OBJ_SET_FREE(premainderChunk, 1);
        OBJ_SET_SIZE(premainderChunk, OBJ_GET_SIZE(pchunk) - size);

        /* Put the remainder chunk back in the free list */
        retval = heap_linkToFreelist(premainderChunk);
        PM_RETURN_IF_ERROR(retval);

        /* Convert the chunk from a heap descriptor to an object descriptor */
        OBJ_SET_SIZE(pchunk, 0);
        OBJ_SET_FREE(pchunk, 0);
        OBJ_SET_SIZE(pchunk, size);

        C_DEBUG_PRINT(VERBOSITY_HIGH,
                      "heap_getChunkImpl()carved, id=%p, s=%d\n", pchunk,
                      size);
    }
    else
    {
        /* Set chunk's type to none (overwrites size field's high byte) */
        OBJ_SET_TYPE((pPmObj_t)pchunk, OBJ_TYPE_NON);
        OBJ_SET_FREE(pchunk, 0);

        C_DEBUG_PRINT(VERBOSITY_HIGH,
                      "heap_getChunkImpl()exact, id=%p, s=%d\n", pchunk,
                      OBJ_GET_SIZE(pchunk));
    }

    /*
     * Set the chunk's GC mark so it will be collected during the next GC cycle
     * if it is not reachable
     */
    OBJ_SET_GCVAL(pchunk, pmHeap.gcval);

    /* Return the chunk */
    *r_pchunk = (uint8_t *)pchunk;

    return retval;
}


/*
 * Allocates chunk of memory.
 * Filters out invalid sizes.
 * Rounds the size up to the next multiple of 4.
 * Obtains a chunk of at least the desired size.
 */
PmReturn_t
heap_getChunk(uint16_t requestedsize, uint8_t **r_pchunk)
{
    PmReturn_t retval;
    uint16_t adjustedsize;

    /* Ensure size request is valid */
    if (requestedsize > HEAP_MAX_LIVE_CHUNK_SIZE)
    {
        PM_RAISE(retval, PM_RET_EX_MEM);
        return retval;
    }

    else if (requestedsize < HEAP_MIN_CHUNK_SIZE)
    {
        requestedsize = HEAP_MIN_CHUNK_SIZE;
    }

    /*
     * Round up the size to a multiple of 4 bytes.
     * This maintains alignment on 32-bit platforms (required).
     */
    adjustedsize = ((requestedsize + 3) & ~3);

    /* Attempt to get a chunk */
    retval = heap_getChunkImpl(adjustedsize, r_pchunk);

#ifdef HAVE_GC
    /* Perform GC if out of memory, gc is enabled and not in native session */
    if ((retval == PM_RET_EX_MEM) && (pmHeap.auto_gc == C_TRUE)
        && (gVmGlobal.nativeframe.nf_active == C_FALSE))
    {
        retval = heap_gcRun();
        PM_RETURN_IF_ERROR(retval);

        /* Attempt to get a chunk */
        retval = heap_getChunkImpl(adjustedsize, r_pchunk);
    }
#endif /* HAVE_GC */

    /* Ensure that the pointer is 4-byte aligned */
    if (retval == PM_RET_OK)
    {
        C_ASSERT(((intptr_t)*r_pchunk & 3) == 0);
    }

    return retval;
}


/* Releases chunk to the free list */
PmReturn_t
heap_freeChunk(pPmObj_t ptr)
{
    PmReturn_t retval;

    C_DEBUG_PRINT(VERBOSITY_HIGH, "heap_freeChunk(), id=%p, s=%d\n",
                  ptr, OBJ_GET_SIZE(ptr));

    /* Ensure the chunk falls within the heap */
    C_ASSERT(((uint8_t *)ptr >= pmHeap.base)
             && ((uint8_t *)ptr < pmHeap.base + PM_HEAP_SIZE));

    /* Insert the chunk into the freelist */
    OBJ_SET_FREE(ptr, 1);

    /* Clear type so that heap descriptor's size's upper byte is zero */
    OBJ_SET_TYPE(ptr, 0);
    retval = heap_linkToFreelist((pPmHeapDesc_t)ptr);
    PM_RETURN_IF_ERROR(retval);

    return retval;
}


/* Returns, by reference, the number of bytes available in the heap */
#if PM_HEAP_SIZE > 65535
uint32_t
#else
uint16_t
#endif
heap_getAvail(void)
{
    return pmHeap.avail;
}


#ifdef HAVE_GC
/*
 * Marks the given object and the objects it references.
 *
 * @param   pobj Any non-free heap object
 * @return  Return code
 */
static PmReturn_t
heap_gcMarkObj(pPmObj_t pobj)
{
    PmReturn_t retval = PM_RET_OK;
    int16_t i = 0;
    int16_t n;
    PmType_t type;

    /* Return if ptr is null or object is already marked */
    if (pobj == C_NULL)
    {
        return retval;
    }
    if (OBJ_GET_GCVAL(pobj) == pmHeap.gcval)
    {
        return retval;
    }

    /* The pointer must be within the heap (native frame is special case) */
    C_ASSERT((((uint8_t *)pobj >= &pmHeap.base[0])
              && ((uint8_t *)pobj <= &pmHeap.base[PM_HEAP_SIZE]))
             || ((uint8_t *)pobj == (uint8_t *)&gVmGlobal.nativeframe));

    /* The object must not already be free */
    C_ASSERT(OBJ_GET_FREE(pobj) == 0);

    type = (PmType_t)OBJ_GET_TYPE(pobj);
    switch (type)
    {
            /* Objects with no references to other objects */
        case OBJ_TYPE_NON:
        case OBJ_TYPE_INT:
        case OBJ_TYPE_FLT:
        case OBJ_TYPE_STR:
        case OBJ_TYPE_NOB:
        case OBJ_TYPE_BOOL:
        case OBJ_TYPE_CIO:
            OBJ_SET_GCVAL(pobj, pmHeap.gcval);
            break;

        case OBJ_TYPE_TUP:
            i = ((pPmTuple_t)pobj)->length;

            /* Mark tuple head */
            OBJ_SET_GCVAL(pobj, pmHeap.gcval);

            /* Mark each obj in tuple */
            while (--i >= 0)
            {
                retval = heap_gcMarkObj(((pPmTuple_t)pobj)->val[i]);
                PM_RETURN_IF_ERROR(retval);
            }
            break;

        case OBJ_TYPE_LST:

            /* Mark the list */
            OBJ_SET_GCVAL(pobj, pmHeap.gcval);

            /* Mark the seglist */
            retval = heap_gcMarkObj((pPmObj_t)((pPmList_t)pobj)->val);
            break;

        case OBJ_TYPE_DIC:
            /* Mark the dict head */
            OBJ_SET_GCVAL(pobj, pmHeap.gcval);

            /* Mark the keys seglist */
            retval = heap_gcMarkObj((pPmObj_t)((pPmDict_t)pobj)->d_keys);
            PM_RETURN_IF_ERROR(retval);

            /* Mark the vals seglist */
            retval = heap_gcMarkObj((pPmObj_t)((pPmDict_t)pobj)->d_vals);
            break;

        case OBJ_TYPE_COB:
            /* Mark the code obj head */
            OBJ_SET_GCVAL(pobj, pmHeap.gcval);

            /* Mark the names tuple */
            retval = heap_gcMarkObj((pPmObj_t)((pPmCo_t)pobj)->co_names);
            PM_RETURN_IF_ERROR(retval);

            /* Mark the consts tuple */
            retval = heap_gcMarkObj((pPmObj_t)((pPmCo_t)pobj)->co_consts);
            PM_RETURN_IF_ERROR(retval);

            /* #122: Mark the code image if it is in RAM */
            if (((pPmCo_t)pobj)->co_memspace == MEMSPACE_RAM)
            {
                retval = heap_gcMarkObj((pPmObj_t)
                                        (((pPmCo_t)pobj)->co_codeimgaddr));
                PM_RETURN_IF_ERROR(retval);
            }

#ifdef HAVE_CLOSURES
            /* #256: Add support for closures */
            /* Mark the cellvars tuple */
            retval = heap_gcMarkObj((pPmObj_t)((pPmCo_t)pobj)->co_cellvars);
#endif /* HAVE_CLOSURES */
            break;

        case OBJ_TYPE_MOD:
        case OBJ_TYPE_FXN:
            /* Module and Func objs are implemented via the PmFunc_t */
            /* Mark the func obj head */
            OBJ_SET_GCVAL(pobj, pmHeap.gcval);

            /* Mark the code obj */
            retval = heap_gcMarkObj((pPmObj_t)((pPmFunc_t)pobj)->f_co);
            PM_RETURN_IF_ERROR(retval);

            /* Mark the attr dict */
            retval = heap_gcMarkObj((pPmObj_t)((pPmFunc_t)pobj)->f_attrs);
            PM_RETURN_IF_ERROR(retval);

            /* Mark the globals dict */
            retval = heap_gcMarkObj((pPmObj_t)((pPmFunc_t)pobj)->f_globals);
            PM_RETURN_IF_ERROR(retval);

#ifdef HAVE_DEFAULTARGS
            /* Mark the default args tuple */
            retval = heap_gcMarkObj((pPmObj_t)((pPmFunc_t)pobj)->f_defaultargs);
            PM_RETURN_IF_ERROR(retval);
#endif /* HAVE_DEFAULTARGS */

#ifdef HAVE_CLOSURES
            /* #256: Mark the closure tuple */
            retval = heap_gcMarkObj((pPmObj_t)((pPmFunc_t)pobj)->f_closure);
#endif /* HAVE_CLOSURES */
            break;

#ifdef HAVE_CLASSES
        case OBJ_TYPE_CLI:
            /* Mark the obj head */
            OBJ_SET_GCVAL(pobj, pmHeap.gcval);

            /* Mark the class */
            retval = heap_gcMarkObj((pPmObj_t)((pPmInstance_t)pobj)->cli_class);
            PM_RETURN_IF_ERROR(retval);

            /* Mark the attrs dict */
            retval = heap_gcMarkObj((pPmObj_t)((pPmInstance_t)pobj)->cli_attrs);
            break;

        case OBJ_TYPE_MTH:
            /* Mark the obj head */
            OBJ_SET_GCVAL(pobj, pmHeap.gcval);

            /* Mark the instance */
            retval = heap_gcMarkObj((pPmObj_t)((pPmMethod_t)pobj)->m_instance);
            PM_RETURN_IF_ERROR(retval);

            /* Mark the func */
            retval = heap_gcMarkObj((pPmObj_t)((pPmMethod_t)pobj)->m_func);
            PM_RETURN_IF_ERROR(retval);

            /* Mark the attrs dict */
            retval = heap_gcMarkObj((pPmObj_t)((pPmMethod_t)pobj)->m_attrs);
            break;

        case OBJ_TYPE_CLO:
            /* Mark the obj head */
            OBJ_SET_GCVAL(pobj, pmHeap.gcval);

            /* Mark the attrs dict */
            retval = heap_gcMarkObj((pPmObj_t)((pPmClass_t)pobj)->cl_attrs);
            PM_RETURN_IF_ERROR(retval);

            /* Mark the base tuple */
            retval = heap_gcMarkObj((pPmObj_t)((pPmClass_t)pobj)->cl_bases);
            break;
#endif /* HAVE_CLASSES */

            /*
             * An obj in ram should not be of these types.
             * Images arrive in RAM as string objects (image is array of bytes)
             */
        case OBJ_TYPE_CIM:
        case OBJ_TYPE_NIM:
            PM_RAISE(retval, PM_RET_EX_SYS);
            return retval;

        case OBJ_TYPE_FRM:
        {
            pPmObj_t *ppobj2 = C_NULL;

            /* Mark the frame obj head */
            OBJ_SET_GCVAL(pobj, pmHeap.gcval);

            /* Mark the previous frame, if this isn't a generator's frame */
            /* Issue #129: Fix iterator losing its object */
            if ((((pPmFrame_t)pobj)->fo_func->f_co->co_flags & CO_GENERATOR) == 0)
            {
                retval = heap_gcMarkObj((pPmObj_t)((pPmFrame_t)pobj)->fo_back);
                PM_RETURN_IF_ERROR(retval);
            }

            /* Mark the fxn obj */
            retval = heap_gcMarkObj((pPmObj_t)((pPmFrame_t)pobj)->fo_func);
            PM_RETURN_IF_ERROR(retval);

            /* Mark the blockstack */
            retval = heap_gcMarkObj((pPmObj_t)
                                    ((pPmFrame_t)pobj)->fo_blockstack);
            PM_RETURN_IF_ERROR(retval);

            /* Mark the attrs dict */
            retval = heap_gcMarkObj((pPmObj_t)((pPmFrame_t)pobj)->fo_attrs);
            PM_RETURN_IF_ERROR(retval);

            /* Mark the globals dict */
            retval = heap_gcMarkObj((pPmObj_t)((pPmFrame_t)pobj)->fo_globals);
            PM_RETURN_IF_ERROR(retval);

            /* Mark each obj in the locals list and the stack */
            ppobj2 = ((pPmFrame_t)pobj)->fo_locals;
            while (ppobj2 < ((pPmFrame_t)pobj)->fo_sp)
            {
                retval = heap_gcMarkObj(*ppobj2);
                PM_RETURN_IF_ERROR(retval);
                ppobj2++;
            }
            break;
        }

        case OBJ_TYPE_BLK:
            /* Mark the block obj head */
            OBJ_SET_GCVAL(pobj, pmHeap.gcval);

            /* Mark the next block in the stack */
            retval = heap_gcMarkObj((pPmObj_t)((pPmBlock_t)pobj)->next);
            break;

        case OBJ_TYPE_SGL:
            /* Mark the seglist obj head */
            OBJ_SET_GCVAL(pobj, pmHeap.gcval);

            /* Mark the seglist's segments */
            n = ((pSeglist_t)pobj)->sl_length;
            pobj = (pPmObj_t)((pSeglist_t)pobj)->sl_rootseg;
            for (i = 0; i < n; i++)
            {
                /* Mark the segment item */
                retval = heap_gcMarkObj(((pSegment_t)pobj)->s_val[i % SEGLIST_OBJS_PER_SEG]);
                PM_RETURN_IF_ERROR(retval);

                /* Mark the segment obj head */
                if ((i % SEGLIST_OBJS_PER_SEG) == 0)
                {
                    OBJ_SET_GCVAL(pobj, pmHeap.gcval);
                }

                /* Point to the next segment */
                else
                if ((i % SEGLIST_OBJS_PER_SEG) == (SEGLIST_OBJS_PER_SEG - 1))
                {
                    pobj = (pPmObj_t)((pSegment_t)pobj)->next;
                    if (pobj == C_NULL)
                    {
                        break;
                    }
                }
            }
            break;

        case OBJ_TYPE_SQI:
            /* Mark the sequence iterator obj head */
            OBJ_SET_GCVAL(pobj, pmHeap.gcval);

            /* Mark the sequence */
            retval = heap_gcMarkObj(((pPmSeqIter_t)pobj)->si_sequence);
            break;

        case OBJ_TYPE_THR:
            /* Mark the thread obj head */
            OBJ_SET_GCVAL(pobj, pmHeap.gcval);

            /* Mark the current frame */
            retval = heap_gcMarkObj((pPmObj_t)((pPmThread_t)pobj)->pframe);
            break;

        case OBJ_TYPE_NFM:
            /*
             * Mark the obj desc.  This doesn't really do much since the
             * native frame is declared static (not from the heap), but this
             * is here in case that ever changes
             */
            OBJ_SET_GCVAL(pobj, pmHeap.gcval);

            /* Mark the native frame's remaining fields if active */
            if (gVmGlobal.nativeframe.nf_active)
            {
                /* Mark the frame stack */
                retval = heap_gcMarkObj((pPmObj_t)
                                        gVmGlobal.nativeframe.nf_back);
                PM_RETURN_IF_ERROR(retval);

                /* Mark the function object */
                retval = heap_gcMarkObj((pPmObj_t)
                                        gVmGlobal.nativeframe.nf_func);
                PM_RETURN_IF_ERROR(retval);

                /* Mark the stack object */
                retval = heap_gcMarkObj(gVmGlobal.nativeframe.nf_stack);
                PM_RETURN_IF_ERROR(retval);

                /* Mark the args to the native func */
                for (i = 0; i < NATIVE_GET_NUM_ARGS(); i++)
                {
                    retval =
                        heap_gcMarkObj(gVmGlobal.nativeframe.nf_locals[i]);
                    PM_RETURN_IF_ERROR(retval);
                }
            }
            break;

#ifdef HAVE_BYTEARRAY
        case OBJ_TYPE_BYA:
            OBJ_SET_GCVAL(pobj, pmHeap.gcval);

            retval = heap_gcMarkObj((pPmObj_t)((pPmBytearray_t)pobj)->val);
            break;

        case OBJ_TYPE_BYS:
            OBJ_SET_GCVAL(pobj, pmHeap.gcval);
            break;
#endif /* HAVE_BYTEARRAY */

        default:
            /* There should be no invalid types */
            PM_RAISE(retval, PM_RET_EX_SYS);
            break;
    }
    return retval;
}


/*
 * Marks the root objects so they won't be collected during the sweep phase.
 * Recursively marks all objects reachable from the roots.
 */
static PmReturn_t
heap_gcMarkRoots(void)
{
    PmReturn_t retval;
    uint8_t i;

    /* Toggle the GC marking value so it differs from the last run */
    pmHeap.gcval ^= 1;

    /* Mark the constant objects */
    retval = heap_gcMarkObj(PM_NONE);
    PM_RETURN_IF_ERROR(retval);
    retval = heap_gcMarkObj(PM_FALSE);
    PM_RETURN_IF_ERROR(retval);
    retval = heap_gcMarkObj(PM_TRUE);
    PM_RETURN_IF_ERROR(retval);
    retval = heap_gcMarkObj(PM_ZERO);
    PM_RETURN_IF_ERROR(retval);
    retval = heap_gcMarkObj(PM_ONE);
    PM_RETURN_IF_ERROR(retval);
    retval = heap_gcMarkObj(PM_NEGONE);
    PM_RETURN_IF_ERROR(retval);
    retval = heap_gcMarkObj(PM_CODE_STR);
    PM_RETURN_IF_ERROR(retval);

    /* Mark the builtins dict */
    retval = heap_gcMarkObj(PM_PBUILTINS);
    PM_RETURN_IF_ERROR(retval);

    /* Mark the native frame if it is active */
    retval = heap_gcMarkObj((pPmObj_t)&gVmGlobal.nativeframe);
    PM_RETURN_IF_ERROR(retval);

    /* Mark the thread list */
    retval = heap_gcMarkObj((pPmObj_t)gVmGlobal.threadList);
    PM_RETURN_IF_ERROR(retval);

    /* Mark the temporary roots */
    for (i = 0; i < pmHeap.temp_root_index; i++)
    {
        retval = heap_gcMarkObj(pmHeap.temp_roots[i]);
        PM_RETURN_IF_ERROR(retval);
    }

    return retval;
}


#if USE_STRING_CACHE
/**
 * Unlinks free objects from the string cache.
 * This function must only be called by the GC after the heap has been marked
 * and before the heap has been swept.
 *
 * This solves the problem where a string object would be collected
 * but its chunk was still linked into the free list
 *
 * @param gcval The current value for chunks marked by the GC
 */
static PmReturn_t
heap_purgeStringCache(uint8_t gcval)
{
    PmReturn_t retval;
    pPmString_t *ppstrcache;
    pPmString_t pstr;

    /* Update string cache pointer if the first string objs are not marked */
    retval = string_getCache(&ppstrcache);
    if (ppstrcache == C_NULL)
    {
        return retval;
    }
    while ((*ppstrcache != C_NULL) && (OBJ_GET_GCVAL(*ppstrcache) != gcval))
    {
        *ppstrcache = (*ppstrcache)->next;
    }
    if (*ppstrcache == C_NULL)
    {
        return retval;
    }

    /* Unlink remaining strings that are not marked */
    for (pstr = *ppstrcache; pstr->next != C_NULL;)
    {
        /* Unlink consecutive non-marked strings */
        while ((pstr->next != C_NULL) && (OBJ_GET_GCVAL(pstr->next) != gcval))
        {
            pstr->next = pstr->next->next;
        }

        /* If not at end of cache, string must be marked, skip it */
        if (pstr->next != C_NULL)
        {
            pstr = pstr->next;
        }
    }

    return retval;
}
#endif


/*
 * Reclaims any object that does not have a current mark.
 * Puts it in the free list.  Coalesces all contiguous free chunks.
 */
static PmReturn_t
heap_gcSweep(void)
{
    PmReturn_t retval;
    pPmObj_t pobj;
    pPmHeapDesc_t pchunk;
    uint16_t totalchunksize;

#if USE_STRING_CACHE
    retval = heap_purgeStringCache(pmHeap.gcval);
#endif

    /* Start at the base of the heap */
    pobj = (pPmObj_t)pmHeap.base;
    while ((uint8_t *)pobj < &pmHeap.base[PM_HEAP_SIZE])
    {
        /* Skip to the next unmarked or free chunk within the heap */
        while (!OBJ_GET_FREE(pobj)
               && (OBJ_GET_GCVAL(pobj) == pmHeap.gcval)
               && ((uint8_t *)pobj < &pmHeap.base[PM_HEAP_SIZE]))
        {
            pobj = (pPmObj_t)((uint8_t *)pobj + OBJ_GET_SIZE(pobj));
        }

        /* Stop if reached the end of the heap */
        if ((uint8_t *)pobj >= &pmHeap.base[PM_HEAP_SIZE])
        {
            break;
        }

        /* Accumulate the sizes of all consecutive unmarked or free chunks */
        totalchunksize = 0;

        /* Coalesce all contiguous free chunks */
        pchunk = (pPmHeapDesc_t)pobj;
        while (OBJ_GET_FREE(pchunk)
               || (!OBJ_GET_FREE(pchunk)
                   && (OBJ_GET_GCVAL(pchunk) != pmHeap.gcval)))
        {
            if ((totalchunksize + OBJ_GET_SIZE(pchunk))
                > HEAP_MAX_FREE_CHUNK_SIZE)
            {
                break;
            }
            totalchunksize = totalchunksize + OBJ_GET_SIZE(pchunk);

            /*
             * If the chunk is already free, unlink it because its size
             * is about to change
             */
            if (OBJ_GET_FREE(pchunk))
            {
                retval = heap_unlinkFromFreelist(pchunk);
                PM_RETURN_IF_ERROR(retval);
            }

            /* Otherwise free and reclaim the unmarked chunk */
            else
            {
                OBJ_SET_TYPE(pchunk, 0);
                OBJ_SET_FREE(pchunk, 1);
            }

            C_DEBUG_PRINT(VERBOSITY_HIGH, "heap_gcSweep(), id=%p, s=%d\n",
                          pchunk, OBJ_GET_SIZE(pchunk));

            /* Proceed to the next chunk */
            pchunk = (pPmHeapDesc_t)
                ((uint8_t *)pchunk + OBJ_GET_SIZE(pchunk));

            /* Stop if it's past the end of the heap */
            if ((uint8_t *)pchunk >= &pmHeap.base[PM_HEAP_SIZE])
            {
                break;
            }
        }

        /* Set the heap descriptor data */
        OBJ_SET_FREE(pobj, 1);
        OBJ_SET_SIZE(pobj, totalchunksize);

        /* Insert chunk into free list */
        retval = heap_linkToFreelist((pPmHeapDesc_t)pobj);
        PM_RETURN_IF_ERROR(retval);

        /* Continue to the next chunk */
        pobj = (pPmObj_t)pchunk;
    }

    return PM_RET_OK;
}


/* Runs the mark-sweep garbage collector */
PmReturn_t
heap_gcRun(void)
{
    PmReturn_t retval;

    /* #239: Fix GC when 2+ unlinked allocs occur */
    /* This assertion fails when there are too many objects on the temporary
     * root stack and a GC occurs; consider increasing PM_HEAP_NUM_TEMP_ROOTS
     */
    C_ASSERT(pmHeap.temp_root_index < HEAP_NUM_TEMP_ROOTS);

    C_DEBUG_PRINT(VERBOSITY_LOW, "heap_gcRun()\n");
    /*heap_dump();*/

    retval = heap_gcMarkRoots();
    PM_RETURN_IF_ERROR(retval);

    retval = heap_gcSweep();
    /*heap_dump();*/
    return retval;
}


/* Enables or disables automatic garbage collection */
PmReturn_t
heap_gcSetAuto(uint8_t auto_gc)
{
    pmHeap.auto_gc = auto_gc;
    return PM_RET_OK;
}

void heap_gcPushTempRoot(pPmObj_t pobj, uint8_t *r_objid)
{
    if (pmHeap.temp_root_index < HEAP_NUM_TEMP_ROOTS)
    {
        *r_objid = pmHeap.temp_root_index;
        pmHeap.temp_roots[pmHeap.temp_root_index] = pobj;
        pmHeap.temp_root_index++;
    }
    return;
}


void heap_gcPopTempRoot(uint8_t objid)
{
    pmHeap.temp_root_index = objid;
}

#else

void heap_gcPushTempRoot(pPmObj_t pobj, uint8_t *r_objid) {}
void heap_gcPopTempRoot(uint8_t objid) {}

#endif /* HAVE_GC */
