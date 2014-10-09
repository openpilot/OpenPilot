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
#include "yaffs_guts.h"
#include "yaffsfs.h"
#include "yaffs_trace.h"
#include "yaffs_osglue.h"


#include <errno.h>

unsigned yaffs_trace_mask =
	YAFFS_TRACE_OS |
	YAFFS_TRACE_ALLOCATE |
	YAFFS_TRACE_SCAN |
	YAFFS_TRACE_BAD_BLOCKS |
	YAFFS_TRACE_ERASE |
	YAFFS_TRACE_GC	 |
	YAFFS_TRACE_WRITE |
	YAFFS_TRACE_TRACING |
	YAFFS_TRACE_DELETION |
	YAFFS_TRACE_BUFFERS |
	YAFFS_TRACE_NANDACCESS |
	YAFFS_TRACE_GC_DETAIL |
	YAFFS_TRACE_SCAN_DEBUG |
	YAFFS_TRACE_MTD	 |
	YAFFS_TRACE_CHECKPOINT	 |
	YAFFS_TRACE_VERIFY	 |
	YAFFS_TRACE_VERIFY_NAND	 |
	YAFFS_TRACE_VERIFY_FULL	 |
	YAFFS_TRACE_VERIFY_ALL	 |
	YAFFS_TRACE_SYNC |
	YAFFS_TRACE_BACKGROUND	 |
	YAFFS_TRACE_LOCK |
	YAFFS_TRACE_MOUNT |
	YAFFS_TRACE_ERROR |
	YAFFS_TRACE_BUG			 |
	YAFFS_TRACE_ALWAYS | 
	0;



int random_seed;
int simulate_power_failure = 0;
static unsigned int pios_flash_device_count=0;


/* Configure the devices that will be used */

#include "yaffs_nor_drv.h"

// called first before device device driver setup so need to change
int yaffs_start_up(void)
{
	static int start_up_called = 0;

	if(start_up_called)
		return 0;
	start_up_called = 1;

	/* Call the OS initialisation (eg. set up lock semaphore */
	yaffsfs_OSInitialisation();

	return 0;
}
#include "pios.h"
#include "pios_trace.h"
#include "pios_stdio.h"

#include <openpilot.h>
#include "pios_flashfs.h" /* API for flash filesystem */
#include "pios_flashfs_logfs_priv.h"

/**
 * @brief Initialize the flash object setting FS.  Each call creates a yaffs device
 * @return 0 if success, -1 if failure
 */
int32_t PIOS_FLASHFS_Logfs_Init(
	__attribute__((unused)) uintptr_t *fs_id, // return identifier for fs device
	__attribute__((unused)) const struct flashfs_logfs_cfg *cfg,     //optional - if flash
	__attribute__((unused)) const struct pios_flash_driver *driver,  //optional - if flash
	__attribute__((unused)) uintptr_t flash_id)						 //optional - if flash
{
    int retval;
    pios_trace(PIOS_TRACE_TEST, "PIOS_FLASHFS_Logfs_Init");
    char devicename[8];
    char logfs_path[12];

    // Initialise the yaffs OS subsystem
    if (yaffs_start_up()) {
        PIOS_Assert(0);
    }

    // Allocate the next device id
    *fs_id = pios_flash_device_count;
    pios_flash_device_count++;

    snprintf(devicename,6, "/dev%01u", (unsigned) *fs_id);

	// Simposix implementation uses a ram nor simulation which can be installed
    // as multiple instances
	yaffs_nor_install_drv(devicename);

	// Attempt to mount the device
    retval = pios_mount(devicename);
    if (retval < 0) {
        pios_trace(PIOS_TRACE_ERROR, "Couldn't mount %s", devicename);
    }
    else {

    	//Create a "logfs" directory on each yaffs device for use by the
    	// pios_logfs API.
    	snprintf(logfs_path, 12, "%s/%s", devicename, PIOS_LOGFS_DIR);

        // Create the logfs directory if it does not already exist
    	retval = pios_mkdir(logfs_path, O_CREAT);
        if (retval < 0) pios_trace(PIOS_TRACE_ERROR, "Couldn't mkdir %s", logfs_path);
    }


    return retval;
}

