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

/*
 * yaffscfg2k.c  The configuration for the "direct" use of yaffs.
 *
 * This file is intended to be modified to your requirements.
 * There is no need to redistribute this file.
 */

#include "yaffscfg.h"
#include "yaffsfs.h"
#include "yaffs_trace.h"
#include "yramsim.h"

unsigned yaffs_trace_mask = 

	YAFFS_TRACE_SCAN |  
	YAFFS_TRACE_GC |
	YAFFS_TRACE_ERASE | 
	YAFFS_TRACE_ERROR | 
	YAFFS_TRACE_TRACING | 
	YAFFS_TRACE_ALLOCATE | 
	YAFFS_TRACE_BAD_BLOCKS |
	YAFFS_TRACE_VERIFY | 
	
	0;
        


// Configuration


int yaffs_start_up(void)
{
	// Stuff to configure YAFFS
	// Stuff to initialise anything special (eg lock semaphore).
	yaffsfs_OSInitialisation();
	yramsim_CreateRamSim("yaffs2",1,1000,0,0);
	yramsim_CreateRamSim("p0",0,0x400,1,0xff);
	yramsim_CreateRamSim("p1",0,0x400,1,0);
	
	return 0;
}



