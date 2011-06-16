/**
 ******************************************************************************
 * @addtogroup OpenPilotSystem OpenPilot System
 * @{
 * @addtogroup OpenPilotLibraries OpenPilot System Libraries
 * @{
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
#define SYNC_VAL			0x3C
#define TYPE_MASK			0xF8
#define TYPE_VER			0x20
#define TYPE_OBJ			(TYPE_VER | 0x00)
#define TYPE_OBJ_REQ		(TYPE_VER | 0x01)
#define TYPE_OBJ_ACK		(TYPE_VER | 0x02)
#define TYPE_ACK			(TYPE_VER | 0x03)
#define TYPE_NACK			(TYPE_VER | 0x04)

// Private types

// Private functions
static int32_t objectTransaction(UAVTalkConnection *connection, UAVObjHandle objectId, uint16_t instId, uint8_t type, int32_t timeout);
static int32_t sendObject(UAVTalkConnection *connection, UAVObjHandle obj, uint16_t instId, uint8_t type);
static int32_t sendSingleObject(UAVTalkConnection *connection, UAVObjHandle obj, uint16_t instId, uint8_t type);
static int32_t sendNack(UAVTalkConnection *connection, uint32_t objId);
static int32_t receiveObject(UAVTalkConnection *connection, uint8_t type, uint32_t objId, uint16_t instId, uint8_t* data, int32_t length);
static void updateAck(UAVTalkConnection *connection, UAVObjHandle obj, uint16_t instId);

/**
 * Initialize the UAVTalk library
 * \param[in] connection UAVTalkConnection to be used
 * \param[in] outputStream Function pointer that is called to send a data buffer
 * \return 0 Success
 * \return -1 Failure
 */
int32_t UAVTalkInitialize(UAVTalkConnection *connection, UAVTalkOutputStream outputStream)
{
	connection->iproc.rxPacketLength = 0;
	connection->iproc.state = UAVTALK_STATE_SYNC;
	connection->outStream = outputStream;
	connection->lock = xSemaphoreCreateRecursiveMutex();
	connection->transLock = xSemaphoreCreateRecursiveMutex();
	vSemaphoreCreateBinary(connection->respSema);
	xSemaphoreTake(connection->respSema, 0); // reset to zero
	UAVTalkResetStats(connection);
	return 0;
}

/**
 * Set the communication output stream
 * \param[in] connection UAVTalkConnection to be used
 * \param[in] outputStream Function pointer that is called to send a data buffer
 * \return 0 Success
 * \return -1 Failure
 */
int32_t UAVTalkSetOutputStream(UAVTalkConnection *connection, UAVTalkOutputStream outputStream)
{
	// Lock
	xSemaphoreTakeRecursive(connection->lock, portMAX_DELAY);
	
	// set output stream
	connection->outStream = outputStream;
	
	// Release lock
	xSemaphoreGiveRecursive(connection->lock);

	return 0;

}

/**
 * Get current output stream
 * \param[in] connection UAVTalkConnection to be used
 * @return UAVTarlkOutputStream the output stream used
 */
UAVTalkOutputStream UAVTalkGetOutputStream(UAVTalkConnection *connection)
{
	return connection->outStream;
}

/**
 * Get communication statistics counters
 * \param[in] connection UAVTalkConnection to be used
 * @param[out] statsOut Statistics counters
 */
void UAVTalkGetStats(UAVTalkConnection *connection, UAVTalkStats* statsOut)
{
	// Lock
	xSemaphoreTakeRecursive(connection->lock, portMAX_DELAY);
	
	// Copy stats
	memcpy(statsOut, &connection->stats, sizeof(UAVTalkStats));
	
	// Release lock
	xSemaphoreGiveRecursive(connection->lock);
}

/**
 * Reset the statistics counters.
 * \param[in] connection UAVTalkConnection to be used
 */
void UAVTalkResetStats(UAVTalkConnection *connection)
{
	// Lock
	xSemaphoreTakeRecursive(connection->lock, portMAX_DELAY);
	
	// Clear stats
	memset(&connection->stats, 0, sizeof(UAVTalkStats));
	
	// Release lock
	xSemaphoreGiveRecursive(connection->lock);
}

/**
 * Request an update for the specified object, on success the object data would have been
 * updated by the GCS.
 * \param[in] connection UAVTalkConnection to be used
 * \param[in] obj Object to update
 * \param[in] instId The instance ID or UAVOBJ_ALL_INSTANCES for all instances.
 * \param[in] timeout Time to wait for the response, when zero it will return immediately
 * \return 0 Success
 * \return -1 Failure
 */
int32_t UAVTalkSendObjectRequest(UAVTalkConnection *connection, UAVObjHandle obj, uint16_t instId, int32_t timeout)
{
	return objectTransaction(connection, obj, instId, TYPE_OBJ_REQ, timeout);
}

/**
 * Send the specified object through the telemetry link.
 * \param[in] connection UAVTalkConnection to be used
 * \param[in] obj Object to send
 * \param[in] instId The instance ID or UAVOBJ_ALL_INSTANCES for all instances.
 * \param[in] acked Selects if an ack is required (1:ack required, 0: ack not required)
 * \param[in] timeoutMs Time to wait for the ack, when zero it will return immediately
 * \return 0 Success
 * \return -1 Failure
 */
int32_t UAVTalkSendObject(UAVTalkConnection *connection, UAVObjHandle obj, uint16_t instId, uint8_t acked, int32_t timeoutMs)
{
	// Send object
	if (acked == 1)
	{
		return objectTransaction(connection, obj, instId, TYPE_OBJ_ACK, timeoutMs);
	}
	else
	{
		return objectTransaction(connection, obj, instId, TYPE_OBJ, timeoutMs);
	}
}

/**
 * Execute the requested transaction on an object.
 * \param[in] connection UAVTalkConnection to be used
 * \param[in] obj Object
 * \param[in] instId The instance ID of UAVOBJ_ALL_INSTANCES for all instances.
 * \param[in] type Transaction type
 * 			  TYPE_OBJ: send object,
 * 			  TYPE_OBJ_REQ: request object update
 * 			  TYPE_OBJ_ACK: send object with an ack
 * \return 0 Success
 * \return -1 Failure
 */
static int32_t objectTransaction(UAVTalkConnection *connection, UAVObjHandle obj, uint16_t instId, uint8_t type, int32_t timeoutMs)
{
	int32_t respReceived;
	
	// Send object depending on if a response is needed
	if (type == TYPE_OBJ_ACK || type == TYPE_OBJ_REQ)
	{
		// Get transaction lock (will block if a transaction is pending)
		xSemaphoreTakeRecursive(connection->transLock, portMAX_DELAY);
		// Send object
		xSemaphoreTakeRecursive(connection->lock, portMAX_DELAY);
		connection->respObj = obj;
		connection->respInstId = instId;
		sendObject(connection, obj, instId, type);
		xSemaphoreGiveRecursive(connection->lock);
		// Wait for response (or timeout)
		respReceived = xSemaphoreTake(connection->respSema, timeoutMs/portTICK_RATE_MS);
		// Check if a response was received
		if (respReceived == pdFALSE)
		{
			// Cancel transaction
			xSemaphoreTakeRecursive(connection->lock, portMAX_DELAY);
			xSemaphoreTake(connection->respSema, 0); // non blocking call to make sure the value is reset to zero (binary sema)
			connection->respObj = 0;
			xSemaphoreGiveRecursive(connection->lock);
			xSemaphoreGiveRecursive(connection->transLock);
			return -1;
		}
		else
		{
			xSemaphoreGiveRecursive(connection->transLock);
			return 0;
		}
	}
	else if (type == TYPE_OBJ)
	{
		xSemaphoreTakeRecursive(connection->lock, portMAX_DELAY);
		sendObject(connection, obj, instId, TYPE_OBJ);
		xSemaphoreGiveRecursive(connection->lock);
		return 0;
	}
	else
	{
		return -1;
	}
}

/**
 * Process an byte from the telemetry stream.
 * \param[in] connection UAVTalkConnection to be used
 * \param[in] rxbyte Received byte
 * \return 0 Success
 * \return -1 Failure
 */
int32_t UAVTalkProcessInputStream(UAVTalkConnection *connection, uint8_t rxbyte)
{
	UAVTalkInputProcessor *iproc = &connection->iproc;
	++connection->stats.rxBytes;
	
	if (iproc->rxPacketLength < 0xffff)
		iproc->rxPacketLength++;   // update packet byte count
	
	// Receive state machine
	switch (iproc->state)
	{
		case UAVTALK_STATE_SYNC:
			if (rxbyte != SYNC_VAL)
				break;
			
			// Initialize and update the CRC
			iproc->cs = PIOS_CRC_updateByte(0, rxbyte);
			
			iproc->rxPacketLength = 1;
			
			iproc->state = UAVTALK_STATE_TYPE;
			break;
			
		case UAVTALK_STATE_TYPE:
			
			// update the CRC
			iproc->cs = PIOS_CRC_updateByte(iproc->cs, rxbyte);
			
			if ((rxbyte & TYPE_MASK) != TYPE_VER)
			{
				iproc->state = UAVTALK_STATE_SYNC;
				break;
			}
			
			iproc->type = rxbyte;
			
			iproc->packet_size = 0;
			
			iproc->state = UAVTALK_STATE_SIZE;
			iproc->rxCount = 0;
			break;
			
		case UAVTALK_STATE_SIZE:
			
			// update the CRC
			iproc->cs = PIOS_CRC_updateByte(iproc->cs, rxbyte);
			
			if (iproc->rxCount == 0)
			{
				iproc->packet_size += rxbyte;
				iproc->rxCount++;
				break;
			}
			
			iproc->packet_size += rxbyte << 8;
			
			if (iproc->packet_size < UAVTALK_MIN_HEADER_LENGTH || iproc->packet_size > UAVTALK_MAX_HEADER_LENGTH + UAVTALK_MAX_PAYLOAD_LENGTH)
			{   // incorrect packet size
				iproc->state = UAVTALK_STATE_SYNC;
				break;
			}
			
			iproc->rxCount = 0;
			iproc->objId = 0;
			iproc->state = UAVTALK_STATE_OBJID;
			break;
			
		case UAVTALK_STATE_OBJID:
			
			// update the CRC
			iproc->cs = PIOS_CRC_updateByte(iproc->cs, rxbyte);
			
			iproc->objId += rxbyte << (8*(iproc->rxCount++));

			if (iproc->rxCount < 4)
				break;
			
			// Search for object, if not found reset state machine
			// except if we got a OBJ_REQ for an object which does not
			// exist, in which case we'll send a NACK

			iproc->obj = UAVObjGetByID(iproc->objId);
			if (iproc->obj == 0 && iproc->type != TYPE_OBJ_REQ)
			{
				connection->stats.rxErrors++;
				iproc->state = UAVTALK_STATE_SYNC;
				break;
			}
			
			// Determine data length
			if (iproc->type == TYPE_OBJ_REQ || iproc->type == TYPE_ACK || iproc->type == TYPE_NACK)
				iproc->length = 0;
			else
				iproc->length = UAVObjGetNumBytes(iproc->obj);
			
			// Check length and determine next state
			if (iproc->length >= UAVTALK_MAX_PAYLOAD_LENGTH)
			{
				connection->stats.rxErrors++;
				iproc->state = UAVTALK_STATE_SYNC;
				break;
			}
			
			// Check the lengths match
			if ((iproc->rxPacketLength + iproc->length) != iproc->packet_size)
			{   // packet error - mismatched packet size
				connection->stats.rxErrors++;
				iproc->state = UAVTALK_STATE_SYNC;
				break;
			}
			
			iproc->instId = 0;
			if (iproc->obj == 0)
			{
				// If this is a NACK, we skip to Checksum
				iproc->state = UAVTALK_STATE_CS;
				iproc->rxCount = 0;

			}
			// Check if this is a single instance object (i.e. if the instance ID field is coming next)
			else if (UAVObjIsSingleInstance(iproc->obj))
			{
				// If there is a payload get it, otherwise receive checksum
				if (iproc->length > 0)
					iproc->state = UAVTALK_STATE_DATA;
				else
					iproc->state = UAVTALK_STATE_CS;

				iproc->rxCount = 0;
			}
			else
			{
				iproc->state = UAVTALK_STATE_INSTID;
				iproc->rxCount = 0;
			}
			
			break;
			
		case UAVTALK_STATE_INSTID:
			
			// update the CRC
			iproc->cs = PIOS_CRC_updateByte(iproc->cs, rxbyte);
			
			iproc->instId += rxbyte << (8*(iproc->rxCount++));

			if (iproc->rxCount < 2)
				break;
			
			iproc->rxCount = 0;
			
			// If there is a payload get it, otherwise receive checksum
			if (iproc->length > 0)
				iproc->state = UAVTALK_STATE_DATA;
			else
				iproc->state = UAVTALK_STATE_CS;
			
			break;
			
		case UAVTALK_STATE_DATA:
			
			// update the CRC
			iproc->cs = PIOS_CRC_updateByte(iproc->cs, rxbyte);
			
			connection->rxBuffer[iproc->rxCount++] = rxbyte;
			if (iproc->rxCount < iproc->length)
				break;
			
			iproc->state = UAVTALK_STATE_CS;
			iproc->rxCount = 0;
			break;
			
		case UAVTALK_STATE_CS:
			
			// the CRC byte
			if (rxbyte != iproc->cs)
			{   // packet error - faulty CRC
				connection->stats.rxErrors++;
				iproc->state = UAVTALK_STATE_SYNC;
				break;
			}
			
			if (iproc->rxPacketLength != (iproc->packet_size + 1))
			{   // packet error - mismatched packet size
				connection->stats.rxErrors++;
				iproc->state = UAVTALK_STATE_SYNC;
				break;
			}
			
			xSemaphoreTakeRecursive(connection->lock, portMAX_DELAY);
			receiveObject(connection, iproc->type, iproc->objId, iproc->instId, connection->rxBuffer, iproc->length);
			connection->stats.rxObjectBytes += iproc->length;
			connection->stats.rxObjects++;
			xSemaphoreGiveRecursive(connection->lock);
			
			iproc->state = UAVTALK_STATE_SYNC;
			break;
			
		default:
			connection->stats.rxErrors++;
			iproc->state = UAVTALK_STATE_SYNC;
	}
	
	// Done
	return 0;
}

/**
 * Receive an object. This function process objects received through the telemetry stream.
 * \param[in] connection UAVTalkConnection to be used
 * \param[in] type Type of received message (TYPE_OBJ, TYPE_OBJ_REQ, TYPE_OBJ_ACK, TYPE_ACK, TYPE_NACK)
 * \param[in] objId ID of the object to work on
 * \param[in] instId The instance ID of UAVOBJ_ALL_INSTANCES for all instances.
 * \param[in] data Data buffer
 * \param[in] length Buffer length
 * \return 0 Success
 * \return -1 Failure
 */
static int32_t receiveObject(UAVTalkConnection *connection, uint8_t type, uint32_t objId, uint16_t instId, uint8_t* data, int32_t length)
{
	UAVObjHandle obj;
	int32_t ret = 0;

	// Get the handle to the Object. Will be zero
	// if object does not exist.
	obj = UAVObjGetByID(objId);
	
	// Process message type
	switch (type) {
		case TYPE_OBJ:
			// All instances, not allowed for OBJ messages
			if (instId != UAVOBJ_ALL_INSTANCES)
			{
				// Unpack object, if the instance does not exist it will be created!
				UAVObjUnpack(obj, instId, data);
				// Check if an ack is pending
				updateAck(connection, obj, instId);
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
					sendObject(connection, obj, instId, TYPE_ACK);
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
			if (obj == 0)
				sendNack(connection, objId);
			else
				sendObject(connection, obj, instId, TYPE_OBJ);
			break;
		case TYPE_NACK:
			// Do nothing on flight side, let it time out.
			break;
		case TYPE_ACK:
			// All instances, not allowed for ACK messages
			if (instId != UAVOBJ_ALL_INSTANCES)
			{
				// Check if an ack is pending
				updateAck(connection, obj, instId);
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
 * \param[in] connection UAVTalkConnection to be used
 * \param[in] obj Object
 * \param[in] instId The instance ID of UAVOBJ_ALL_INSTANCES for all instances.
 */
static void updateAck(UAVTalkConnection *connection, UAVObjHandle obj, uint16_t instId)
{
	if (connection->respObj == obj && (connection->respInstId == instId || connection->respInstId == UAVOBJ_ALL_INSTANCES))
	{
		xSemaphoreGive(connection->respSema);
		connection->respObj = 0;
	}
}

/**
 * Send an object through the telemetry link.
 * \param[in] connection UAVTalkConnection to be used
 * \param[in] obj Object handle to send
 * \param[in] instId The instance ID or UAVOBJ_ALL_INSTANCES for all instances
 * \param[in] type Transaction type
 * \return 0 Success
 * \return -1 Failure
 */
static int32_t sendObject(UAVTalkConnection *connection, UAVObjHandle obj, uint16_t instId, uint8_t type)
{
	uint32_t numInst;
	uint32_t n;
	
	// If all instances are requested and this is a single instance object, force instance ID to zero
	if ( instId == UAVOBJ_ALL_INSTANCES && UAVObjIsSingleInstance(obj) )
	{
		instId = 0;
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
				sendSingleObject(connection, obj, n, type);
			}
			return 0;
		}
		else
		{
			return sendSingleObject(connection, obj, instId, type);
		}
	}
	else if (type == TYPE_OBJ_REQ)
	{
		return sendSingleObject(connection, obj, instId, TYPE_OBJ_REQ);
	}
	else if (type == TYPE_ACK)
	{
		if ( instId != UAVOBJ_ALL_INSTANCES )
		{
			return sendSingleObject(connection, obj, instId, TYPE_ACK);
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
 * \param[in] connection UAVTalkConnection to be used
 * \param[in] obj Object handle to send
 * \param[in] instId The instance ID (can NOT be UAVOBJ_ALL_INSTANCES, use sendObject() instead)
 * \param[in] type Transaction type
 * \return 0 Success
 * \return -1 Failure
 */
static int32_t sendSingleObject(UAVTalkConnection *connection, UAVObjHandle obj, uint16_t instId, uint8_t type)
{
	int32_t length;
	int32_t dataOffset;
	uint32_t objId;
	
	// Setup type and object id fields
	objId = UAVObjGetID(obj);
	connection->txBuffer[0] = SYNC_VAL;  // sync byte
	connection->txBuffer[1] = type;
	// data length inserted here below
	connection->txBuffer[4] = (uint8_t)(objId & 0xFF);
	connection->txBuffer[5] = (uint8_t)((objId >> 8) & 0xFF);
	connection->txBuffer[6] = (uint8_t)((objId >> 16) & 0xFF);
	connection->txBuffer[7] = (uint8_t)((objId >> 24) & 0xFF);
	
	// Setup instance ID if one is required
	if (UAVObjIsSingleInstance(obj))
	{
		dataOffset = 8;
	}
	else
	{
		connection->txBuffer[8] = (uint8_t)(instId & 0xFF);
		connection->txBuffer[9] = (uint8_t)((instId >> 8) & 0xFF);
		dataOffset = 10;
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
	if (length >= UAVTALK_MAX_PAYLOAD_LENGTH)
	{
		return -1;
	}
	
	// Copy data (if any)
	if (length > 0)
	{
		if ( UAVObjPack(obj, instId, &connection->txBuffer[dataOffset]) < 0 )
		{
			return -1;
		}
	}
	
	// Store the packet length
	connection->txBuffer[2] = (uint8_t)((dataOffset+length) & 0xFF);
	connection->txBuffer[3] = (uint8_t)(((dataOffset+length) >> 8) & 0xFF);
	
	// Calculate checksum
	connection->txBuffer[dataOffset+length] = PIOS_CRC_updateCRC(0, connection->txBuffer, dataOffset+length);
	
	// Send buffer
	if (connection->outStream!=NULL) (*connection->outStream)(connection->txBuffer, dataOffset+length+UAVTALK_CHECKSUM_LENGTH);
	
	// Update stats
	++connection->stats.txObjects;
	connection->stats.txBytes += dataOffset+length+UAVTALK_CHECKSUM_LENGTH;
	connection->stats.txObjectBytes += length;
	
	// Done
	return 0;
}

/**
 * Send a NACK through the telemetry link.
 * \param[in] connection UAVTalkConnection to be used
 * \param[in] objId Object ID to send a NACK for
 * \return 0 Success
 * \return -1 Failure
 */
static int32_t sendNack(UAVTalkConnection *connection, uint32_t objId)
{
	int32_t dataOffset;

	connection->txBuffer[0] = SYNC_VAL;  // sync byte
	connection->txBuffer[1] = TYPE_NACK;
	// data length inserted here below
	connection->txBuffer[4] = (uint8_t)(objId & 0xFF);
	connection->txBuffer[5] = (uint8_t)((objId >> 8) & 0xFF);
	connection->txBuffer[6] = (uint8_t)((objId >> 16) & 0xFF);
	connection->txBuffer[7] = (uint8_t)((objId >> 24) & 0xFF);

	dataOffset = 8;

	// Store the packet length
	connection->txBuffer[2] = (uint8_t)((dataOffset) & 0xFF);
	connection->txBuffer[3] = (uint8_t)(((dataOffset) >> 8) & 0xFF);

	// Calculate checksum
	connection->txBuffer[dataOffset] = PIOS_CRC_updateCRC(0, connection->txBuffer, dataOffset);

	// Send buffer
	if (connection->outStream!=NULL) (*connection->outStream)(connection->txBuffer, dataOffset+UAVTALK_CHECKSUM_LENGTH);

	// Update stats
	connection->stats.txBytes += dataOffset+UAVTALK_CHECKSUM_LENGTH;

	// Done
	return 0;
}

/**
 * @}
 * @}
 */
