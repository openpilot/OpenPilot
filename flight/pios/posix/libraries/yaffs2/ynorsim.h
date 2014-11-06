/*
 * YAFFS: Yet another Flash File System . A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2011 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 * Note: Only YAFFS headers are LGPL, YAFFS C code is covered by GPL.
 */

#ifndef __Y_NORSIM_H__
#define __Y_NORSIM_H__

#include "yaffs_guts.h"
#include "pios_flash.h"



void ynorsim_shutdown(uintptr_t flash_id);
void ynorsim_initialise(char *name,
                                   uintptr_t flash_id,
                                   int n_blocks,
				   int block_size_bytes);
extern const struct pios_flash_driver pios_norsim_flash_driver;

#endif
