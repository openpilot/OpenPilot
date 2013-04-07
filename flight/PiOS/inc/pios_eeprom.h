/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_EEPROM EEPROM reading/writing functions
 * @brief PIOS EEPROM reading/writing functions
 * @{
 *
 * @file       pios_eeprom.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      COM layer functions
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

#ifndef PIOS_EEPROM_H
#define PIOS_EEPROM_H

/* Public Structures */
struct pios_eeprom_cfg {
	uint32_t base_address;
	uint32_t max_size;
};

/* Public Functions */
extern void PIOS_EEPROM_Init(const struct pios_eeprom_cfg *cfg);
extern int32_t PIOS_EEPROM_Save(uint8_t *data, uint32_t len);
extern int32_t PIOS_EEPROM_Load(uint8_t *data, uint32_t len);

#endif /* PIOS_EEPROM_H */

/**
  * @}
  * @}
  */
