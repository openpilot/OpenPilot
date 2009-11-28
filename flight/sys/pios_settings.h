
/**
 * Project: OpenPilot
 *    
 * @author The OpenPilot Team, http://www.openpilot.org, Copyright (C) 2009.
 *    
 * @see The GNU Public License (GPL)
 */
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

/* Global Defines */

/* Global types */
typedef struct {
	bool Enabled;
	uint32_t Baud;
} UARTSettingsTypeDef;

typedef struct {
	UARTSettingsTypeDef GPSUART;
	UARTSettingsTypeDef TELEMUART;
	UARTSettingsTypeDef AUXUART;
} SettingsTypeDef;

/*Global Veriables */
extern SettingsTypeDef Settings;

/* Function Prototypes */

#endif /* PIOS_SETTINGS_H */