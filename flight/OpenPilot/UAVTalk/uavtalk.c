/**
 ******************************************************************************
 *
 * @file       uavtalk.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      UAVTalk library, implements to telemetry protocol. See the wiki for more details.
 * 			   This library should not be called directly by the application, it is only used by the
 * 		       Telemetry module.
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
#include "uavtalk.h"
#include "utlist.h"
#include "FreeRTOS.h"
#include "semphr.h"

// Private constants
#define TYPE_MASK 0xFC
#define TYPE_BASE 0x50
#define TYPE_OBJ (TYPE_BASE | 0x00)
#define TYPE_OBJ_REQ (TYPE_BASE | 0x01)
#define TYPE_OBJ_ACK (TYPE_BASE | 0x02)
#define TYPE_ACK (TYPE_BASE | 0x03)

#define HEADER_LENGTH 6 // type (1), object ID (4), length (1)
#define CHECKSUM_LENGTH 2
#define MAX_PAYLOAD_LENGTH 256
#define MAX_PACKET_LENGTH (HEADER_LENGTH+MAX_PAYLOAD_LENGTH+CHECKSUM_LENGTH)
#define MAX_UPDATE_PERIOD_MS 1000
#define MIN_UPDATE_PERIOD_MS 1

// Private types
struct ObjectHandleStruct {
    uint32_t objectId;
    UAVTalkUnpackCb packCb;
    UAVTalkUnpackCb unpackCb;
    xSemaphoreHandle sema;
    uint32_t waitingResp;
    int32_t updatePeriodMs;
    int32_t timeToNextUpdateMs;
    struct ObjectHandleStruct* next;
};
typedef struct ObjectHandleStruct ObjectHandle;
typedef enum {STATE_SYNC, STATE_OBJID, STATE_LENGTH, STATE_DATA, STATE_CS} RxState;

// Private variables
UAVTalkOutputStream outStream;
ObjectHandle* objects;
int32_t timeToNextUpdateMs;
xSemaphoreHandle mutex;
uint8_t rxBuffer[MAX_PACKET_LENGTH];
uint8_t txBuffer[MAX_PACKET_LENGTH];

// Private functions
uint16_t updateChecksum(uint16_t cs, uint8_t* data, int32_t length);
ObjectHandle* findObject(uint32_t objId);
int32_t objectTransaction(uint32_t objectId, uint8_t type, int32_t timeout);
int32_t sendObject(ObjectHandle* obj, uint8_t type);
int32_t receiveObject(uint8_t type, ObjectHandle* obj, uint8_t* data, int32_t length);

/**
 * Initialize the UAVTalk library
 * \param[in] outputStream Function pointer that is called to send a data buffer
 * \return 0 Success
 * \return -1 Failure
 */
int32_t UAVTalkInitialize(UAVTalkOutputStream outputStream) {
    outStream = outputStream;
    mutex = xSemaphoreCreateRecursiveMutex();
    timeToNextUpdateMs = 0;
    objects = NULL;

    return 0;
}

/**
 * Connect an object to the UAVTalk library. All objects needs to be registered, this is needed
 * so that the library knows how to call the pack and unpack functions of the object.
 * \param[in] objectId ID of the object
 * \param[in] packCb Callback function that is used to pack the object, called each time the object needs to be sent.
 * \param[in] unpackCb Callback function that is used to unpack the object, called each time the object is received.
 * \return 0 Success
 * \return -1 Failure
 */
int32_t UAVTalkConnectObject(uint32_t objectId, UAVTalkPackCb packCb, UAVTalkUnpackCb unpackCb, int32_t updatePeriodMs) {
    ObjectHandle* obj;
    // Lock
    xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
	// Check that the object is not already connected
	LL_FOREACH(objects, obj)
	{
		if (obj->objectId == objectId)
		{
			// Already registered, ignore
			xSemaphoreGiveRecursive(mutex);
			return -1;
		}
	}
    // Create handle
    obj = (ObjectHandle*)malloc(sizeof(ObjectHandle));
    obj->objectId = objectId;
    obj->packCb = packCb;
    obj->unpackCb = unpackCb;
    vSemaphoreCreateBinary(obj->sema);
    obj->waitingResp = 0;
    obj->updatePeriodMs = updatePeriodMs;
    obj->timeToNextUpdateMs = 0;
    // Add to list
    LL_APPEND(objects, obj);
    // Done
    xSemaphoreGiveRecursive(mutex);
    return 0;
}

/**
 * Setup object for periodic updates.
 * \param[in] objectId ID of the object to update
 * \param[in] updatePeriodMs The update period in ms, if zero then periodic updates are disabled
 * \return 0 Success
 * \return -1 Failure
 */
int32_t UAVTalkSetUpdatePeriod(uint32_t objectId, int32_t updatePeriodMs) {
    ObjectHandle* obj;
    // Lock
    xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
    // Get and update object
    obj = findObject(objectId);
    if (obj != 0) {
        obj->updatePeriodMs = updatePeriodMs;
        obj->timeToNextUpdateMs = 0;
        xSemaphoreGiveRecursive(mutex);
        return 0;
    }
    else {
    	xSemaphoreGiveRecursive(mutex);
        return -1;
    }
}

/**
 * Request an update for the specified object, on success the object data would have been
 * updated by the GCS.
 * \param[in] objectId ID of the object to update
 * \param[in] timeout Time to wait for the response, when zero it will return immediately
 * \return 0 Success
 * \return -1 Failure
 */
int32_t UAVTalkSendObjectRequest(uint32_t objectId, int32_t timeout) {
    return objectTransaction(objectId, TYPE_OBJ_REQ, timeout);
}

/**
 * Send the specified object through the telemetry link.
 * \param[in] objectId ID of the object to send
 * \param[in] acked Selects if an ack is required (1:ack required, 0: ack not required)
 * \param[in] timeoutMs Time to wait for the ack, when zero it will return immediately
 * \return 0 Success
 * \return -1 Failure
 */
int32_t UAVTalkSendObject(uint32_t objectId, uint8_t acked, int32_t timeoutMs) {
    if (acked == 1) {
        return objectTransaction(objectId, TYPE_OBJ_ACK, timeoutMs);
    } else {
        return objectTransaction(objectId, TYPE_OBJ, timeoutMs);
    }
}

/**
 * Execute the requested transaction on an object.
 * \param[in] objectId ID of object
 * \param[in] type Transaction type
 * 			  TYPE_OBJ: send object,
 * 			  TYPE_OBJ_REQ: request object update
 * 			  TYPE_OBJ_ACK: send object with an ack
 * \return 0 Success
 * \return -1 Failure
 */
int32_t objectTransaction(uint32_t objectId, uint8_t type, int32_t timeoutMs) {
    ObjectHandle* obj;

    // Lock
    xSemaphoreTakeRecursive(mutex, portMAX_DELAY);

    // Find object
    obj = findObject(objectId);
    if (obj == 0) {
    	xSemaphoreGiveRecursive(mutex);
        return -1;
    }

    // Send object depending on if a response is needed
    if (type == TYPE_OBJ_ACK || type == TYPE_OBJ_REQ) {
        sendObject(obj, type);
        xSemaphoreGiveRecursive(mutex); // need to release lock since the next call will block until a response is received
        xSemaphoreTake(obj->sema, 0); // the semaphore needs to block on the next call, here we make sure the value is zero (binary sema)
        xSemaphoreTake(obj->sema, timeoutMs/portTICK_RATE_MS); // lock on object until a response is received (or timeout)
        xSemaphoreTakeRecursive(mutex, portMAX_DELAY); // complete transaction
        // Check if a response was received
        if (obj->waitingResp == 1) {
            obj->waitingResp = 0;
            xSemaphoreGiveRecursive(mutex);
            return -1;
        } else {
        	xSemaphoreGiveRecursive(mutex);
            return 0;
        }
    } else if (type == TYPE_OBJ) {
        sendObject(obj, TYPE_OBJ);
        xSemaphoreGiveRecursive(mutex);
        return 0;
    } else {
    	xSemaphoreGiveRecursive(mutex);
        return -1;
    }
}

/**
 * Handle periodic updates for all objects.
 * \return The time to wait until the next update (in ms)
 * \return 0 Success
 * \return -1 Failure
 */
int32_t UAVTalkProcessPeriodicUpdates(void) {
    ObjectHandle* obj;
    int32_t minDelay = MAX_UPDATE_PERIOD_MS;

    // Lock
    xSemaphoreTakeRecursive(mutex, portMAX_DELAY);

    // Iterate through each object and update its timer, if zero then transmit object.
    // Also calculate smallest delay to next update (will be used for setting timeToNextUpdateMs)
    LL_FOREACH(objects, obj) {
        // If object is configured for periodic updates
        if (obj->updatePeriodMs > 0) {
            obj->timeToNextUpdateMs -= timeToNextUpdateMs;
            // Check if time for the next update
            if (obj->timeToNextUpdateMs <= 0) {
                // Reset timer
                obj->timeToNextUpdateMs = obj->updatePeriodMs;
                // Send object
                sendObject(obj, TYPE_OBJ);
            }
            // Update minimum delay
            if (obj->timeToNextUpdateMs < minDelay) {
                minDelay = obj->timeToNextUpdateMs;
            }
        }
    }

    // Check if delay for the next update is too short
    if (minDelay < MIN_UPDATE_PERIOD_MS) {
        minDelay = MIN_UPDATE_PERIOD_MS;
    }

    // Done
    timeToNextUpdateMs = minDelay;
    xSemaphoreGiveRecursive(mutex);
    return timeToNextUpdateMs;
}

/**
 * Process an byte from the telemetry stream.
 * \param[in] rxbyte Received byte
 * \return 0 Success
 * \return -1 Failure
 */
int32_t UAVTalkProcessInputStream(uint8_t rxbyte) {
    static uint8_t tmpBuffer[4];
    static ObjectHandle* obj;
    static uint8_t type;
    static uint32_t objId;
    static uint8_t length;
    static uint16_t cs, csRx;
    static int32_t rxCount;
    static RxState state = STATE_SYNC;

    // Receive state machine
    switch (state) {
    case STATE_SYNC:
        if ((rxbyte & TYPE_MASK) == TYPE_BASE ) {
            cs = rxbyte;
            type = rxbyte;
            state = STATE_OBJID;
            rxCount = 0;
        }
        break;
    case STATE_OBJID:
        tmpBuffer[rxCount++] = rxbyte;
        if (rxCount == 4) {
            // Search for object, if not found reset state machine
            objId = (tmpBuffer[3] << 24) | (tmpBuffer[2] << 16) | (tmpBuffer[1] << 8) | (tmpBuffer[0]);
            xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
            obj = findObject(objId);
            xSemaphoreGiveRecursive(mutex);
            if (obj == 0) {
                state = STATE_SYNC;
            } else {
                cs = updateChecksum(cs, tmpBuffer, 4);
                state = STATE_LENGTH;
                rxCount = 0;
            }
        }
        break;
    case STATE_LENGTH:
        length = (int32_t)rxbyte;
        if (length > MAX_PAYLOAD_LENGTH ||
            ((type == TYPE_OBJ_REQ || type == TYPE_ACK) && length != 0)) {
            state = STATE_SYNC;
        } else {
            cs = updateChecksum(cs, &length, 1);
            rxCount = 0;
            if (length > 0) {
                state = STATE_DATA;
            } else {
                state = STATE_CS;
            }
        }
        break;
    case STATE_DATA:
        rxBuffer[rxCount++] = rxbyte;
        if (rxCount == length) {
            cs = updateChecksum(cs, rxBuffer, length);
            state = STATE_CS;
            rxCount = 0;
        }
        break;
    case STATE_CS:
        tmpBuffer[rxCount++] = rxbyte;
        if (rxCount == 2) {
            csRx = (tmpBuffer[1] << 8) | (tmpBuffer[0]);
            if (csRx == cs) {
                xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
                receiveObject(type, obj, rxBuffer, length);
                xSemaphoreGiveRecursive(mutex);
            }
            state = STATE_SYNC;
        }
        break;
    default:
        state = STATE_SYNC;
    }

    // Done
    return 0;
}

/**
 * Receive an object. This function process objects received through the telemetry stream.
 * \param[in] type Type of received message (TYPE_OBJ, TYPE_OBJ_REQ, TYPE_OBJ_ACK, TYPE_ACK)
 * \param[in] obj Handle of the received object
 * \param[in] data Data buffer
 * \param[in] length Buffer length
 * \return 0 Success
 * \return -1 Failure
 */
int32_t receiveObject(uint8_t type, ObjectHandle* obj, uint8_t* data, int32_t length) {
    // Unpack object if the message is of type OBJ or OBJ_ACK
    if (type == TYPE_OBJ || type == TYPE_OBJ_ACK) {
       (obj->unpackCb)(obj->objectId, data, length);
    }

    // Send requested object if message is of type OBJ_REQ
    if (type == TYPE_OBJ_REQ) {
       sendObject(obj, TYPE_OBJ);
    }

    // Send ACK if message is of type OBJ_ACK
    if (type == TYPE_OBJ_ACK) {
       sendObject(obj, TYPE_ACK);
    }

    // If a response was pending on the object, unblock any waiting tasks
    if (type == TYPE_ACK || type == TYPE_OBJ) {
        if (obj->waitingResp == 1) {
            obj->waitingResp = 0;
            xSemaphoreGive(obj->sema);
        }
    }

    // Done
    return 0;
}

/**
 * Send an object through the telemetry link.
 * \param[in] obj Object handle to send
 * \param[in] type Transaction type
 * \return 0 Success
 * \return -1 Failure
 */
int32_t sendObject(ObjectHandle* obj, uint8_t type) {
    int32_t length;
    uint16_t cs = 0;

    // Check for valid packet type
    if (type != TYPE_OBJ && type != TYPE_OBJ_ACK && type != TYPE_OBJ_REQ && type != TYPE_ACK) {
        return -1;
    }

    // If a response is expected, set the flag
    if (type == TYPE_OBJ_ACK || type == TYPE_OBJ_REQ) {
        obj->waitingResp = 1;
    }

    // Setup type and object id fields
    txBuffer[0] = type;
    txBuffer[1] = (uint8_t)(obj->objectId & 0xFF);
    txBuffer[2] = (uint8_t)((obj->objectId >> 8) & 0xFF);
    txBuffer[3] = (uint8_t)((obj->objectId >> 16) & 0xFF);
    txBuffer[4] = (uint8_t)((obj->objectId >> 24) & 0xFF);

    // Setup length and data field (if one)
    if (type == TYPE_ACK || type == TYPE_OBJ_REQ) {
        length = 0;
    } else {
        // Pack object
        length = (obj->packCb)(obj->objectId, &txBuffer[HEADER_LENGTH], MAX_PAYLOAD_LENGTH);
        // Check length
        if (length > MAX_PAYLOAD_LENGTH || length <= 0) {
            return -1;
        }
    }
    txBuffer[5] = (uint8_t)length;

    // Calculate checksum
    cs = 0;
    cs = updateChecksum(cs, txBuffer, HEADER_LENGTH+length);
    txBuffer[HEADER_LENGTH+length] = (uint8_t)(cs & 0xFF);
    txBuffer[HEADER_LENGTH+length+1] = (uint8_t)((cs >> 8) & 0xFF);

    // Send buffer
    if (outStream!=NULL) (*outStream)(txBuffer, HEADER_LENGTH+length+CHECKSUM_LENGTH);

    // Done
    return 0;
}

/**
 * Update checksum.
 * TODO: Replace with CRC-16
 * \param[in] data Data buffer to update checksum on
 * \param[in] length Length of buffer
 * \return Updated checksum
 */
uint16_t updateChecksum(uint16_t cs, uint8_t* data, int32_t length) {
    int32_t n;
    for (n = 0; n < length; ++n) {
        cs += (uint16_t)data[n];
    }
    return cs;
}

/**
 * Find an object handle given the object ID
 * \param[in] objId Object ID
 * \return The object handle or NULL if not found
 */
ObjectHandle* findObject(uint32_t objId) {
    ObjectHandle* obj;
    LL_FOREACH(objects, obj) {
        if (obj->objectId == objId) {
            return obj;
        }
    }
    return NULL;
}

