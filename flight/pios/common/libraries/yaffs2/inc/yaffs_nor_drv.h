/**
 ******************************************************************************
 * @addtogroup Yaffs lower layer
 * @{
 * @addtogroup   Yaffs lower layer
 * @brief Interface yaffs to PIOS flash driver or simposix nor simulator
 * @{
 *
 * @file       yaffs_nor_drv.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      yaffs flash file system
 * @see        The GNU Public License (GPL) Version 3
 * @notes
 *
 *****************************************************************************/

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


