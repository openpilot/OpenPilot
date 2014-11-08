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


#ifndef __YAFFS_NOR_DRV_H__
#define __YAFFS_NOR_DRV_H__

#include <stdint.h>

struct yaffs_dev;
struct flashfs_logfs_cfg;
struct pios_flash_driver;

struct pios_yaffs_driver_context
{
const struct flashfs_logfs_cfg *cfg;
const struct pios_flash_driver *driver;
uintptr_t flash_id;
};


void yaffs_nor_install_drv(const char *name,
                           uint16_t max_name_len,
                           const struct flashfs_logfs_cfg *cfg,
                           const struct pios_flash_driver *driver,
                           uintptr_t flash_id);

#endif


