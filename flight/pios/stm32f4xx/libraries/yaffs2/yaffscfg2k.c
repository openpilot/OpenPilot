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
	YAFFS_TRACE_ERROR |
	YAFFS_TRACE_BUG			 |
	YAFFS_TRACE_ALWAYS | 
	0;

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

#define MAX_DEVICENAME_LEN 8
/**
 * @brief Initialize the flash object setting FS.  Each call creates a yaffs device
 * @return 0 if success, -1 if failure
 */
int32_t PIOS_FLASHFS_Logfs_Init(
	uintptr_t *fs_id, // return identifier for fs device
	const struct flashfs_logfs_cfg *cfg,   
	const struct pios_flash_driver *driver,
	uintptr_t flash_id)			
{
    int retval;
    pios_trace(PIOS_TRACE_TEST, "PIOS_FLASHFS_Logfs_Init");
    char devicename[MAX_DEVICENAME_LEN];
    char logfs_path[12];

    /* Make sure the underlying flash driver provides the minimal set of required methods */
    PIOS_Assert(driver->start_transaction);
    PIOS_Assert(driver->end_transaction);
    PIOS_Assert(driver->erase_sector);
    PIOS_Assert(driver->write_data);
    PIOS_Assert(driver->read_data);

    // Initialise the yaffs OS subsystem
    if (yaffs_start_up()) {
        PIOS_Assert(0);
    }

    // Allocate the next device id
    *fs_id = pios_flash_device_count;
    pios_flash_device_count++;

    snprintf(devicename,6, "/dev%01u", (unsigned) *fs_id);

    // stm32f4 implementation uses
    yaffs_nor_install_drv(devicename, MAX_DEVICENAME_LEN, cfg, driver, flash_id);

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
        if (retval < 0)
        {
            if (yaffsfs_GetLastError() == -EEXIST)
            {
                retval = 0;
            }
            else
            {
                pios_trace(PIOS_TRACE_ERROR, "Couldn't mkdir %s", logfs_path);
            }
        }
    }


    return retval;
}

