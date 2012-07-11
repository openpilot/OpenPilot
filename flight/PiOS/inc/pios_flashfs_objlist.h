/**
 ******************************************************************************
 *
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_FLASHFS_OBJLIST Object list based flash filesystem (low ram)
 * @{
 *
 * @file       pios_flashfs_objlist.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      A file system for storing UAVObject in flash chip
 * @see        The GNU Public License (GPL) Version 3
 *
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

#include "openpilot.h"
#include "uavobjectmanager.h"

struct flashfs_cfg {
	uint32_t table_magic;
	uint32_t obj_magic;
	uint32_t obj_table_start;
	uint32_t obj_table_end;
	uint32_t sector_size;
	uint32_t chip_size;
};

int32_t PIOS_FLASHFS_Init(const struct flashfs_cfg * cfg);
int32_t PIOS_FLASHFS_Format();
int32_t PIOS_FLASHFS_ObjSave(UAVObjHandle obj, uint16_t instId, uint8_t * data);
int32_t PIOS_FLASHFS_ObjLoad(UAVObjHandle obj, uint16_t instId, uint8_t * data);
int32_t PIOS_FLASHFS_ObjDelete(UAVObjHandle obj, uint16_t instId);
