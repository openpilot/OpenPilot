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

/* 
 * This implementation is heavily based on the Linux Kernel initcall
 * infrastructure:
 *   http://lxr.linux.no/#linux/include/linux/init.h
 *   http://git.kernel.org/?p=linux/kernel/git/torvalds/linux-2.6.git;a=blob;f=include/linux/init.h
 */

/*
 * Used for initialization calls..
 */

typedef int32_t (*initcall_t)(void);
typedef struct {
	initcall_t fn_minit;
	initcall_t fn_tinit;
} initmodule_t;

/* Init module section */
extern initmodule_t __module_initcall_start[], __module_initcall_end[];

/* initcalls are now grouped by functionality into separate 
 * subsections. Ordering inside the subsections is determined
 * by link order.
 *
 * The `id' arg to __define_initcall() is needed so that multiple initcalls
 * can point at the same handler without causing duplicate-symbol build errors.
 */

#define __define_initcall(level,fn,id) \
	static initcall_t __initcall_##fn##id __attribute__((__used__)) \
	__attribute__((__section__(".initcall" level ".init"))) = fn

#define __define_module_initcall(level, ifn, sfn) \
	static initmodule_t __initcall_##fn __attribute__((__used__)) \
	__attribute__((__section__(".initcall" level ".init"))) = { .fn_minit = ifn, .fn_tinit = sfn };

#define MODULE_INITCALL(ifn, sfn)		__define_module_initcall("module", ifn, sfn)

#define MODULE_INITIALISE_ALL  { for (initmodule_t *fn = __module_initcall_start; fn < __module_initcall_end; fn++) \
									if (fn->fn_minit) \
										(fn->fn_minit)(); }

#define MODULE_TASKCREATE_ALL  { for (initmodule_t *fn = __module_initcall_start; fn < __module_initcall_end; fn++) \
									if (fn->fn_tinit) \
									   (fn->fn_tinit)(); }

#endif	/* PIOS_INITCALL_H */

/**
 * @}
 * @}
 */
