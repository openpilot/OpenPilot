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

struct nor_sim;

void ynorsim_rd32(struct nor_sim *sim, u32 *addr, u32 *data, int nwords);
void ynorsim_wr32(struct nor_sim *sim, u32 *addr, u32 *data, int nwords);
void ynorsim_erase(struct nor_sim *sim, u32 *addr);
void ynorsim_shutdown(struct nor_sim *sim);
struct nor_sim *ynorsim_initialise(char *name, int n_blocks, int block_size_bytes);
u32 * ynorsim_get_base(struct nor_sim *sim);

#endif
