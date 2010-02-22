/**
 ******************************************************************************
 *
 * @file       uavobjectlist.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Object list library. This library holds a collection of all objects.
 * 			   It can be used by all modules/libraries to find an object reference.
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

#include <stdlib.h> // for malloc
#include "string.h" // for strcmp
#include "uavobjectlist.h"
#include "utlist.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

// Private types
struct ObjectListStruct {
	UAVObject* obj;
    struct ObjectListStruct* next;
};
typedef struct ObjectListStruct ObjectList;

// Private variables
ObjectList* objList;
xSemaphoreHandle mutex;

/**
 * Initialize the object list
 * \return 0 Success
 * \return -1 Failure
 */
int32_t UAVObjectListInitialize()
{
	// Initialize object list
	objList = NULL;
	// Create mutex
	mutex = xSemaphoreCreateRecursiveMutex();
	if (mutex == NULL)
		return -1;
	// Done
	return 0;
}

/**
 * Register an object to the list
 * \param[in] obj Object to register
 * \return 0 Success
 * \return -1 Failure
 */
int32_t UAVObjectListRegister(UAVObject* obj)
{
	ObjectList* objEntry;

	// Get lock
	xSemaphoreTakeRecursive(mutex, portMAX_DELAY);

	// Check that the object is not already registred
	LL_FOREACH(objList, objEntry)
	{
		if (objList->obj == obj)
		{
			// Already registered, ignore
			xSemaphoreGiveRecursive(mutex);
			return -1;
		}
	}

	// Create and append entry
	objEntry = (ObjectList*)malloc(sizeof(ObjectList));
	objEntry->obj = obj;
	LL_APPEND(objList, objEntry);

	// Release lock
	xSemaphoreGiveRecursive(mutex);
	return 0;
}

/**
 * Retrieve an object from the list given its id
 * \param[in] The object ID
 * \return The object or NULL if not found.
 */
UAVObject* UAVObjectListGetByID(int32_t id)
{
	ObjectList* objEntry;

	// Get lock
	xSemaphoreTakeRecursive(mutex, portMAX_DELAY);

	// Look for object
	LL_FOREACH(objList, objEntry)
	{
		if (objEntry->obj->objectID == id)
		{
			// Release lock
			xSemaphoreGiveRecursive(mutex);
			// Done, object found
			return objEntry->obj;
		}
	}

	// Object not found, release lock and return error
	xSemaphoreGiveRecursive(mutex);
	return NULL;
}

/**
 * Retrieve an object from the list given its name
 * \param[in] name The name of the object
 * \return 0 Success
 * \return -1 Failure
 */
UAVObject* UAVObjectListGetByName(char* name)
{
	ObjectList* objEntry;

	// Get lock
	xSemaphoreTakeRecursive(mutex, portMAX_DELAY);

	// Look for object
	LL_FOREACH(objList, objEntry)
	{
		if (strcmp(objEntry->obj->name, name) == 0)
		{
			// Release lock
			xSemaphoreGiveRecursive(mutex);
			// Done, object found
			return objEntry->obj;
		}
	}

	// Object not found, release lock and return error
	xSemaphoreGiveRecursive(mutex);
	return NULL;
}

/**
 * Iterate through all objects in the list.
 * \param iterator This function will be called once for each object,
 * the object will be passed as a parameter
 */
void UAVObjectsIterate(void (*iterator)(UAVObject* obj))
{
	ObjectList* objEntry;

	// Get lock
	xSemaphoreTakeRecursive(mutex, portMAX_DELAY);

	// Iterate through the list and invoke iterator for each object
	LL_FOREACH(objList, objEntry)
	{
		(*iterator)(objEntry->obj);
	}

	// Release lock
	xSemaphoreGiveRecursive(mutex);
}
