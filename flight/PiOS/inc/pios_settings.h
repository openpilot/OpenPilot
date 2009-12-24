/**
 ******************************************************************************
 *
 * @file       pios_settings.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2009.   
 * @brief      Settings functions header 
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

#ifndef PIOS_SETTINGS_H
#define PIOS_SETTINGS_H


/* Global types */
typedef struct {
	uint32_t Baudrate;
} GPSSettingsTypeDef;

typedef struct {
	uint32_t Baudrate;
} TelemSettingsTypeDef;

typedef struct {
	bool Enabled;
	uint32_t Baudrate;
} USARTSettingsTypeDef;

typedef struct {
	uint16_t PositionMax;
	uint16_t PositionMin;
} ServosSettingsTypeDef;

typedef struct {
	GPSSettingsTypeDef GPS;
	TelemSettingsTypeDef Telem;
	USARTSettingsTypeDef AuxUSART;
	ServosSettingsTypeDef Servos;
} SettingsTypeDef;

/*Global Variables */
extern SettingsTypeDef Settings;

/* Public Functions */
extern void PIOS_Settings_Load(void);
extern void PIOS_Settings_Dump(USART_TypeDef* USARTx);
extern int32_t PIOS_Settings_CheckForFiles(void);

#endif /* PIOS_SETTINGS_H */