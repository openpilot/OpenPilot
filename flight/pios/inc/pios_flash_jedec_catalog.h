/**
 ******************************************************************************
 *
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_FLASH JEDEC devices catalog
 * @{
 *
 * @file       pios_flash_jedec_catalog.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @brief      Driver for talking to most JEDEC flash chips
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


#ifndef PIOS_FLASH_JEDEC_CATALOG_H_
#define PIOS_FLASH_JEDEC_CATALOG_H_
#include <pios_flash_jedec_priv.h>

// this structure contains the catalog of all "known" flash chip used
const struct pios_flash_jedec_cfg pios_flash_jedec_catalog [] =
{
		{ // m25p16
			.expect_manufacturer = JEDEC_MANUFACTURER_ST,
			.expect_memorytype   = 0x20,
			.expect_capacity     = 0x15,
			.sector_erase        = 0xD8,
			.chip_erase          = 0xC7,
		},
		{ // m25px16
			.expect_manufacturer = JEDEC_MANUFACTURER_ST,
			.expect_memorytype   = 0x71,
			.expect_capacity     = 0x15,
			.sector_erase        = 0xD8,
			.chip_erase          = 0xC7,
		},
		{ //w25x
			.expect_manufacturer = JEDEC_MANUFACTURER_WINBOND,
			.expect_memorytype   = 0x30,
			.expect_capacity     = 0x13,
			.sector_erase        = 0x20,
			.chip_erase          = 0x60
		},
		{ //25q16
			.expect_manufacturer = JEDEC_MANUFACTURER_WINBOND,
			.expect_memorytype   = 0x40,
			.expect_capacity     = 0x15,
			.sector_erase        = 0x20,
			.chip_erase          = 0x60
		}
};
const uint32_t pios_flash_jedec_catalog_size = NELEMENTS(pios_flash_jedec_catalog);


#endif /* PIOS_FLASH_JEDEC_CATALOG_H_ */
