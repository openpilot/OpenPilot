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


#ifndef __MODULE_H__
#define __MODULE_H__


/**
 * \file
 * \brief Module Object Type
 *
 * Module object type header.
 */


/**
 * Creates a Module Obj for the given Code Obj.
 *
 * Use a func struct to represent the Module obj because
 * the module's construction code must execute later,
 * but set the type to OBJ_TYPE_MOD so that it is
 * not otherwise callable.
 *
 * @param   pco Ptr to code obj
 * @param   pmod Return by reference; ptr to new module obj
 * @return  Return status
 */
PmReturn_t mod_new(pPmObj_t pco, pPmObj_t *pmod);

/**
 * Imports a module of the given name.
 * Searches for an image with a matching name.
 * A code obj is created for the code image.
 * A module obj is created for the code obj.
 *
 * @param   pstr String obj containing name of code obj to load.
 * @param   pmod Return by reference; ptr to imported module
 * @return  Return status
 */
PmReturn_t mod_import(pPmObj_t pstr, pPmObj_t *pmod);

#endif /* __MODULE_H__ */
