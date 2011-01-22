/**
 * @file       plat.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      PyMite platform definitions for OpenPilot
 *
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

#include "pm.h"
#include "openpilot.h"

int pylinenum;

PmReturn_t plat_init(void)
{
    return PM_RET_OK;
}

PmReturn_t plat_deinit(void)
{
    return PM_RET_OK;
}

/*
 * Gets a byte from the address in the designated memory space
 * Post-increments *paddr.
 */
uint8_t plat_memGetByte(PmMemSpace_t memspace, uint8_t const **paddr)
{
    uint8_t b = 0;

    switch (memspace)
    {
        case MEMSPACE_RAM:
        case MEMSPACE_PROG:
            b = **paddr;
            *paddr += 1;
            return b;
        case MEMSPACE_EEPROM:
        case MEMSPACE_SEEPROM:
        case MEMSPACE_OTHER0:
        case MEMSPACE_OTHER1:
        case MEMSPACE_OTHER2:
        case MEMSPACE_OTHER3:
        default:
            return 0;
    }
}

PmReturn_t plat_getByte(uint8_t *b)
{
	return PM_RET_ERR;
}

PmReturn_t plat_putByte(uint8_t b)
{
    return PM_RET_ERR;
}

PmReturn_t plat_getMsTicks(uint32_t *r_ticks)
{
    *r_ticks = xTaskGetTickCount() * portTICK_RATE_MS;
    return PM_RET_OK;
}

void plat_reportError(PmReturn_t result)
{
    /* TODO: Copy error information to UAVObject */
	/* gVmGlobal.errVmRelease gVmGlobal.errFileId gVmGlobal.errLineNum */
}
