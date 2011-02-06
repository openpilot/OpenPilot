/**
 ******************************************************************************
 *
 * @file       saved_settings.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      RF Module hardware layer
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

#include <string.h>     // memmove, memset

#include "crc.h"
#include "gpio_in.h"
#include "saved_settings.h"
#include "main.h"

#if defined(PIOS_COM_DEBUG)
    #define SAVED_SETTINGS_DEBUG
#endif

// *****************************************************************

// default aes 128-bit encryption key
const uint8_t saved_settings_default_aes_key[16] = {0x65, 0x3b, 0x71, 0x89, 0x4a, 0xf4, 0xc8, 0xcb, 0x18, 0xd4, 0x9b, 0x4d, 0x4a, 0xbe, 0xc8, 0x37};

// *****************************************************************

#define pages   1                             // number of flash pages to use

uint32_t        eeprom_addr;                  // the address of the emulated EEPROM area in program flash area
uint16_t        eeprom_page_size;             // flash page size

volatile t_saved_settings       saved_settings __attribute__ ((aligned(4)));     // a RAM copy of the settings stored in EEPROM
t_saved_settings                tmp_settings __attribute__ ((aligned(4)));

// *****************************************************************
// Private functions

bool saved_settings_page_empty(int page)
{   // return TRUE if the flash page is emtpy (erased), otherwise return FALSE

    if (page < 0 || page >= pages)
        return FALSE;

    __IO uint32_t *addr = (__IO uint32_t *)(eeprom_addr + eeprom_page_size * page);
    int32_t len = eeprom_page_size / 4;

    for (int32_t i = len; i > 0; i--)
        if (*addr++ != 0xffffffff)
            return FALSE;       // the page is not erased

    return TRUE;
}

bool saved_settings_settings_empty(uint32_t addr)
{   // return TRUE if the settings flash area is emtpy (erased), otherwise return FALSE

    __IO uint8_t *p = (__IO uint8_t *)addr;

    for (int32_t i = sizeof(t_saved_settings); i > 0; i--)
        if (*p++ != 0xff)
            return FALSE;       // the flash area is not empty/erased

    return TRUE;
}

// *****************************************************************

int32_t saved_settings_read(void)
{   // look for the last valid settings saved in EEPROM

    uint32_t flash_addr;
    __IO uint8_t *p1;
    uint8_t *p2;

    flash_addr = eeprom_addr;

    if (saved_settings_settings_empty(flash_addr))
    {
        #if defined(SAVED_SETTINGS_DEBUG)
            DEBUG_PRINTF("settings Read, no settings found at %08X\r\n", flash_addr);
        #endif

        return -1;   // no settings found at the specified addr
    }

    // copy the data from program flash area into temp settings area
    p1 = (__IO uint8_t *)flash_addr;
    p2 = (uint8_t *)&tmp_settings;
    for (int32_t i = 0; i < sizeof(t_saved_settings); i++)
        *p2++ = *p1++;

    // calculate and check the CRC
    uint32_t crc1 = tmp_settings.crc;
    tmp_settings.crc = 0;
    uint32_t crc2 = updateCRC32Data(0xffffffff, (void *)&tmp_settings, sizeof(t_saved_settings));
    if (crc2 != crc1)
    {
        #if defined(SAVED_SETTINGS_DEBUG)
            DEBUG_PRINTF("settings Read crc error: %08X %08X\r\n", crc1, crc2);
        #endif

        return -2;      // error
    }

    memmove((void *)&saved_settings, (void *)&tmp_settings, sizeof(t_saved_settings));

    #if defined(SAVED_SETTINGS_DEBUG)
        DEBUG_PRINTF("settings Read OK!\r\n");
    #endif

    return 0;   // OK
}

// *****************************************************************
// Public functions

int32_t saved_settings_save(void)
{   // save the settings into EEPROM

    FLASH_Status fs;
    uint32_t flash_addr;
    uint8_t *p1;
    __IO uint8_t *p2;
    uint32_t *p3;
    bool do_save;

    // size of the settings aligned to 4 bytes
//  uint16_t settings_size = (uint16_t)(sizeof(t_saved_settings) + 3) & 0xfffc;

    // address of settings in FLASH area
    flash_addr = eeprom_addr;

    // *****************************************
    // calculate and add the CRC

    saved_settings.crc = 0;
    saved_settings.crc = updateCRC32Data(0xffffffff, (void *)&saved_settings, sizeof(t_saved_settings));

    // *****************************************
    // first check to see if we need to save the settings

    p1 = (uint8_t *)&saved_settings;
    p2 = (__IO uint8_t *)flash_addr;
    do_save = FALSE;

    for (int32_t i = 0; i < sizeof(t_saved_settings); i++)
    {
        if (*p1++ != *p2++)
        {   // we need to save the settings
            do_save = TRUE;
            break;
        }
    }

    if (!do_save)
    {
        #if defined(SAVED_SETTINGS_DEBUG)
            DEBUG_PRINTF("settings already saved OK\r\n");
        #endif

        return 0;   // settings already saved .. all OK
    }

    // *****************************************

    // Unlock the Flash Program Erase controller
    FLASH_Unlock();

    if (!saved_settings_page_empty(0))
    {   // erase the page
        #if defined(SAVED_SETTINGS_DEBUG)
            DEBUG_PRINTF("settings erasing page .. ");
        #endif

        fs = FLASH_ErasePage(eeprom_addr);
        if (fs != FLASH_COMPLETE)
        {   // error
            FLASH_Lock();

            #if defined(SAVED_SETTINGS_DEBUG)
                DEBUG_PRINTF("error %d\r\n", fs);
            #endif

            return -1;
        }

        #if defined(SAVED_SETTINGS_DEBUG)
            DEBUG_PRINTF("OK\r\n");
        #endif
    }
    else
    {
        #if defined(SAVED_SETTINGS_DEBUG)
            DEBUG_PRINTF("settings page already erased\r\n");
        #endif
    }

    // *****************************************
    // save the settings into flash area (emulated EEPROM area)

    p1 = (uint8_t *)&saved_settings;
    p3 = (uint32_t *)flash_addr;

    // write 4 bytes at a time into program flash area (emulated EEPROM area)
    for (int32_t i = 0; i < sizeof(t_saved_settings); p3++)
    {
        uint32_t value = 0;
        if (i < sizeof(t_saved_settings)) value |= (uint32_t)*p1++ << 0;  else value |= 0x000000ff; i++;
        if (i < sizeof(t_saved_settings)) value |= (uint32_t)*p1++ << 8;  else value |= 0x0000ff00; i++;
        if (i < sizeof(t_saved_settings)) value |= (uint32_t)*p1++ << 16; else value |= 0x00ff0000; i++;
        if (i < sizeof(t_saved_settings)) value |= (uint32_t)*p1++ << 24; else value |= 0xff000000; i++;

        fs = FLASH_ProgramWord((uint32_t)p3, value); // write a 32-bit value
        if (fs != FLASH_COMPLETE)
        {
            FLASH_Lock();

            #if defined(SAVED_SETTINGS_DEBUG)
                DEBUG_PRINTF("settings FLASH_ProgramWord error: %d\r\n", fs);
            #endif

            return -2;                                  // error
        }
    }

    // Lock the Flash Program Erase controller
    FLASH_Lock();

    // *****************************************
    // now error check it by reading it back (simple compare)

    p1 = (uint8_t *)&saved_settings;
    p2 = (__IO uint8_t *)flash_addr;

    for (int32_t i = 0; i < sizeof(t_saved_settings); i++)
    {
        if (*p1++ != *p2++)
        {
            #if defined(SAVED_SETTINGS_DEBUG)
                DEBUG_PRINTF("settings WriteSettings compare error\r\n");
            #endif

            return -3;                                  // error
        }
    }

    // *****************************************

    #if defined(SAVED_SETTINGS_DEBUG)
        DEBUG_PRINTF("settings Save OK!\r\n");
    #endif

    return 0;   // OK
}

void saved_settings_init(void)
{
    // **********
    // determine emulated EEPROM details

    if (flash_size < 256000)
        eeprom_page_size = 1024;  // 1 kByte
    else
        eeprom_page_size = 2048;  // 2 kByte

    // place emulated eeprom at end of program flash area
    eeprom_addr = (0x08000000 + flash_size) - (eeprom_page_size * pages);

    #if defined(SAVED_SETTINGS_DEBUG)
        DEBUG_PRINTF("\r\n");
        DEBUG_PRINTF("settings eeprom addr: %08x\r\n", eeprom_addr);
        DEBUG_PRINTF("settings eeprom page size: %u\r\n", eeprom_page_size);
        DEBUG_PRINTF("settings eeprom pages: %u\r\n", pages);
    #endif

    // **********
    // default settings

    memset((void *)&saved_settings, 0xff, sizeof(t_saved_settings));

   	saved_settings.mode = MODE_NORMAL;

    saved_settings.destination_id = 0;

    saved_settings.frequency_band = FREQBAND_UNKNOWN;

    saved_settings.rf_xtal_cap = 0x7f;

    saved_settings.aes_enable = FALSE;
    memmove((void *)&saved_settings.aes_key, saved_settings_default_aes_key, sizeof(saved_settings.aes_key));

    saved_settings.serial_baudrate = 57600;

//    saved_settings.crc = 0;
//    saved_settings.crc = updateCRC32Data(0xffffffff, (void *)&saved_settings, sizeof(t_saved_settings));

    // **********

    // Lock the Flash Program Erase controller
    FLASH_Lock();

    saved_settings_read();

    // **********
}

// *****************************************************************
