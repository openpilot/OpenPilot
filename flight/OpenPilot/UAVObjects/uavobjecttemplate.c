/**
 ******************************************************************************
 *
 * @file       uavobjecttemplate.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Template file for all objects. This file will not compile, it is used
 * 			   by the parser as a template to generate all other objects. All $(x) fields
 * 			   will be replaced by the parser with the actual object information.
 * 			   Each object has a meta object associated with it. The meta object
 *             contains information such as the telemetry and logging properties.
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

#define $(METAOBJECT)

#include <string.h> // for memcpy
#include "$(NAMELC).h"
#ifndef METAOBJECT
#include "$(NAMELC)meta.h"
#endif //METAOBJECT
#include "uavobjectlist.h"
#include "uavobjectutils.h"
#include "FreeRTOS.h"
#include "semphr.h"

// Private constants
#define OBJECT_ID $(OBJID)
#define METADATA_ID $(METAID)
#define NAME $(NAMESTR)

// Private variables
ObjectContext context;
$(NAME)Data data;
uint32_t objectID;
#ifdef METAOBJECT
UAVObjectMetadata metaData;
#endif //METAOBJECT

// Private functions
int32_t pack(uint32_t objId, uint8_t* data, int32_t maxLength);
int32_t unpack(uint32_t objId, uint8_t* data, int32_t length);
int32_t initializeData(const char* init);
void getMetadata(UAVObjectMetadata* dataOut);
void setMetadata(const UAVObjectMetadata* dataIn);
void requestUpdate();
void updated();
int32_t connect(xQueueHandle queue, int32_t ignoreUnpack);
int32_t disconnect(xQueueHandle queue);
int32_t getFieldIndexByName(char* name);

/**
 * Initialize object.
 * \return 0 Success
 * \return -1 Failure
 */
int32_t $(NAME)Initialize()
{
	// Create mutex
	context.mutex = xSemaphoreCreateRecursiveMutex();
	if (context.mutex == NULL)
		return -1;

	// Setup UAVObject table
	context.obj.objectID = OBJECT_ID;
	context.obj.metadataID = METADATA_ID;
#ifndef METAOBJECT
	context.obj.isMetadata = 0;
#else
	context.obj.isMetadata = 1;
#endif //METAOBJECT
	context.obj.name = NAME;
	context.obj.numBytes = sizeof($(NAME)Data);
	context.obj.pack = &pack;
	context.obj.unpack = &unpack;
	context.obj.initializeData = initializeData;
	context.obj.getMetadata = &getMetadata;
	context.obj.setMetadata = &setMetadata;
	context.obj.requestUpdate = &requestUpdate;
	context.obj.updated = &updated;
	context.obj.connect = &connect;
	context.obj.disconnect = &disconnect;

	// Initialise connected queue list
	context.queues = NULL;
	objectID = OBJECT_ID;

	// Initialise linked object
#ifndef METAOBJECT
	$(NAME)MetaInitialize();
	context.obj.linkedObj = $(NAME)MetaGet();
	context.obj.linkedObj->linkedObj = &context.obj;
#else
	context.obj.linkedObj = NULL;
#endif //METAOBJECT

	// Register object with object list
	UAVObjectListRegister(&context.obj);

	return 0;
}

/**
 * See UAVObject.h description for more details.
 */
int32_t pack(uint32_t objId, uint8_t* dataOut, int32_t maxLength)
{
	int32_t res;
	// Lock
	xSemaphoreTakeRecursive(context.mutex, portMAX_DELAY);
	// Pack data
	if (sizeof($(NAME)Data) <= maxLength)
	{
		memcpy(dataOut, &data, sizeof($(NAME)Data));
		res = sizeof($(NAME)Data);
	}
	else
	{
		res = -1;
	}
	// Unlock and return
	xSemaphoreGiveRecursive(context.mutex);
	return sizeof($(NAME)Data);
}

/**
 * See UAVObject.h description for more details.
 */
int32_t unpack(uint32_t objId, uint8_t* dataIn, int32_t length)
{
	// Lock
	xSemaphoreTakeRecursive(context.mutex, portMAX_DELAY);
	// Unpack data
	memcpy(&data, dataIn, sizeof($(NAME)Data));
	// Trigger event
	UAVObjUtilsSendEvent(&context, QMSG_UNPACKED);
	// Unlock and return
	xSemaphoreGiveRecursive(context.mutex);
	return 0;
}

/**
 * See UAVObject.h description for more details.
 */
int32_t initializeData(const char* init)
{
	return -1;
}

/**
 * See UAVObject.h description for more details.
 */
void getMetadata(UAVObjectMetadata* dataOut)
{
	// Get lock
	xSemaphoreTakeRecursive(context.mutex, portMAX_DELAY);

	// Get data
#ifndef METAOBJECT
	$(NAME)MetaGetData(dataOut);
#else
	memcpy(dataOut, &metaData, sizeof(UAVObjectMetadata));
#endif //METAOBJECT

	// Release lock
	xSemaphoreGiveRecursive(context.mutex);
}

/**
 * See UAVObject.h description for more details.
 */
void setMetadata(const UAVObjectMetadata* dataIn)
{
	// Get lock
	xSemaphoreTakeRecursive(context.mutex, portMAX_DELAY);

	// Get data
#ifndef METAOBJECT
	$(NAME)MetaSetData(dataIn);
#else
	memcpy(&metaData, dataIn, sizeof(UAVObjectMetadata));
#endif //METAOBJECT

	// Release lock
	xSemaphoreGiveRecursive(context.mutex);
}

/**
 * See UAVObject.h description for more details.
 */
void requestUpdate()
{
	UAVObjUtilsRequestUpdate(&context);
}

/**
 * See UAVObject.h description for more details.
 */
void updated()
{
	UAVObjUtilsUpdated(&context);
}

/**
 * See UAVObject.h description for more details.
 */
int32_t connect(xQueueHandle queue, int32_t eventMask)
{
	return UAVObjUtilsConnect(&context, queue, eventMask);
}

/**
 * See UAVObject.h description for more details.
 */
int32_t disconnect(xQueueHandle queue)
{
	return UAVObjUtilsDisconnect(&context, queue);
}

/**
 * Get object's data structure.
 * \param[out] dataOut Data structure to copy data into
 */
void $(NAME)GetData($(NAME)Data* dataOut)
{
	// Get lock
	xSemaphoreTakeRecursive(context.mutex, portMAX_DELAY);
	// Copy data
	memcpy(dataOut, &data, sizeof($(NAME)Data));
	// Release lock
	xSemaphoreGiveRecursive(context.mutex);
}

/**
 * Set object's data structure.
 * \param[in] dataIn Data structure to copy data from.
 */
void $(NAME)SetData(const $(NAME)Data* dataIn)
{
	// Get lock
	xSemaphoreTakeRecursive(context.mutex, portMAX_DELAY);
	// Copy data
	memcpy(&data, dataIn, sizeof($(NAME)Data));
	// Send notification for updates
	UAVObjUtilsSendEvent(&context, QMSG_UPDATED);
	// Release lock
	xSemaphoreGiveRecursive(context.mutex);
}




