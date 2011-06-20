/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Initcall infrastructure
 * @{
 * @addtogroup   PIOS_INITCALL Generic Initcall Macros
 * @brief Initcall Macros
 * @{
 *
 * @file       pios_initcall.h  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      Initcall header
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
/* 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 3 of the License, or 
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License along 
 * with this program; if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef PIOS_INITCALL_H
#define PIOS_INITCALL_H

/**
 * Just a stub define to make things compile.
 * Automatically link based initialization currently doesn't work
 * since posix really runs on a multitude of architectures
 * and we cannot define a linker script for each of them atm
 */

#define uavobj_initcall(fn)
#define module_initcall(ifn, iparam, sfn, sparam, flags)

#define MODULE_TASKCREATE_ALL();

#endif	/* PIOS_INITCALL_H */

/**
 * @}
 * @}
 */
