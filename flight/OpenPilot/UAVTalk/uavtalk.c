/**
 ******************************************************************************
 *
 * @file       uavtalk.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      UAVTalk library, implements to telemetry protocol. See the wiki for more details.
 * 	       This library should not be called directly by the application, it is only used by the
 * 	       Telemetry module.
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

#include "openpilot.h"

// Private constants
#define TYPE_MASK 0xFC
#define TYPE_VER 0x10
#define TYPE_OBJ (TYPE_VER | 0x00)
#define TYPE_OBJ_REQ (TYPE_VER | 0x01)
#define TYPE_OBJ_ACK (TYPE_VER | 0x02)
#define TYPE_ACK (TYPE_VER | 0x03)

#define HEADER_LENGTH 7 // type (1), object ID (4), instance ID (2, not used in single objects)
#define CHECKSUM_LENGTH 2
#define MAX_PAYLOAD_LENGTH 256
#define MAX_PACKET_LENGTH (HEADER_LENGTH+MAX_PAYLOAD_LENGTH+CHECKSUM_LENGTH)

// Private types
typedef enum {STATE_SYNC, STATE_OBJID, STATE_INSTID, STATE_DATA, STATE_CS} RxState;

// Private variables
UAVTalkOutputStream outStream;
xSemaphoreHandle lock;
xSemaphoreHandle respSema;
UAVObjHandle respObj;
uint16_t respInstId;
uint8_t rxBuffer[MAX_PACKET_LENGTH];
uint8_t txBuffer[MAX_PACKET_LENGTH];

// Private functions
uint16_t updateChecksum(uint16_t cs, uint8_t* data, int32_t length);
int32_t objectTransaction(uint32_t objectId, uint16_t instId, uint8_t type, int32_t timeout);
int32_t sendObject(UAVObjHandle obj, uint16_t instId, uint8_t type);
int32_t sendSingleObject(UAVObjHandle obj, uint16_t instId, uint8_t type);
int32_t receiveObject(uint8_t type, UAVObjHandle obj, uint16_t instId, uint8_t* data, int32_t length);
void updateAck(UAVObjHandle obj, uint16_t instId);

/**
 * Initialize the UAVTalk library
 * \param[in] outputStream Function pointer that is called to send a data buffer
 * \return 0 Success
 * \return -1 Failure
 */
int32_t UAVTalkInitialize(UAVTalkOutputStream outputStream)
{
    outStream = outputStream;
    lock = xSemaphoreCreateRecursiveMutex();
    vSemaphoreCreateBinary(respSema);
    return 0;
}

/**
 * Request an update for the specified object, on success the object data would have been
 * updated by the GCS.
 * \param[in] obj Object to update
 * \param[in] instId The instance ID of UAVOBJ_ALL_INSTANCES for all instances.
 * \param[in] timeout Time to wait for the response, when zero it will return immediately
 * \return 0 Success
 * \return -1 Failure
 */
int32_t UAVTalkSendObjectRequest(UAVObjHandle obj, uint16_t instId, int32_t timeout)
{
    return objectTransaction(obj, instId, TYPE_OBJ_REQ, timeout);
}

/**
 * Send the specified object through the telemetry link.
 * \param[in] obj Object to send
 * \param[in] instId The instance ID
 * \param[in] acked Selects if an ack is required (1:ack required, 0: ack not required)
 * \param[in] timeoutMs Time to wait for the ack, when zero it will return immediately
 * \return 0 Success
 * \return -1 Failure
 */
int32_t UAVTalkSendObject(UAVObjHandle obj, uint16_t instId, uint8_t acked, int32_t timeoutMs)
{
	// Make sure a valid instance id is requested
	if (instId == UAVOBJ_ALL_INSTANCES)
	{
		return -1;
	}
	// Send object
    if (acked == 1)
    {
        return objectTransaction(obj, instId, TYPE_OBJ_ACK, timeoutMs);
    }
    else
    {
        return objectTransaction(obj, instId, TYPE_OBJ, timeoutMs);
    }
}

/**
 * Execute the requested transaction on an object.
 * \param[in] obj Object
 * \param[in] instId The instance ID of UAVOBJ_ALL_INSTANCES for all instances.
 * \param[in] type Transaction type
 * 			  TYPE_OBJ: send object,
 * 			  TYPE_OBJ_REQ: request object update
 * 			  TYPE_OBJ_ACK: send object with an ack
 * \return 0 Success
 * \return -1 Failure
 */
int32_t objectTransaction(UAVObjHandle obj, uint16_t instId, uint8_t type, int32_t timeoutMs)
{
	int32_t respReceived;

	// Lock
    xSemaphoreTakeRecursive(lock, portMAX_DELAY);

    // Send object depending on if a response is needed
    if (type == TYPE_OBJ_ACK || type == TYPE_OBJ_REQ)
    {
        sendObject(obj, instId, type);
        respObj = obj;
        respInstId = instId;
        xSemaphoreGiveRecursive(lock); // need to release lock since the next call will block until a response is received
    	xSemaphoreTake(respSema, 0); // the semaphore needs to block on the next call, here we make sure the value is zero (binary sema)
    	respReceived = xSemaphoreTake(respSema, timeoutMs/portTICK_RATE_MS); // lock on object until a response is received (or timeout)
    	// Check if a response was received
    	if (respReceived == pdFALSE)
    	{
    		return -1;
    	}
    	else
    	{
    		return 0;
    	}
    }
    else if (type == TYPE_OBJ)
    {
        sendObject(obj, instId, TYPE_OBJ);
        xSemaphoreGiveRecursive(lock);
        return 0;
    }
    else
    {
    	xSemaphoreGiveRecursive(lock);
        return -1;
    }
}

/**
 * Process an byte from the telemetry stream.
 * \param[in] rxbyte Received byte
 * \return 0 Success
 * \return -1 Failure
 */
int32_t UAVTalkProcessInputStream(uint8_t rxbyte)
{
    static uint8_t tmpBuffer[4];
    static UAVObjHandle obj;
    static uint8_t type;
    static uint32_t objId;
    static uint16_t instId;
    static uint32_t length;
    static uint16_t cs, csRx;
    static int32_t rxCount;
    static RxState state = STATE_SYNC;

    // Receive state machine
    switch (state) {
    case STATE_SYNC:
        if ((rxbyte & TYPE_MASK) == TYPE_VER )
        {
            cs = rxbyte;
            type = rxbyte;
            state = STATE_OBJID;
            rxCount = 0;
        }
        break;
    case STATE_OBJID:
        tmpBuffer[rxCount++] = rxbyte;
        if (rxCount == 4)
        {
            // Search for object, if not found reset state machine
            objId = (tmpBuffer[3] << 24) | (tmpBuffer[2] << 16) | (tmpBuffer[1] << 8) | (tmpBuffer[0]);
            obj = UAVObjGetByID(objId);
            if (obj == 0)
            {
                state = STATE_SYNC;
            }
            else
            {
            	// Update checksum
                cs = updateChecksum(cs, tmpBuffer, 4);
                // Determine data length
                if (type == TYPE_OBJ_REQ || type == TYPE_ACK)
                {
                	length = 0;
                }
                else
                {
					length = UAVObjGetNumBytes(obj);
                }
                // Check length and determine next state
				if (length >= MAX_PAYLOAD_LENGTH)
				{
					state = STATE_SYNC;
				}
				else
				{
					// Check if this is a single instance object (i.e. if the instance ID field is coming next)
					if ( UAVObjIsSingleInstance(obj) )
					{
						// If there is a payload get it, otherwise receive checksum
			        	if (length > 0)
			        	{
			        		state = STATE_DATA;
			        	}
			        	else
			        	{
			        		state = STATE_CS;
			        	}
						instId = 0;
						rxCount = 0;
					}
					else
					{
						state = STATE_INSTID;
						rxCount = 0;
					}
				}
            }
        }
        break;
    case STATE_INSTID:
        tmpBuffer[rxCount++] = rxbyte;
        if (rxCount == 2)
        {
        	instId = (tmpBuffer[1] << 8) | (tmpBuffer[0]);
        	cs = updateChecksum(cs, tmpBuffer, 2);
        	rxCount = 0;
        	// If there is a payload get it, otherwise receive checksum
        	if (length > 0)
        	{
        		state = STATE_DATA;
        	}
        	else
        	{
        		state = STATE_CS;
        	}
        }
        break;
    case STATE_DATA:
        rxBuffer[rxCount++] = rxbyte;
        if (rxCount == length)
        {
            cs = updateChecksum(cs, rxBuffer, length);
            state = STATE_CS;
            rxCount = 0;
        }
        break;
    case STATE_CS:
        tmpBuffer[rxCount++] = rxbyte;
        if (rxCount == 2)
        {
            csRx = (tmpBuffer[1] << 8) | (tmpBuffer[0]);
            if (csRx == cs)
            {
                xSemaphoreTakeRecursive(lock, portMAX_DELAY);
                receiveObject(type, obj, instId, rxBuffer, length);
                xSemaphoreGiveRecursive(lock);
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
 * \param[in] instId The instance ID of UAVOBJ_ALL_INSTANCES for all instances.
 * \param[in] data Data buffer
 * \param[in] length Buffer length
 * \return 0 Success
 * \return -1 Failure
 */
int32_t receiveObject(uint8_t type, UAVObjHandle obj, uint16_t instId, uint8_t* data, int32_t length)
{
    int32_t ret = 0;

    // Process message type
    switch (type) {
    case TYPE_OBJ:
        // All instances, not allowed for OBJ messages
        if (instId != UAVOBJ_ALL_INSTANCES)
        {
            // Unpack object, if the instance does not exist it will be created!
        	UAVObjUnpack(obj, instId, data);
            // Check if an ack is pending
            updateAck(obj, instId);
        }
        else
        {
            ret = -1;
        }
        break;
    case TYPE_OBJ_ACK:
        // All instances, not allowed for OBJ_ACK messages
        if (instId != UAVOBJ_ALL_INSTANCES)
        {
            // Unpack object, if the instance does not exist it will be created!
        	if ( UAVObjUnpack(obj, instId, data) == 0 )
        	{
        		// Transmit ACK
        		sendObject(obj, instId, TYPE_ACK);
        	}
        	else
        	{
        		ret = -1;
        	}
        }
        else
        {
        	ret = -1;
        }
        break;
    case TYPE_OBJ_REQ:
        // Send requested object if message is of type OBJ_REQ
    	sendObject(obj, instId, TYPE_OBJ);
        break;
    case TYPE_ACK:
        // All instances, not allowed for ACK messages
        if (instId != UAVOBJ_ALL_INSTANCES)
        {
            // Check if an ack is pending
            updateAck(obj, instId);
        }
        else
        {
        	ret = -1;
        }
        break;
    default:
        ret = -1;
    }
    // Done
    return ret;
}

/**
 * Check if an ack is pending on an object and give response semaphore
 */
void updateAck(UAVObjHandle obj, uint16_t instId)
{
    if (respObj == obj && (respInstId == instId || respInstId == UAVOBJ_ALL_INSTANCES))
    {
    	xSemaphoreGive(respSema);
        respObj = 0;
    }
}

/**
 * Send an object through the telemetry link.
 * \param[in] obj Object handle to send
 * \param[in] instId The instance ID or UAVOBJ_ALL_INSTANCES for all instances
 * \param[in] type Transaction type
 * \return 0 Success
 * \return -1 Failure
 */
int32_t sendObject(UAVObjHandle obj, uint16_t instId, uint8_t type)
{
	uint32_t numInst;
	uint32_t n;

	// If all instances are requested on a single instance object it is an error
	if ( instId == UAVOBJ_ALL_INSTANCES && UAVObjIsSingleInstance(obj) )
	{
		return -1;
	}

    // Process message type
    if ( type == TYPE_OBJ || type == TYPE_OBJ_ACK )
    {
    	if (instId == UAVOBJ_ALL_INSTANCES)
    	{
			// Get number of instances
			numInst = UAVObjGetNumInstances(obj);
			// Send all instances
			for (n = 0; n < numInst; ++n)
			{
				sendSingleObject(obj, n, type);
			}
			return 0;
    	}
    	else
    	{
    		return sendSingleObject(obj, instId, type);
    	}
    }
    else if (type == TYPE_OBJ_REQ)
    {
        return sendSingleObject(obj, instId, TYPE_OBJ_REQ);
    }
    else if (type == TYPE_ACK)
    {
        if ( instId != UAVOBJ_ALL_INSTANCES )
        {
            return sendSingleObject(obj, instId, TYPE_ACK);
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }
}

/**
 * Send an object through the telemetry link.
 * \param[in] obj Object handle to send
 * \param[in] instId The instance ID (can NOT be UAVOBJ_ALL_INSTANCES, use sendObject() instead)
 * \param[in] type Transaction type
 * \return 0 Success
 * \return -1 Failure
 */
int32_t sendSingleObject(UAVObjHandle obj, uint16_t instId, uint8_t type)
{
    int32_t length;
    int32_t dataOffset;
    uint16_t cs = 0;
    uint32_t objId;

    // Setup type and object id fields
    objId = UAVObjGetID(obj);
    txBuffer[0] = type;
    txBuffer[1] = (uint8_t)(objId & 0xFF);
    txBuffer[2] = (uint8_t)((objId >> 8) & 0xFF);
    txBuffer[3] = (uint8_t)((objId >> 16) & 0xFF);
    txBuffer[4] = (uint8_t)((objId >> 24) & 0xFF);

    // Setup instance ID if one is required
    if (UAVObjIsSingleInstance(obj))
    {
    	dataOffset = 5;
    }
    else
    {
    	txBuffer[5] = (uint8_t)(instId & 0xFF);
    	txBuffer[6] = (uint8_t)((instId >> 8) & 0xFF);
    	dataOffset = 7;
    }

    // Determine data length
    if (type == TYPE_OBJ_REQ || type == TYPE_ACK)
    {
    	length = 0;
    }
    else
    {
		length = UAVObjGetNumBytes(obj);
    }

    // Check length
    if (length >= MAX_PAYLOAD_LENGTH)
    {
    	return -1;
    }

    // Copy data (if any)
    if (length > 0)
    {
    	if ( UAVObjPack(obj, instId, &txBuffer[dataOffset]) < 0 )
    	{
    		return -1;
    	}
    }

    // Calculate checksum
    cs = 0;
    cs = updateChecksum(cs, txBuffer, dataOffset+length);
    txBuffer[dataOffset+length] = (uint8_t)(cs & 0xFF);
    txBuffer[dataOffset+length+1] = (uint8_t)((cs >> 8) & 0xFF);

    // Send buffer
    if (outStream!=NULL) (*outStream)(txBuffer, dataOffset+length+CHECKSUM_LENGTH);

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
uint16_t updateChecksum(uint16_t cs, uint8_t* data, int32_t length)
{
    int32_t n;
    for (n = 0; n < length; ++n)
    {
        cs += (uint16_t)data[n];
    }
    return cs;
}







