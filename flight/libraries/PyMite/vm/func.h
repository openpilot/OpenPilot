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


#ifndef __FUNC_H__
#define __FUNC_H__

/**
 * \file
 * \brief Function Object Type
 *
 * Function object type header.
 */

/**
 * Function obj
 *
 * A function is like an instance of a code obj.
 * Contains ptr to its code obj and has its own attributes dict.
 *
 * The first (__main__) module that is executed has a function obj
 * created for it to execute the bytecode which builds the module.
 */
typedef struct PmFunc_s
{
    /** Object descriptor */
    PmObjDesc_t od;

    /** Ptr to code obj */
    pPmCo_t f_co;

    /** Ptr to attribute dict */
    pPmDict_t f_attrs;

    /** Ptr to globals dict */
    pPmDict_t f_globals;

#ifdef HAVE_DEFAULTARGS
    /** Ptr to tuple holding default args */
    pPmTuple_t f_defaultargs;
#endif /* HAVE_DEFAULTARGS */

#ifdef HAVE_CLOSURES
    /** Ptr to tuple of cell values */
    pPmTuple_t f_closure;
#endif /* HAVE_CLOSURES */

} PmFunc_t,
 *pPmFunc_t;


/**
 * Creates a Function Obj for the given Code Obj.
 * Allocate space for a Func obj and fill the fields.
 *
 * @param   pco ptr to code obj
 * @param   pglobals ptr to globals dict (from containing func/module)
 * @param   r_pfunc Return by reference; pointer to new function
 * @return  Return status
 */
PmReturn_t func_new(pPmObj_t pco, pPmObj_t pglobals, pPmObj_t *r_pfunc);

#endif /* __FUNC_H__ */
