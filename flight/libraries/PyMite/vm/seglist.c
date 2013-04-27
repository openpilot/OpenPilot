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
#define __FILE_ID__ 0x10


/**
 * \file
 * \brief Segmented list data type and operations
 *
 * The segmented list is used to implement the Python
 * List and Dict data types.
 * The segmented list is used in preference to the linked list
 * in order to reduce the memory overhead.
 *
 * Unused slots in the segments are expected to contain C_NULL.
 *
 * List implementation:
 * When used in a List, the Seglist.currseg field points
 * to the last segment in the list.
 * The function seglist_appendItem() should be used to append
 * items to the List.
 * Inserting and deleting List items is a more complicated
 * matter.
 *
 * Dict implementation:
 * The currseg field is meaningless since rootseg always
 * points to the current active segment.
 * The function seglist_pushItem() should be used to put
 * items in the Dict.
 * A Dict uses one Seglist for keys and another for values.
 * A Dict entry's (key, value) pair share the same index in
 * the Seglist.
 */


#include "pm.h"


/**
 * Set this to 1 if seglist_clear() should manually free its segments.
 * Set this to 0 if seglist_clear() should do nothing
 * and let the GC reclaim objects.
 */
#define SEGLIST_CLEAR_SEGMENTS 1


PmReturn_t
seglist_appendItem(pSeglist_t pseglist, pPmObj_t pobj)
{
    return seglist_insertItem(pseglist, pobj, pseglist->sl_length);
}


PmReturn_t
seglist_clear(pSeglist_t pseglist)
{
    pSegment_t pseg1 = C_NULL;
    pSegment_t pseg2 = C_NULL;

#if SEGLIST_CLEAR_SEGMENTS
    /* Deallocate all linked segments */
    pseg1 = ((pSeglist_t)pseglist)->sl_rootseg;
    while (pseg1 != C_NULL)
    {
        pseg2 = pseg1->next;
        PM_RETURN_IF_ERROR(heap_freeChunk((pPmObj_t)pseg1));
        pseg1 = pseg2;
    }
#endif

    /* Clear seglist fields */
    ((pSeglist_t)pseglist)->sl_rootseg = C_NULL;
    ((pSeglist_t)pseglist)->sl_lastseg = C_NULL;
    ((pSeglist_t)pseglist)->sl_length = 0;

    return PM_RET_OK;
}


PmReturn_t
seglist_findEqual(pSeglist_t pseglist, pPmObj_t pobj, int16_t *r_index)
{
    pSegment_t pseg;
    int8_t i = 0;
    uint8_t segindex;

    C_ASSERT(pseglist != C_NULL);
    C_ASSERT(pobj != C_NULL);
    C_ASSERT((*r_index >= 0));
    C_ASSERT((*r_index == 0) || (*r_index < pseglist->sl_length));

    /* Walk out to the starting segment */
    pseg = pseglist->sl_rootseg;
    for (i = (*r_index / SEGLIST_OBJS_PER_SEG); i > (int8_t)0; i--)
    {
        C_ASSERT(pseg != C_NULL);
        pseg = pseg->next;
    }

    /* Set the starting index within the segment */
    segindex = *r_index % SEGLIST_OBJS_PER_SEG;

    /* Search the remaining segments */
    for (; pseg != C_NULL; pseg = pseg->next)
    {
        while (segindex < SEGLIST_OBJS_PER_SEG)
        {
            /* If past the end of the seglist, return no item found */
            if (*r_index >= pseglist->sl_length)
            {
                return PM_RET_NO;
            }

            /* If items are equal, return with index of found item */
            if (obj_compare(pobj, pseg->s_val[segindex]) == C_SAME)
            {
                return PM_RET_OK;
            }

            /* Proceed to next item */
            segindex++;
            (*r_index)++;
        }

        /* Proceed to next segment */
        segindex = 0;
    }
    return PM_RET_NO;
}


PmReturn_t
seglist_getItem(pSeglist_t pseglist, int16_t index, pPmObj_t *r_pobj)
{
    pSegment_t pseg;
    int16_t i;

    C_ASSERT(pseglist != C_NULL);
    C_ASSERT(index >= 0);
    C_ASSERT(index < pseglist->sl_length);

    /* Walk out to the proper segment */
    pseg = pseglist->sl_rootseg;
    C_ASSERT(pseg != C_NULL);
    for (i = (index / SEGLIST_OBJS_PER_SEG); i > 0; i--)
    {
        pseg = pseg->next;
        C_ASSERT(pseg != C_NULL);
    }

    /* Return ptr to obj in this seg at the index */
    *r_pobj = pseg->s_val[index % SEGLIST_OBJS_PER_SEG];
    return PM_RET_OK;
}


PmReturn_t
seglist_insertItem(pSeglist_t pseglist, pPmObj_t pobj, int16_t index)
{
    PmReturn_t retval = PM_RET_OK;
    pSegment_t pseg = C_NULL;
    pPmObj_t pobj1 = C_NULL;
    pPmObj_t pobj2 = C_NULL;
    int8_t indx = (int8_t)0;
    int16_t i = 0;
    uint8_t *pchunk;

    C_ASSERT(index <= pseglist->sl_length);

    /* If a new seg is needed */
    if ((pseglist->sl_length % SEGLIST_OBJS_PER_SEG) == 0)
    {
        /* Alloc and init new segment */
        retval = heap_getChunk(sizeof(Segment_t), &pchunk);
        PM_RETURN_IF_ERROR(retval);
        pseg = (pSegment_t)pchunk;
        OBJ_SET_TYPE(pseg, OBJ_TYPE_SEG);
        sli_memset((unsigned char *)pseg->s_val,
                   0, SEGLIST_OBJS_PER_SEG * sizeof(pPmObj_t));
        pseg->next = C_NULL;

        /* If this is the first seg, set as root */
        if (pseglist->sl_rootseg == C_NULL)
        {
            pseglist->sl_rootseg = pseg;
        }

        /* Else append the seg to the end */
        else
        {
            pseglist->sl_lastseg->next = pseg;
        }

        /* Either way, this is now the last segment */
        pseglist->sl_lastseg = pseg;
    }

    /* Walk out to the segment for insertion */
    pseg = pseglist->sl_rootseg;
    C_ASSERT(pseg != C_NULL);
    for (i = (index / SEGLIST_OBJS_PER_SEG); i > 0; i--)
    {
        pseg = pseg->next;
        C_ASSERT(pseg != C_NULL);
    }

    /* Insert obj and ripple copy all those afterward */
    indx = index % SEGLIST_OBJS_PER_SEG;;
    pobj1 = pobj;
    while (pobj1 != C_NULL)
    {
        pobj2 = pseg->s_val[indx];
        pseg->s_val[indx] = pobj1;
        pobj1 = pobj2;
        indx++;

        /* If indx exceeds this seg, go to next seg */
        if ((indx >= SEGLIST_OBJS_PER_SEG) && (pobj1 != C_NULL))
        {
            pseg = pseg->next;
            C_ASSERT(pseg != C_NULL);
            indx = (int8_t)0;
        }
    }
    pseglist->sl_length++;
    return retval;
}


PmReturn_t
seglist_new(pSeglist_t *r_pseglist)
{
    PmReturn_t retval = PM_RET_OK;

    retval = heap_getChunk(sizeof(Seglist_t), (uint8_t **)r_pseglist);
    PM_RETURN_IF_ERROR(retval);

    OBJ_SET_TYPE(*r_pseglist, OBJ_TYPE_SGL);
    (*r_pseglist)->sl_rootseg = C_NULL;
    (*r_pseglist)->sl_lastseg = C_NULL;
    (*r_pseglist)->sl_length = 0;
    return retval;
}


PmReturn_t
seglist_setItem(pSeglist_t pseglist, pPmObj_t pobj, int16_t index)
{
    pSegment_t pseg;
    int16_t i;

    C_ASSERT(index <= pseglist->sl_length);

    /* Walk out to the proper segment */
    pseg = pseglist->sl_rootseg;
    C_ASSERT(pseg != C_NULL);
    for (i = (index / SEGLIST_OBJS_PER_SEG); i > 0; i--)
    {
        pseg = pseg->next;
        C_ASSERT(pseg != C_NULL);
    }

    /* Set item in this seg at the index */
    pseg->s_val[index % SEGLIST_OBJS_PER_SEG] = pobj;
    return PM_RET_OK;
}


PmReturn_t
seglist_removeItem(pSeglist_t pseglist, uint16_t index)
{
    pSegment_t pseg;
    int16_t i,
      k;

    C_ASSERT(index < pseglist->sl_length);

    /* Walk through the segments */
    pseg = pseglist->sl_rootseg;
    C_ASSERT(pseg != C_NULL);
    for (i = (index / SEGLIST_OBJS_PER_SEG); i > 0; i--)
    {
        pseg = pseg->next;
        C_ASSERT(pseg != C_NULL);
    }

    /*
     * pseg now points to the correct segment of the item to be removed, so
     * start ripple copying all following items up to the last
     * in the last segment
     */

    for (i = index; i < ((pseglist->sl_length) - 1); i++)
    {
        k = i % SEGLIST_OBJS_PER_SEG;

        /* Copy element i+1 to slot i */
        if ((k + 1) == SEGLIST_OBJS_PER_SEG)
        {
            /* Source is first item in next segment */
            pseg->s_val[i % SEGLIST_OBJS_PER_SEG] = (pseg->next)->s_val[0];
            pseg = pseg->next;
        }
        else
        {
            /* Source and target are in the same segment */
            pseg->s_val[k] = pseg->s_val[k + 1];
        }
    }

    pseglist->sl_length -= 1;

    /* Remove the last segment if it was emptied */
    if (pseglist->sl_length % SEGLIST_OBJS_PER_SEG == 0)
    {
        pseg = pseglist->sl_rootseg;

        /* Find the segment before the last */
        for (i = 0; i < ((pseglist->sl_length - 1) / SEGLIST_OBJS_PER_SEG);
             i++)
        {
            pseg = pseg->next;
            C_ASSERT(pseg != C_NULL);
        }
        if (pseg->next == C_NULL)
        {
            /*
             * Seglist is now completely empty and the last segment can be
             * recycled.
             */
#if SEGLIST_CLEAR_SEGMENTS
            PM_RETURN_IF_ERROR(heap_freeChunk((pPmObj_t)pseg));
#endif
            pseglist->sl_lastseg = C_NULL;
            pseglist->sl_rootseg = C_NULL;
        }
        else
        {
            /* At least one segment remains */
            pseglist->sl_lastseg = pseg;
            pseg->next = C_NULL;
        }
    }
    else
    {
        /* Zero out the now unused slot */
        pseg->s_val[pseglist->sl_length % SEGLIST_OBJS_PER_SEG] = C_NULL;
    }

    return PM_RET_OK;
}
