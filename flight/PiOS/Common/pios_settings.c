/**
 ******************************************************************************
 *
 * @file       pios_settings.c  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
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

#if !defined(PIOS_DONT_USE_SETTINGS)


/* Private Function Prototypes */


/* Local Variables */
SettingsTypeDef Settings;


/**
* Populate System global Settings into Structs using MinIni, defaults are also set here via macros from pios_config.h
*/
/* Value Reading:	ini_getl("Section", "Key", (DefaultValue), IniFile);					*/
/* String Reading:	ini_gets("Section", "Key", "DefaultValue", StrBuffer, sizearray(StrBuffer), IniFile); 	*/
void PIOS_Settings_Load(void)
{
	/* Unused yet, until we load strings
	char StrBuffer[100];
	long Result;
	*/

	/* Section: GPS */
	Settings.GPS.Baudrate = 		(uint32_t) 	ini_getl("GPS", "Baudrate", GPS_BAUDRATE, SETTINGS_FILE);

	/* Section: Telemetry */
	Settings.Telem.Baudrate = 		(uint32_t) 	ini_getl("Telemetry", "Baudrate", TELEM_BAUDRATE, SETTINGS_FILE);

	/* Section: Auxillary_USART */
	Settings.AuxUSART.Enabled = 	(bool) 		ini_getl("Auxillary_USART", "Enabled", AUXUART_ENABLED, SETTINGS_FILE);
	Settings.AuxUSART.Baudrate = 	(uint32_t) 	ini_getl("Auxillary_USART", "Baudrate", AUXUART_BAUDRATE, SETTINGS_FILE);

	/* Section: Servos */
	Settings.Servos.PositionMin =	(uint16_t)	ini_getl("Servos", "PositionMin", SERVOS_POSITION_MIN, SETTINGS_FILE);
	Settings.Servos.PositionMax =	(uint16_t)	ini_getl("Servos", "PositionMax", SERVOS_POSITION_MAX, SETTINGS_FILE);
}

/**
* Dump Settings struct contents to UART
* \param[in] USARTx USART name (GPS, TELEM, AUX)
*/
void PIOS_Settings_Dump(USART_TypeDef* USARTx)
{
	/* Implement once UART is fully implemented */
}

/**
* Check if settings files exist on the drive
* \return 0 All files found
* \return >0 Number of files missing
*/
int32_t PIOS_Settings_CheckForFiles(void)
{
	DIRINFO di;
	DIRENT de;
	int FoundCount = 0;

	di.scratch = PIOS_SDCARD_Sector;

	/* Open the root directory */
	DFS_OpenDir(&PIOS_SDCARD_VolInfo, (uint8_t *)"", &di);

	/* Scan the directory for all files */
	while(!DFS_GetNext(&PIOS_SDCARD_VolInfo, &di, &de)) {
		if(de.name[0]) {
			uint8_t file_name[13];
			DFS_DirToCanonical((char *)file_name, (char *)de.name);

			if(strcmp((char *)file_name, SETTINGS_FILE) == 0) {
				FoundCount++;
			}
		}
	}

	/* If one or more files are missing, return the number of missing files */
	if(FoundCount != 1)
	{
		return FoundCount;
	}

	/* All files found */
	return 0;
}

#endif
