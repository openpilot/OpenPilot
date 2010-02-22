/**
 ******************************************************************************
 *
 * @file       uavobjectutils.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Functions common to all objects, they are included in this file
 * 			   to avoid duplication on each generated object. The functions
 *             in this file should be called only by UAVObjects.
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
#include <string.h> // for memcpy
#include "uavobjectutils.h"
#include "uavobject.h"
#include "utlist.h"

/**
 * See UAVObject.h description for more details.
 */
int32_t UAVObjUtilsConnect(ObjectContext* context, xQueueHandle queue, int32_t eventMask)
{
	ObjectQueueList* queueHandle;

	// Lock
	xSemaphoreTakeRecursive(context->mutex, portMAX_DELAY);

	// Check that the queue is not already connected, if it is simply update event mask
	LL_FOREACH(context->queues, queueHandle)
	{
		if (queueHandle->queue == queue)
		{
			// Already connected, update event mask and return
			queueHandle->eventMask = eventMask;
			xSemaphoreGiveRecursive(context->mutex);
			return 0;
		}
	}

	// Add queue to list
	queueHandle = (ObjectQueueList*)malloc(sizeof(ObjectQueueList));
	queueHandle->queue = queue;
	queueHandle->eventMask = eventMask;
	LL_APPEND(context->queues, queueHandle);

	// Done
	xSemaphoreGiveRecursive(context->mutex);
	return 0;
}

/**
 * See UAVObject.h description for more details.
 */
int32_t UAVObjUtilsDisconnect(ObjectContext* context, xQueueHandle queue)
{
	ObjectQueueList* queueHandle;

	// Lock
	xSemaphoreTakeRecursive(context->mutex, portMAX_DELAY);

	// Find queue and remove it
	LL_FOREACH(context->queues, queueHandle)
	{
		if (queueHandle->queue == queue)
		{
			LL_DELETE(context->queues, queueHandle);
			xSemaphoreGiveRecursive(context->mutex);
			return 0;
		}
	}

	// If this point is reached the queue was not found
	xSemaphoreGiveRecursive(context->mutex);
	return -1;
}

/**
 * See UAVObject.h description for more details.
 */
void UAVObjUtilsSendEvent(ObjectContext* context, UAVObjectQMsgEvent event)
{
	ObjectQueueList* queueHandle;
	UAVObjectQMsg msg;

	// Setup event
	msg.obj = &context->obj;
	msg.event = event;

	// Go through each object and push the object ID in the queue (if event is activated for the queue)
    LL_FOREACH(context->queues, queueHandle)
	{
    	if ( queueHandle->eventMask == 0 || (queueHandle->eventMask & event) != 0 )
    	{
    		xQueueSend(queueHandle->queue, &msg, 0); // do not wait if queue is full
    	}
    }
}

/**
 * See UAVObject.h description for more details.
 */
void UAVObjUtilsRequestUpdate(ObjectContext* context)
{
	xSemaphoreTakeRecursive(context->mutex, portMAX_DELAY);
	UAVObjUtilsSendEvent(context, QMSG_UPDATE_REQ);
	xSemaphoreGiveRecursive(context->mutex);
}

/**
 * See UAVObject.h description for more details.
 */
void UAVObjUtilsUpdated(ObjectContext* context)
{
	xSemaphoreTakeRecursive(context->mutex, portMAX_DELAY);
	UAVObjUtilsSendEvent(context, QMSG_UPDATED_MANUAL);
	xSemaphoreGiveRecursive(context->mutex);
}
