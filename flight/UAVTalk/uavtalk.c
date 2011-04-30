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

#define MIN_HEADER_LENGTH	8	// sync(1), type (1), size (2), object ID (4)
#define MAX_HEADER_LENGTH	10	// sync(1), type (1), size (2), object ID (4), instance ID (2, not used in single objects)

#define CHECKSUM_LENGTH		1

#define MAX_PAYLOAD_LENGTH	256

#define MAX_PACKET_LENGTH	(MAX_HEADER_LENGTH + MAX_PAYLOAD_LENGTH + CHECKSUM_LENGTH)

// CRC lookup table
static const uint8_t crc_table[256] = {
	0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d,
	0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65, 0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d,
	0xe0, 0xe7, 0xee, 0xe9, 0xfc, 0xfb, 0xf2, 0xf5, 0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
	0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85, 0xa8, 0xaf, 0xa6, 0xa1, 0xb4, 0xb3, 0xba, 0xbd,
	0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2, 0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea,
	0xb7, 0xb0, 0xb9, 0xbe, 0xab, 0xac, 0xa5, 0xa2, 0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
	0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32, 0x1f, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0d, 0x0a,
	0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42, 0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a,
	0x89, 0x8e, 0x87, 0x80, 0x95, 0x92, 0x9b, 0x9c, 0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
	0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec, 0xc1, 0xc6, 0xcf, 0xc8, 0xdd, 0xda, 0xd3, 0xd4,
	0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c, 0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44,
	0x19, 0x1e, 0x17, 0x10, 0x05, 0x02, 0x0b, 0x0c, 0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
	0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b, 0x76, 0x71, 0x78, 0x7f, 0x6a, 0x6d, 0x64, 0x63,
	0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b, 0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13,
	0xae, 0xa9, 0xa0, 0xa7, 0xb2, 0xb5, 0xbc, 0xbb, 0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8d, 0x84, 0x83,
	0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb, 0xe6, 0xe1, 0xe8, 0xef, 0xfa, 0xfd, 0xf4, 0xf3
};

// Private types
typedef enum {STATE_SYNC, STATE_TYPE, STATE_SIZE, STATE_OBJID, STATE_INSTID, STATE_DATA, STATE_CS} RxState;

// Private variables
static UAVTalkOutputStream outStream;
static xSemaphoreHandle lock;
static xSemaphoreHandle transLock;
static xSemaphoreHandle respSema;
static UAVObjHandle respObj;
static uint16_t respInstId;
static uint8_t rxBuffer[MAX_PACKET_LENGTH];
static uint8_t txBuffer[MAX_PACKET_LENGTH];
static UAVTalkStats stats;

// Private functions
static uint8_t updateCRCbyte(uint8_t crc, const uint8_t data);
static uint8_t updateCRC(uint8_t crc, const uint8_t* data, int32_t length);
static int32_t objectTransaction(UAVObjHandle objectId, uint16_t instId, uint8_t type, int32_t timeout);
static int32_t sendObject(UAVObjHandle obj, uint16_t instId, uint8_t type);
static int32_t sendSingleObject(UAVObjHandle obj, uint16_t instId, uint8_t type);
static int32_t sendNack(uint32_t objId);
static int32_t receiveObject(uint8_t type, uint32_t objId, uint16_t instId, uint8_t* data, int32_t length);
static void updateAck(UAVObjHandle obj, uint16_t instId);

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
	transLock = xSemaphoreCreateRecursiveMutex();
	vSemaphoreCreateBinary(respSema);
	xSemaphoreTake(respSema, 0); // reset to zero
	UAVTalkResetStats();
	return 0;
}

/**
 * Get communication statistics counters
 * @param[out] statsOut Statistics counters
 */
void UAVTalkGetStats(UAVTalkStats* statsOut)
{
	// Lock
	xSemaphoreTakeRecursive(lock, portMAX_DELAY);
	
	// Copy stats
	memcpy(statsOut, &stats, sizeof(UAVTalkStats));
	
	// Release lock
	xSemaphoreGiveRecursive(lock);
}

/**
 * Reset the statistics counters.
 */
void UAVTalkResetStats()
{
	// Lock
	xSemaphoreTakeRecursive(lock, portMAX_DELAY);
	
	// Clear stats
	memset(&stats, 0, sizeof(UAVTalkStats));
	
	// Release lock
	xSemaphoreGiveRecursive(lock);
}

/**
 * Request an update for the specified object, on success the object data would have been
 * updated by the GCS.
 * \param[in] obj Object to update
 * \param[in] instId The instance ID or UAVOBJ_ALL_INSTANCES for all instances.
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
 * \param[in] instId The instance ID or UAVOBJ_ALL_INSTANCES for all instances.
 * \param[in] acked Selects if an ack is required (1:ack required, 0: ack not required)
 * \param[in] timeoutMs Time to wait for the ack, when zero it will return immediately
 * \return 0 Success
 * \return -1 Failure
 */
int32_t UAVTalkSendObject(UAVObjHandle obj, uint16_t instId, uint8_t acked, int32_t timeoutMs)
{
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
static int32_t objectTransaction(UAVObjHandle obj, uint16_t instId, uint8_t type, int32_t timeoutMs)
{
	int32_t respReceived;
	
	// Send object depending on if a response is needed
	if (type == TYPE_OBJ_ACK || type == TYPE_OBJ_REQ)
	{
		// Get transaction lock (will block if a transaction is pending)
		xSemaphoreTakeRecursive(transLock, portMAX_DELAY);
		// Send object
		xSemaphoreTakeRecursive(lock, portMAX_DELAY);
		respObj = obj;
		respInstId = instId;
		sendObject(obj, instId, type);
		xSemaphoreGiveRecursive(lock);
		// Wait for response (or timeout)
		respReceived = xSemaphoreTake(respSema, timeoutMs/portTICK_RATE_MS);
		// Check if a response was received
		if (respReceived == pdFALSE)
		{
			// Cancel transaction
			xSemaphoreTakeRecursive(lock, portMAX_DELAY);
			xSemaphoreTake(respSema, 0); // non blocking call to make sure the value is reset to zero (binary sema)
			respObj = 0;
			xSemaphoreGiveRecursive(lock);
			xSemaphoreGiveRecursive(transLock);
			return -1;
		}
		else
		{
			xSemaphoreGiveRecursive(transLock);
			return 0;
		}
	}
	else if (type == TYPE_OBJ)
	{
		xSemaphoreTakeRecursive(lock, portMAX_DELAY);
		sendObject(obj, instId, TYPE_OBJ);
		xSemaphoreGiveRecursive(lock);
		return 0;
	}
	else
	{
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
	static UAVObjHandle obj;
	static uint8_t type;
	static uint16_t packet_size;
	static uint32_t objId;
	static uint16_t instId;
	static uint32_t length;
	static uint8_t cs, csRx;
	static int32_t rxCount;
	static RxState state = STATE_SYNC;
	static uint16_t rxPacketLength = 0;
	
	++stats.rxBytes;
	
	if (rxPacketLength < 0xffff)
		rxPacketLength++;   // update packet byte count
	
	// Receive state machine
	switch (state)
	{
		case STATE_SYNC:
			if (rxbyte != SYNC_VAL)
				break;
			
			// Initialize and update the CRC
			cs = updateCRCbyte(0, rxbyte);
			
			rxPacketLength = 1;
			
			state = STATE_TYPE;
			break;
			
		case STATE_TYPE:
			
			// update the CRC
			cs = updateCRCbyte(cs, rxbyte);
			
			if ((rxbyte & TYPE_MASK) != TYPE_VER)
			{
				state = STATE_SYNC;
				break;
			}
			
			type = rxbyte;
			
			packet_size = 0;
			
			state = STATE_SIZE;
			rxCount = 0;
			break;
			
		case STATE_SIZE:
			
			// update the CRC
			cs = updateCRCbyte(cs, rxbyte);
			
			if (rxCount == 0)
			{
				packet_size += rxbyte;
				rxCount++;
				break;
			}
			
			packet_size += rxbyte << 8;
			
			if (packet_size < MIN_HEADER_LENGTH || packet_size > MAX_HEADER_LENGTH + MAX_PAYLOAD_LENGTH)
			{   // incorrect packet size
				state = STATE_SYNC;
				break;
			}
			
			rxCount = 0;
			objId = 0;
			state = STATE_OBJID;
			break;
			
		case STATE_OBJID:
			
			// update the CRC
			cs = updateCRCbyte(cs, rxbyte);
			
			objId += rxbyte << (8*(rxCount++));

			if (rxCount < 4)
				break;
			
			// Search for object, if not found reset state machine
			// except if we got a OBJ_REQ for an object which does not
			// exist, in which case we'll send a NACK

			obj = UAVObjGetByID(objId);
			if (obj == 0 && type != TYPE_OBJ_REQ)
			{
				stats.rxErrors++;
				state = STATE_SYNC;
				break;
			}
			
			// Determine data length
			if (type == TYPE_OBJ_REQ || type == TYPE_ACK || type == TYPE_NACK)
				length = 0;
			else
				length = UAVObjGetNumBytes(obj);
			
			// Check length and determine next state
			if (length >= MAX_PAYLOAD_LENGTH)
			{
				stats.rxErrors++;
				state = STATE_SYNC;
				break;
			}
			
			// Check the lengths match
			if ((rxPacketLength + length) != packet_size)
			{   // packet error - mismatched packet size
				stats.rxErrors++;
				state = STATE_SYNC;
				break;
			}
			
			instId = 0;
			if (obj == 0)
			{
				// If this is a NACK, we skip to Checksum
				state = STATE_CS;
				rxCount = 0;

			}
			// Check if this is a single instance object (i.e. if the instance ID field is coming next)
			else if (UAVObjIsSingleInstance(obj))
			{
				// If there is a payload get it, otherwise receive checksum
				if (length > 0)
					state = STATE_DATA;
				else
					state = STATE_CS;

				rxCount = 0;
			}
			else
			{
				state = STATE_INSTID;
				rxCount = 0;
			}
			
			break;
			
		case STATE_INSTID:
			
			// update the CRC
			cs = updateCRCbyte(cs, rxbyte);
			
			instId += rxbyte << (8*(rxCount++));

			if (rxCount < 2)
				break;
			
			rxCount = 0;
			
			// If there is a payload get it, otherwise receive checksum
			if (length > 0)
				state = STATE_DATA;
			else
				state = STATE_CS;
			
			break;
			
		case STATE_DATA:
			
			// update the CRC
			cs = updateCRCbyte(cs, rxbyte);
			
			rxBuffer[rxCount++] = rxbyte;
			if (rxCount < length)
				break;
			
			state = STATE_CS;
			rxCount = 0;
			break;
			
		case STATE_CS:
			
			// the CRC byte
			csRx = rxbyte;
			
			if (csRx != cs)
			{   // packet error - faulty CRC
				stats.rxErrors++;
				state = STATE_SYNC;
				break;
			}
			
			if (rxPacketLength != (packet_size + 1))
			{   // packet error - mismatched packet size
				stats.rxErrors++;
				state = STATE_SYNC;
				break;
			}
			
			xSemaphoreTakeRecursive(lock, portMAX_DELAY);
			receiveObject(type, objId, instId, rxBuffer, length);
			stats.rxObjectBytes += length;
			stats.rxObjects++;
			xSemaphoreGiveRecursive(lock);
			
			state = STATE_SYNC;
			break;
			
		default:
			stats.rxErrors++;
			state = STATE_SYNC;
	}
	
	// Done
	return 0;
}

/**
 * Receive an object. This function process objects received through the telemetry stream.
 * \param[in] type Type of received message (TYPE_OBJ, TYPE_OBJ_REQ, TYPE_OBJ_ACK, TYPE_ACK, TYPE_NACK)
 * \param[in] objId ID of the object to work on
 * \param[in] instId The instance ID of UAVOBJ_ALL_INSTANCES for all instances.
 * \param[in] data Data buffer
 * \param[in] length Buffer length
 * \return 0 Success
 * \return -1 Failure
 */
static int32_t receiveObject(uint8_t type, uint32_t objId, uint16_t instId, uint8_t* data, int32_t length)
{
	static UAVObjHandle obj;
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
			if (obj == 0)
				sendNack(objId);
			else
				sendObject(obj, instId, TYPE_OBJ);
			break;
		case TYPE_NACK:
			// Do nothing on flight side, let it time out.
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
static void updateAck(UAVObjHandle obj, uint16_t instId)
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
static int32_t sendObject(UAVObjHandle obj, uint16_t instId, uint8_t type)
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
static int32_t sendSingleObject(UAVObjHandle obj, uint16_t instId, uint8_t type)
{
	int32_t length;
	int32_t dataOffset;
	uint32_t objId;
	
	// Setup type and object id fields
	objId = UAVObjGetID(obj);
	txBuffer[0] = SYNC_VAL;  // sync byte
	txBuffer[1] = type;
	// data length inserted here below
	txBuffer[4] = (uint8_t)(objId & 0xFF);
	txBuffer[5] = (uint8_t)((objId >> 8) & 0xFF);
	txBuffer[6] = (uint8_t)((objId >> 16) & 0xFF);
	txBuffer[7] = (uint8_t)((objId >> 24) & 0xFF);
	
	// Setup instance ID if one is required
	if (UAVObjIsSingleInstance(obj))
	{
		dataOffset = 8;
	}
	else
	{
		txBuffer[8] = (uint8_t)(instId & 0xFF);
		txBuffer[9] = (uint8_t)((instId >> 8) & 0xFF);
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
	
	// Store the packet length
	txBuffer[2] = (uint8_t)((dataOffset+length) & 0xFF);
	txBuffer[3] = (uint8_t)(((dataOffset+length) >> 8) & 0xFF);
	
	// Calculate checksum
	txBuffer[dataOffset+length] = updateCRC(0, txBuffer, dataOffset+length);
	
	// Send buffer
	if (outStream!=NULL) (*outStream)(txBuffer, dataOffset+length+CHECKSUM_LENGTH);
	
	// Update stats
	++stats.txObjects;
	stats.txBytes += dataOffset+length+CHECKSUM_LENGTH;
	stats.txObjectBytes += length;
	
	// Done
	return 0;
}

/**
 * Send a NACK through the telemetry link.
 * \param[in] objId Object ID to send a NACK for
 * \return 0 Success
 * \return -1 Failure
 */
static int32_t sendNack(uint32_t objId)
{
	int32_t dataOffset;

	txBuffer[0] = SYNC_VAL;  // sync byte
	txBuffer[1] = TYPE_NACK;
	// data length inserted here below
	txBuffer[4] = (uint8_t)(objId & 0xFF);
	txBuffer[5] = (uint8_t)((objId >> 8) & 0xFF);
	txBuffer[6] = (uint8_t)((objId >> 16) & 0xFF);
	txBuffer[7] = (uint8_t)((objId >> 24) & 0xFF);

	dataOffset = 8;

	// Store the packet length
	txBuffer[2] = (uint8_t)((dataOffset) & 0xFF);
	txBuffer[3] = (uint8_t)(((dataOffset) >> 8) & 0xFF);

	// Calculate checksum
	txBuffer[dataOffset] = updateCRC(0, txBuffer, dataOffset);

	// Send buffer
	if (outStream!=NULL) (*outStream)(txBuffer, dataOffset+CHECKSUM_LENGTH);

	// Update stats
	stats.txBytes += dataOffset+CHECKSUM_LENGTH;

	// Done
	return 0;
}


/**
 * Update the crc value with new data.
 *
 * Generated by pycrc v0.7.5, http://www.tty1.net/pycrc/
 * using the configuration:
 *    Width        = 8
 *    Poly         = 0x07
 *    XorIn        = 0x00
 *    ReflectIn    = False
 *    XorOut       = 0x00
 *    ReflectOut   = False
 *    Algorithm    = table-driven
 *
 * \param crc      The current crc value.
 * \param data     Pointer to a buffer of \a data_len bytes.
 * \param length   Number of bytes in the \a data buffer.
 * \return         The updated crc value.
 */
static uint8_t updateCRCbyte(uint8_t crc, const uint8_t data)
{
	return crc_table[crc ^ data];
}
static uint8_t updateCRC(uint8_t crc, const uint8_t* data, int32_t length)
{
	// use registers for speed
	register int32_t len = length;
	register uint8_t crc8 = crc;
	register const uint8_t *p = data;
	
	while (len--)
		crc8 = crc_table[crc8 ^ *p++];
	
	return crc8;
}

/**
 * @}
 * @}
 */
