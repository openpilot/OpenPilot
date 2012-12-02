/**
 ******************************************************************************
 * @file       pios_flashfs.h
 * @author     PhoenixPilot, http://github.com/PhoenixPilot, Copyright (C) 2012
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_FLASHFS Flash Filesystem API Definition
 * @{
 * @brief Flash Filesystem API Definition
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

#ifndef PIOS_FLASHFS_H_
#define PIOS_FLASHFS_H_

#include <stdint.h>
#include "uavobjectmanager.h"	/* UAVObjHandle */

int32_t PIOS_FLASHFS_Format();
int32_t PIOS_FLASHFS_ObjSave(UAVObjHandle obj, uint16_t instId, uint8_t * data);
int32_t PIOS_FLASHFS_ObjLoad(UAVObjHandle obj, uint16_t instId, uint8_t * data);
int32_t PIOS_FLASHFS_ObjDelete(UAVObjHandle obj, uint16_t instId);

#endif	/* PIOS_FLASHFS_H_ */
