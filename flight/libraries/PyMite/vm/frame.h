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


#ifndef __FRAME_H__
#define __FRAME_H__


/**
 * \file
 * \brief VM Frame
 *
 * VM frame header.
 */


/**
 * The maximum number of local variables a native function can have.
 * This defines the length of the locals array in the native frame struct.
 */
#define NATIVE_MAX_NUM_LOCALS   8


/**
 * Block Type
 *
 * Numerical values to put in the 'b_type' field of the tPmBlockType struct.
 */
typedef enum PmBlockType_e
{
    /** Invalid block type */
    B_INVALID = 0,

    /** Loop type */
    B_LOOP,

    /** Try type */
    B_TRY
} PmBlockType_t, *pPmBlockType_t;


/**
 * Block
 *
 * Extra info for loops and trys (others?)
 * Frames use linked list of blocks to handle
 * nested loops and try-catch blocks.
 */
typedef struct PmBlock_s
{
    /** Obligatory obj descriptor */
    PmObjDesc_t od;

    /** Ptr to backup stack ptr */
    pPmObj_t *b_sp;

    /** Handler fxn obj */
    uint8_t const *b_handler;

    /** Block type */
    PmBlockType_t b_type:8;

    /** Next block in stack */
    struct PmBlock_s *next;
} PmBlock_t,
 *pPmBlock_t;


/**
 * Frame
 *
 * A struct that holds the execution frame of a function, including the stack,
 * local vars and pointer to the code object.
 *
 * This struct doesn't declare the stack.
 * frame_new() is responsible for allocating the extra memory
 * at the tail of fo_locals[] to hold both the locals and stack.
 */
typedef struct PmFrame_s
{
    /** Obligatory obj descriptor */
    PmObjDesc_t od;

    /** Ptr to previous frame obj */
    struct PmFrame_s *fo_back;

    /** Ptr to fxn obj */
    pPmFunc_t fo_func;

    /** Mem space where func's CO comes from */
    PmMemSpace_t fo_memspace:8;

    /** Instrxn ptr (pts into memspace) */
    uint8_t const *fo_ip;

    /** Linked list of blocks */
    pPmBlock_t fo_blockstack;

    /** Local attributes dict (non-fast locals) */
    pPmDict_t fo_attrs;

    /** Global attributes dict (pts to root frame's globals */
    pPmDict_t fo_globals;

    /** Points to next empty slot in fo_locals (1 past TOS) */
    pPmObj_t *fo_sp;

    /** Frame can be an import-frame that handles RETURN differently */
    uint8_t fo_isImport:1;

#ifdef HAVE_CLASSES
    /** Flag to indicate class initailzer frame; handle RETURN differently */
    uint8_t fo_isInit:1;
#endif /* HAVE_CLASSES */

    /** Array of local vars and stack (space appended at alloc) */
    pPmObj_t fo_locals[1];
    /* WARNING: Do not put new fields below fo_locals */
} PmFrame_t,
 *pPmFrame_t;


/**
 * Native Frame
 *
 * A struct that holds the execution frame of a native function,
 * including the args and single stack slot, and pointer to the code object.
 *
 * This struct doesn't need an OD because it is only used statically in the
 * globals struct.  There's only one native frame, the global one.
 * This happens because a native function is a leaf node
 * in the call tree (a native func can't call python funcs).
 */
typedef struct PmNativeFrame_s
{
    /** Object descriptor */
    PmObjDesc_t od;

    /** Ptr to previous frame obj */
    struct PmFrame_s *nf_back;

    /** Ptr to fxn obj */
    pPmFunc_t nf_func;

    /** Single stack slot */
    pPmObj_t nf_stack;

    /** Boolean to indicate if the native frame is active */
    uint8_t nf_active;

    /** Number of args passed to the native function */
    uint8_t nf_numlocals;

    /** Local vars */
    pPmObj_t nf_locals[NATIVE_MAX_NUM_LOCALS];
} PmNativeFrame_t,
 *pPmNativeFrame_t;


/**
 * Allocate space for a new frame, fill its fields
 * with respect to the given function object.
 * Return pointer to the new frame.
 *
 * @param   pfunc ptr to Function object.
 * @param   r_pobj Return value; the new frame.
 * @return  Return status.
 */
PmReturn_t frame_new(pPmObj_t pfunc, pPmObj_t *r_pobj);

#endif /* __FRAME_H__ */
