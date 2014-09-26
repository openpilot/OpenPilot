/*
 * YAFFS: Yet Another Flash File System. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2011 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "yaffs_attribs.h"


void yaffs_load_attribs(struct yaffs_obj *obj, struct yaffs_obj_hdr *oh)
{
#ifdef CONFIG_YAFFS_WINCE
	obj->win_atime[0] = oh->win_atime[0];
	obj->win_ctime[0] = oh->win_ctime[0];
	obj->win_mtime[0] = oh->win_mtime[0];
	obj->win_atime[1] = oh->win_atime[1];
	obj->win_ctime[1] = oh->win_ctime[1];
	obj->win_mtime[1] = oh->win_mtime[1];
#else
	obj->yst_uid = oh->yst_uid;
	obj->yst_gid = oh->yst_gid;
	obj->yst_atime = oh->yst_atime;
	obj->yst_mtime = oh->yst_mtime;
	obj->yst_ctime = oh->yst_ctime;
	obj->yst_rdev = oh->yst_rdev;
#endif
}


void yaffs_load_attribs_oh(struct yaffs_obj_hdr *oh, struct yaffs_obj *obj)
{
#ifdef CONFIG_YAFFS_WINCE
	oh->win_atime[0] = obj->win_atime[0];
	oh->win_ctime[0] = obj->win_ctime[0];
	oh->win_mtime[0] = obj->win_mtime[0];
	oh->win_atime[1] = obj->win_atime[1];
	oh->win_ctime[1] = obj->win_ctime[1];
	oh->win_mtime[1] = obj->win_mtime[1];
#else
	oh->yst_uid = obj->yst_uid;
	oh->yst_gid = obj->yst_gid;
	oh->yst_atime = obj->yst_atime;
	oh->yst_mtime = obj->yst_mtime;
	oh->yst_ctime = obj->yst_ctime;
	oh->yst_rdev = obj->yst_rdev;
#endif
}

void yaffs_attribs_init(struct yaffs_obj *obj, u32 gid, u32 uid, u32 rdev)
{

#ifdef CONFIG_YAFFS_WINCE
	yfsd_win_file_time_now(obj->win_atime);
	obj->win_ctime[0] = obj->win_mtime[0] = obj->win_atime[0];
	obj->win_ctime[1] = obj->win_mtime[1] = obj->win_atime[1];

#else
	yaffs_load_current_time(obj, 1, 1);
	obj->yst_rdev = rdev;
	obj->yst_uid = uid;
	obj->yst_gid = gid;
#endif
}

void yaffs_load_current_time(struct yaffs_obj *obj, int do_a, int do_c)
{
#ifdef CONFIG_YAFFS_WINCE
	yfsd_win_file_time_now(obj->win_atime);
	obj->win_ctime[0] = obj->win_mtime[0] =
	    obj->win_atime[0];
	obj->win_ctime[1] = obj->win_mtime[1] =
	    obj->win_atime[1];

#else

	obj->yst_mtime = Y_CURRENT_TIME;
	if (do_a)
		obj->yst_atime = obj->yst_atime;
	if (do_c)
		obj->yst_ctime = obj->yst_atime;
#endif
}

#ifndef CONFIG_YAFFS_OP  // unused yaffs code
static Y_LOFF_T yaffs_get_file_size(struct yaffs_obj *obj)
{
	YCHAR *alias = NULL;
	obj = yaffs_get_equivalent_obj(obj);

	switch (obj->variant_type) {
	case YAFFS_OBJECT_TYPE_FILE:
		return obj->variant.file_variant.file_size;
	case YAFFS_OBJECT_TYPE_SYMLINK:
		alias = obj->variant.symlink_variant.alias;
		if (!alias)
			return 0;
		return yaffs_strnlen(alias, YAFFS_MAX_ALIAS_LENGTH);
	default:
		return 0;
	}
}
#endif

