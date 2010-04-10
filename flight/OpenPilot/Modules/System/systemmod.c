/**
 ******************************************************************************
 *
 * @file       systemmod.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      System module
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

#include "systemmod.h"
#include "settingspersistence.h"

// Private constants

// Private types

// Private variables

// Private functions
static void ObjectUpdatedCb(UAVObjEvent* ev);

/**
 * Initialise the module, called on startup.
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t SystemModInitialize(void)
{
	// Listen for ExampleObject1 updates, connect a callback function
	SettingsPersistenceConnectCallback(&ObjectUpdatedCb);

	return 0;
}

/**
 * Function called in response to object updates
 */
static void ObjectUpdatedCb(UAVObjEvent* ev)
{
	SettingsPersistenceData setper;

	// If the object updated was the SettingsPersistence execute requested action
	if ( ev->obj == SettingsPersistenceHandle() )
	{
		// Get object data
		SettingsPersistenceGet(&setper);

		// Execute action
		if ( setper.Operation == SETTINGSPERSISTENCE_OPERATION_LOAD)
		{
			UAVObjLoadSettings();
		}
		else if ( setper.Operation == SETTINGSPERSISTENCE_OPERATION_SAVE)
		{
			UAVObjSaveSettings();
		}
	}
}


