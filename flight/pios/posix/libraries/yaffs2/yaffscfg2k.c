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
#include <signal.h>


#include "pios.h"
#include "pios_trace.h"
#include "pios_stdio.h"

#include <openpilot.h>
#include "pios_flashfs.h" /* API for flash filesystem */
#include "pios_flashfs_logfs_priv.h"

#include <pios_stdio.h>


unsigned yaffs_trace_mask =
	YAFFS_TRACE_ERROR |
	YAFFS_TRACE_BUG			 |
	YAFFS_TRACE_ALWAYS |
	YAFFS_TRACE_MTD |
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



void yaffsSigHandler ( int sig)
{
    char devicename[8];
    int fs_id;

	    pios_trace(PIOS_TRACE_TEST, "yaffsSigHandler sig=%d", sig);
        switch (sig)
        {
          case SIGQUIT:
          case SIGTERM:
          case SIGKILL:
          case SIGINT:

			   for (fs_id =0; fs_id < pios_flash_device_count; fs_id++)
			   {
			     snprintf(devicename,6, "/dev%01u", (unsigned) fs_id);

			     pios_umount((const char *)devicename);

			   }
			   pios_flash_device_count=0;
			   exit(1);
               break;
          default:
		break;

        }
}

static void yaffsSigSetup
(
void (*sighandler)(int sig)
)
{
    //sigset_t block_sigusr;
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = sighandler;
    if (sigaction(SIGQUIT, &sa, NULL))  return;


    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = sighandler;
    if (sigaction(SIGINT , &sa, NULL))  return;


    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = sighandler;
    if (sigaction(SIGTERM, &sa, NULL))  return;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = sighandler;
    if (sigaction(SIGKILL, &sa, NULL))  return;

    return;
}




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
	yaffs_nor_install_drv(devicename, cfg, driver, flash_id);


	sigset_t sigset;
	sigemptyset(&sigset);
	yaffsSigSetup(yaffsSigHandler);

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

