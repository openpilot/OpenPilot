/**
 ******************************************************************************
 *
 * @file       pios_flash_internal.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @brief      brief goes here.
 *             --
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

#include "pios.h"

#ifdef PIOS_INCLUDE_FLASH_INTERNAL

#include "stm32f10x_flash.h"
#include "pios_flash_internal_priv.h"
#include "pios_flash.h"
#include <stdbool.h>

struct device_flash_sector {
    uint32_t start;
    uint32_t size;
    uint16_t st_sector;
};

static bool PIOS_Flash_Internal_GetSectorInfo(uint32_t address, uint8_t *sector_number, uint32_t *sector_start, uint32_t *sector_size)
{
    uint16_t sector = (address - 0x08000000) / 1024;

    if (sector <= 127) {
        /* address lies within this sector */
        *sector_number = sector;
        *sector_start  = sector * 1024 + 0x08000000;
        *sector_size   = 1024;
        return true;
    }

    return false;
}

enum pios_internal_flash_dev_magic {
    PIOS_INTERNAL_FLASH_DEV_MAGIC = 0x33445902,
};

struct pios_internal_flash_dev {
    enum pios_internal_flash_dev_magic magic;

#if defined(PIOS_INCLUDE_FREERTOS)
    xSemaphoreHandle transaction_lock;
#endif /* defined(PIOS_INCLUDE_FREERTOS) */
};

static bool PIOS_Flash_Internal_Validate(struct pios_internal_flash_dev *flash_dev)
{
    return flash_dev && (flash_dev->magic == PIOS_INTERNAL_FLASH_DEV_MAGIC);
}

#if defined(PIOS_INCLUDE_FREERTOS)
static struct pios_internal_flash_dev *PIOS_Flash_Internal_alloc(void)
{
    struct pios_internal_flash_dev *flash_dev;

    flash_dev = (struct pios_internal_flash_dev *)pvPortMalloc(sizeof(*flash_dev));
    if (!flash_dev) {
        return NULL;
    }

    flash_dev->magic = PIOS_INTERNAL_FLASH_DEV_MAGIC;

    return flash_dev;
}
#else
static struct pios_internal_flash_dev pios_internal_flash_devs[PIOS_INTERNAL_FLASH_MAX_DEVS];
static uint8_t pios_internal_flash_num_devs;
static struct pios_internal_flash_dev *PIOS_Flash_Internal_alloc(void)
{
    struct pios_internal_flash_dev *flash_dev;

    if (pios_internal_flash_num_devs >= PIOS_INTERNAL_FLASH_MAX_DEVS) {
        return NULL;
    }

    flash_dev = &pios_internal_flash_devs[pios_internal_flash_num_devs++];
    flash_dev->magic = PIOS_INTERNAL_FLASH_DEV_MAGIC;

    return flash_dev;
}

#endif /* defined(PIOS_INCLUDE_FREERTOS) */

int32_t PIOS_Flash_Internal_Init(uintptr_t *flash_id, __attribute__((unused)) const struct pios_flash_internal_cfg *cfg)
{
    struct pios_internal_flash_dev *flash_dev;

    flash_dev = PIOS_Flash_Internal_alloc();
    if (flash_dev == NULL) {
        return -1;
    }

#if defined(PIOS_INCLUDE_FREERTOS)
    flash_dev->transaction_lock = xSemaphoreCreateMutex();
#endif /* defined(PIOS_INCLUDE_FREERTOS) */

    *flash_id = (uintptr_t)flash_dev;

    return 0;
}

/**********************************
 *
 * Provide a PIOS flash driver API
 *
 *********************************/
#include "pios_flash.h"

static int32_t PIOS_Flash_Internal_StartTransaction(uintptr_t flash_id)
{
    struct pios_internal_flash_dev *flash_dev = (struct pios_internal_flash_dev *)flash_id;

    if (!PIOS_Flash_Internal_Validate(flash_dev)) {
        return -1;
    }

#if defined(PIOS_INCLUDE_FREERTOS)
    if (xSemaphoreTake(flash_dev->transaction_lock, portMAX_DELAY) != pdTRUE) {
        return -2;
    }
#endif /* defined(PIOS_INCLUDE_FREERTOS) */

    /* Unlock the internal flash so we can write to it */
    FLASH_Unlock();
    return 0;
}

static int32_t PIOS_Flash_Internal_EndTransaction(uintptr_t flash_id)
{
    struct pios_internal_flash_dev *flash_dev = (struct pios_internal_flash_dev *)flash_id;

    if (!PIOS_Flash_Internal_Validate(flash_dev)) {
        return -1;
    }

#if defined(PIOS_INCLUDE_FREERTOS)
    if (xSemaphoreGive(flash_dev->transaction_lock) != pdTRUE) {
        return -2;
    }
#endif /* defined(PIOS_INCLUDE_FREERTOS) */

    /* Lock the internal flash again so we can no longer write to it */
    FLASH_Lock();

    return 0;
}

static int32_t PIOS_Flash_Internal_EraseSector(uintptr_t flash_id, uint32_t addr)
{
    struct pios_internal_flash_dev *flash_dev = (struct pios_internal_flash_dev *)flash_id;

    if (!PIOS_Flash_Internal_Validate(flash_dev)) {
        return -1;
    }

    uint8_t sector_number;
    uint32_t sector_start;
    uint32_t sector_size;
    if (!PIOS_Flash_Internal_GetSectorInfo(addr,
                                           &sector_number,
                                           &sector_start,
                                           &sector_size)) {
        /* We're asking for an invalid flash address */
        return -2;
    }

    if (FLASH_ErasePage(sector_start) != FLASH_COMPLETE) {
        return -3;
    }

    return 0;
}

static int32_t PIOS_Flash_Internal_WriteData(uintptr_t flash_id, uint32_t addr, uint8_t *data, uint16_t len)
{
    PIOS_Assert(data);

    struct pios_internal_flash_dev *flash_dev = (struct pios_internal_flash_dev *)flash_id;

    if (!PIOS_Flash_Internal_Validate(flash_dev)) {
        return -1;
    }

    uint8_t sector_number;
    uint32_t sector_start;
    uint32_t sector_size;
    uint32_t hword_data;
    uint32_t offset;

    /* Ensure that the base address is in a valid sector */
    if (!PIOS_Flash_Internal_GetSectorInfo(addr,
                                           &sector_number,
                                           &sector_start,
                                           &sector_size)) {
        /* We're asking for an invalid flash address */
        return -2;
    }

    /* Ensure that the entire write occurs within the same sector */
    if ((uintptr_t)addr + len > sector_start + sector_size) {
        /* Write crosses the end of the sector */
        return -3;
    }

    /* Write the data */
    uint32_t temp_addr = addr;
    uint16_t numberOfhWords = len / 2;
    uint16_t x = 0;
    FLASH_Status status;
    for (x = 0; x < numberOfhWords; ++x) {
        offset     = 2 * x;
        hword_data = (data[offset + 1] << 8) | data[offset];

        if (hword_data != *(uint16_t *)(temp_addr + offset)) {
            status = FLASH_ProgramHalfWord(temp_addr + offset, hword_data);
        } else {
            status = FLASH_COMPLETE;
        }
        PIOS_Assert(status == FLASH_COMPLETE);
    }

    uint16_t mod = len % 2;
    if (mod == 1) {
        offset     = 2 * x;
        hword_data = 0xFF00 | data[offset];
        if (hword_data != *(uint16_t *)(temp_addr + offset)) {
            status = FLASH_ProgramHalfWord(temp_addr + offset, hword_data);
        } else {
            status = FLASH_COMPLETE;
        }
        PIOS_Assert(status == FLASH_COMPLETE);
    }

    return 0;
}

static int32_t PIOS_Flash_Internal_ReadData(uintptr_t flash_id, uint32_t addr, uint8_t *data, uint16_t len)
{
    PIOS_Assert(data);

    struct pios_internal_flash_dev *flash_dev = (struct pios_internal_flash_dev *)flash_id;

    if (!PIOS_Flash_Internal_Validate(flash_dev)) {
        return -1;
    }

    uint8_t sector_number;
    uint32_t sector_start;
    uint32_t sector_size;

    /* Ensure that the base address is in a valid sector */
    if (!PIOS_Flash_Internal_GetSectorInfo(addr,
                                           &sector_number,
                                           &sector_start,
                                           &sector_size)) {
        /* We're asking for an invalid flash address */
        return -2;
    }

    /* Ensure that the entire read occurs within the same sector */
    if ((uintptr_t)addr + len > sector_start + sector_size) {
        /* Read crosses the end of the sector */
        return -3;
    }

    /* Read the data into the buffer directly */
    memcpy(data, (void *)addr, len);

    return 0;
}

/* Provide a flash driver to external drivers */
const struct pios_flash_driver pios_internal_flash_driver = {
    .start_transaction = PIOS_Flash_Internal_StartTransaction,
    .end_transaction   = PIOS_Flash_Internal_EndTransaction,
    .erase_sector = PIOS_Flash_Internal_EraseSector,
    .write_data   = PIOS_Flash_Internal_WriteData,
    .read_data    = PIOS_Flash_Internal_ReadData,
};

#endif /* PIOS_INCLUDE_FLASH_INTERNAL */
