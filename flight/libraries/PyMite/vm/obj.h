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


#ifndef __OBJ_H__
#define __OBJ_H__


/**
 * \file
 * \brief Object Type
 *
 * Object type header.
 */


/** Object descriptor field constants */
#define OD_MARK_SHIFT 14
#define OD_FREE_SHIFT 15
#define OD_MARK_BIT (uint16_t)(1 << OD_MARK_SHIFT)
#define OD_FREE_BIT (uint16_t)(1 << OD_FREE_SHIFT)
#define OD_SIZE_MASK (uint16_t)(0x01FF)
#define OD_TYPE_MASK (uint16_t)(0x3E00)
#define OD_TYPE_SHIFT 9

/** Heap descriptor size mask */
#define HD_SIZE_MASK (uint16_t)(0x3FFF)


/**
 * Gets the free bit of the given object to the given value.
 * If the object is marked free, it is not being used by the VM.
 */
#define OBJ_GET_FREE(pobj) \
    ((((pPmObj_t)pobj)->od >> OD_FREE_SHIFT) & (uint8_t)1)

/**
 * Sets the free bit of the given object to the given value.
 * Setting the free bit means that the object will use the heap descriptor
 * structure instead of the object descriptor structure.
 */
#define OBJ_SET_FREE(pobj, free) \
    do \
    { \
        ((pPmObj_t)pobj)->od = ((uint8_t)free) \
                               ? ((pPmObj_t)pobj)->od | OD_FREE_BIT \
                               : ((pPmObj_t)pobj)->od & ~OD_FREE_BIT;\
    } \
    while (0)

/*
 * #99: od_size bits are shifted because size is a scaled value
 * True size is always a multiple of 4, so the lower two bits are ignored
 * and two more significant bits are gained.
 */
/**
 * Gets the size of the chunk in bytes.
 * Tests whether the object is free as that determines whether the chunk is
 * using an object descriptor or a heap descriptor.  Heap descriptors have
 * a larger size field and use a different bit mask than object descriptors.
 */
#define OBJ_GET_SIZE(pobj) \
    ((((pPmObj_t)pobj)->od & OD_FREE_BIT) \
     ? ((((pPmObj_t)pobj)->od & HD_SIZE_MASK) << 2) \
     : ((((pPmObj_t)pobj)->od & OD_SIZE_MASK) << 2))

/**
 * Sets the size of the chunk in bytes.
 * Tests whether the object is free as that determines whether the chunk is
 * using an object descriptor or a heap descriptor.  Heap descriptors have
 * a larger size field and use a different bit mask than object descriptors.
 */
#define OBJ_SET_SIZE(pobj, size) \
    do \
    { \
        if (((pPmObj_t)pobj)->od & OD_FREE_BIT) \
        { \
            ((pPmObj_t)pobj)->od &= ~HD_SIZE_MASK; \
            ((pPmObj_t)pobj)->od |= (((size) >> 2) & HD_SIZE_MASK); \
        } \
        else \
        { \
            ((pPmObj_t)pobj)->od &= ~OD_SIZE_MASK; \
            ((pPmObj_t)pobj)->od |= (((size) >> 2) & OD_SIZE_MASK); \
        } \
    } \
    while (0)

/**
 * Gets the type of the object
 * This MUST NOT be called on objects that are free.
 */
#define OBJ_GET_TYPE(pobj) \
    (((((pPmObj_t)pobj)->od) & OD_TYPE_MASK) >> OD_TYPE_SHIFT)

/**
 * Sets the type of the object
 * This MUST NOT be called on objects that are free.
 */
#define OBJ_SET_TYPE(pobj, type) \
    do \
    { \
        ((pPmObj_t)pobj)->od &= ~OD_TYPE_MASK; \
        ((pPmObj_t)pobj)->od |= (((type) << OD_TYPE_SHIFT) & OD_TYPE_MASK); \
    } \
    while (0)


/**
 * Object type enum
 *
 * These values go in the od_type fields of the obj descriptor.
 * Be sure these values correspond to those in the image creator
 * tool.
 * The hashable types are grouped together for convenience.
 *
 * WARNING: od_type must be at most 5 bits! (must be < 0x20)
 */
typedef enum PmType_e
{
    OBJ_TYPE_HASHABLE_MIN = 0x00,

    /** None */
    OBJ_TYPE_NON = 0x00,

    /** Signed integer */
    OBJ_TYPE_INT = 0x01,

    /** Floating point 32b */
    OBJ_TYPE_FLT = 0x02,

    /** String */
    OBJ_TYPE_STR = 0x03,

    /** Tuple (immutable sequence) */
    OBJ_TYPE_TUP = 0x04,

    /** Code obj */
    OBJ_TYPE_COB = 0x05,

    /** Module obj */
    OBJ_TYPE_MOD = 0x06,

    /** Class obj */
    OBJ_TYPE_CLO = 0x07,

    /** Function obj (callable) */
    OBJ_TYPE_FXN = 0x08,

    /** Class instance */
    OBJ_TYPE_CLI = 0x09,

    /** Code image in static memory */
    OBJ_TYPE_CIM = 0x0A,

    /** Native function image */
    OBJ_TYPE_NIM = 0x0B,

    /** Native function object */
    OBJ_TYPE_NOB = 0x0C,

    /** Thread */
    OBJ_TYPE_THR = 0x0D,

    /** Boolean object */
    OBJ_TYPE_BOOL = 0x0F,

    /** Code image object */
    OBJ_TYPE_CIO = 0x10,

    /** Method object */
    OBJ_TYPE_MTH = 0x11,

    /* All types after this are not hashable */
    OBJ_TYPE_HASHABLE_MAX = 0x11,

    /** List (mutable sequence) */
    OBJ_TYPE_LST = 0x12,

    /** Dictionary (hash table) */
    OBJ_TYPE_DIC = 0x13,

#ifdef HAVE_BYTEARRAY
    /** Bytearray (mutable) */
    OBJ_TYPE_BYA = 0x14,
#endif /* HAVE_BYTEARRAY */

    /* All types after this are not accessible to the user */
    OBJ_TYPE_ACCESSIBLE_MAX = 0x18,

#ifdef HAVE_BYTEARRAY
    /** Bytes (mutable container for Bytearray type) */
    OBJ_TYPE_BYS = 0x18,
#endif /* HAVE_BYTEARRAY */

    /** Frame type */
    OBJ_TYPE_FRM = 0x19,

    /** Block type (for,while,try,etc) */
    OBJ_TYPE_BLK = 0x1A,

    /** Segment (within a seglist) */
    OBJ_TYPE_SEG = 0x1B,

    /** Seglist */
    OBJ_TYPE_SGL = 0x1C,

    /** Sequence iterator */
    OBJ_TYPE_SQI = 0x1D,

    /** Native frame (there is only one) */
    OBJ_TYPE_NFM = 0x1E,
} PmType_t, *pPmType_t;


/**
 * Object Descriptor
 *
 * All active PyMite "objects" must have this at the top of their struct.
 * (CodeObj, Frame, Dict, List, Tuple, etc.).
 *
 * The following is a diagram of the object descriptor:
 * \verbatim
 *              MSb           LSb
 *               7 6 5 4 3 2 1 0
 *     pchunk-> +-+-+-+-+-+-+-+-+     S := Size of the chunk (2 LSbs dropped)
 *              |     S[9:2]    |     F := Free bit
 *              +-+-+---------+-+     M := GC Mark bit
 *              |F|M|    T    |S|     T := Object type (PyMite specific)
 *              +-+-+---------+-+
 *              | object data   |
 *              ...           ...
 *              | end data      |     Theoretical min size == 2
 *              +---------------+     Effective min size == 8
 *                                    (due to pmHeapDesc_t)
 * \endverbatim
 * Macros are used to get and set field values.
 * Using macros eliminates declaring bit fields which fails on some compilers.
 */
typedef uint16_t PmObjDesc_t,
 *pPmObjDesc_t;

/**
 * Object
 *
 * The abstract empty object type for PyMite.
 */
typedef struct PmObj_s
{
    /** Object descriptor */
    PmObjDesc_t od;
} PmObj_t,
 *pPmObj_t;

/** Boolean object */
typedef struct PmBoolean_s
{
    /** Object descriptor */
    PmObjDesc_t od;

    /** Boolean value */
    int32_t val;
}
PmBoolean_t, *pPmBoolean_t;


/**
 * Loads an object from an image in memory.
 * Return pointer to object.
 * Leave add pointing one byte past end of obj.
 *
 * The following lists the simple object types
 * and their image structures:
 * -None:
 *      -type:      int8_t - OBJ_TYPE_NON
 *
 * -Int:
 *      -type:      int8_t - OBJ_TYPE_INT
 *      -value:     int32_t - signed integer value
 *
 * -Float:
 *      -type:      int8_t - OBJ_TYPE_FLOAT
 *      -value:     float32_t - 32-bit floating point value
 *
 * -Slice (is this allowed in img?):
 *      -type:      int8_t - OBJ_TYPE_SLICE
 *      -index1:    int16_t - first index.
 *      -index2:    int16_t - second index.
 *
 * @param   memspace memory space/type
 * @param   paddr ptr to ptr to obj
 *          return by reference: paddr pts to
 *          first byte after obj
 * @param   r_pobj Return arg, the loaded object.
 * @return  Return status
 */
PmReturn_t obj_loadFromImg(PmMemSpace_t memspace,
                           uint8_t const **paddr, pPmObj_t *r_pobj);

/**
 * Loads a code object from a code image object
 *
 * @param pimg Ptr to a code image object
 * @param r_pobj Return arg, the loaded object
 * @return  Returns status
 */
PmReturn_t obj_loadFromImgObj(pPmObj_t pimg, pPmObj_t *r_pobj);

/**
 * Finds the boolean value of the given object.
 *
 * @param   pobj Ptr to object to test.
 * @return  Nonzero value if object is False.
 */
int8_t obj_isFalse(pPmObj_t pobj);

/**
 * Returns the boolean true if the item is in the object
 *
 * @param   pobj Ptr to container object
 * @param   pitem Ptr to item
 */
PmReturn_t obj_isIn(pPmObj_t pobj, pPmObj_t pitem);

/**
 * Compares two objects for equality.
 *
 * @param   pobj1 Ptr to first object.
 * @param   pobj2 Ptr to second object.
 * @return  C_SAME if the items are equivalent, C_DIFFER otherwise.
 */
int8_t obj_compare(pPmObj_t pobj1, pPmObj_t pobj2);

/**
 * Print an object, thereby using objects helpers.
 *
 * @param   pobj Ptr to object for printing.
 * @param   is_expr_repr Influences the way None and strings are printed.
 *                       If 0, None is printed, strings are printed.
 *                       If 1, None is not printed and strings are printed
 *                       surrounded with single quotes and unprintable
 *                       characters are escaped.
 * @param   is_nested    Influences the way None and strings are printed.
 *                       If 1, None will be printed and strings will be
 *                       surrounded with single quotes and escaped.
 *                       This argument overrides the is_expr_repr argument.
 * @return  Return status
 */
PmReturn_t obj_print(pPmObj_t pobj, uint8_t is_expr_repr, uint8_t is_nested);

#ifdef HAVE_BACKTICK
/**
 * Returns by reference a string object that is the human-readable
 * representation of the object. Used by the backtick operation (UNARY_CONVERT).
 *
 * @param pobj Ptr to object to represent
 * @param r_pstr Return arg, the string object
 * @return Return status
 */
PmReturn_t obj_repr(pPmObj_t pobj, pPmObj_t *r_pstr);
#endif /* HAVE_BACKTICK */

#endif /* __OBJ_H__ */
