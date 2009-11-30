/**
 ******************************************************************************
 *
 * @file       pios_settings.c  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2009.   
 * @brief      Gets and sets PiOS settings, normally from SDCard. 
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   PIOS_SETTINGS Settings Functions
 * @{
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


/* Project Includes */
#include "pios.h"


/* Private Function Prototypes */


/* Local Variables */
SettingsTypeDef Settings;


/**
* Populate System global Settings into Structs using MinIni, defaults are also set here.
*/
/* Value Reading:	ini_getl("Section", "Key", (DefaultValue), IniFile);					*/
/* String Reading:	ini_gets("Section", "Key", "DefaultValue", StrBuffer, sizearray(StrBuffer), IniFile); 	*/
void LoadSettings(void)
{
	char StrBuffer[100];
	long Result;
	
	/* Section: GPS */
	Settings.GPS.Baudrate = 	(uint32_t) 	ini_getl("GPS", "Baudrate", GPS_BAUDRATE, SETTINGS_FILE);
	
	/* Section: Telemetry */
	Settings.Telem.Baudrate = 	(uint32_t) 	ini_getl("Telemetry", "Baudrate", TELEM_BAUDRATE, SETTINGS_FILE);
	
	/* Section: Auxillary_UART */
	Settings.AuxUART.Enabled = 	(bool) 		ini_getl("Auxillary_UART", "Enabled", AUXUART_ENABLED, SETTINGS_FILE);
	Settings.AuxUART.Baudrate = 	(uint32_t) 	ini_getl("Auxillary_UART", "Baudrate", AUXUART_BAUDRATE, SETTINGS_FILE);
}
