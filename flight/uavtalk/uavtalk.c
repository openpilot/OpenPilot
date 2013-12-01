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
 *             This library should not be called directly by the application, it is only used by the
 *             Telemetry module.
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
#include "uavtalk_priv.h"

// Size of instance ID (2 bytes)
#define UAVTALK_INSTANCE_LENGTH 2

// Private functions
static int32_t objectTransaction(UAVTalkConnectionData *connection, uint8_t type, UAVObjHandle obj, uint16_t instId, int32_t timeout);
static int32_t sendObject(UAVTalkConnectionData *connection, uint8_t type, uint32_t objId, uint16_t instId, UAVObjHandle obj);
static int32_t sendSingleObject(UAVTalkConnectionData *connection, uint8_t type, uint32_t objId, uint16_t instId, UAVObjHandle obj);
static int32_t receiveObject(UAVTalkConnectionData *connection, uint8_t type, uint32_t objId, uint16_t instId, uint8_t *data, int32_t length);
static void updateAck(UAVTalkConnectionData *connection, uint8_t type, uint32_t objId, uint16_t instId);

/**
 * Initialize the UAVTalk library
 * \param[in] connection UAVTalkConnection to be used
 * \param[in] outputStream Function pointer that is called to send a data buffer
 * \return 0 Success
 * \return -1 Failure
 */
UAVTalkConnection UAVTalkInitialize(UAVTalkOutputStream outputStream)
{
    // allocate object
    UAVTalkConnectionData *connection = pvPortMalloc(sizeof(UAVTalkConnectionData));

    if (!connection) {
        return 0;
    }
    connection->canari      = UAVTALK_CANARI;
    connection->iproc.rxPacketLength = 0;
    connection->iproc.state = UAVTALK_STATE_SYNC;
    connection->outStream   = outputStream;
    connection->lock = xSemaphoreCreateRecursiveMutex();
    connection->transLock   = xSemaphoreCreateRecursiveMutex();
    // allocate buffers
    connection->rxBuffer    = pvPortMalloc(UAVTALK_MAX_PACKET_LENGTH);
    if (!connection->rxBuffer) {
        return 0;
    }
    connection->txBuffer = pvPortMalloc(UAVTALK_MAX_PACKET_LENGTH);
    if (!connection->txBuffer) {
        return 0;
    }
    vSemaphoreCreateBinary(connection->respSema);
    xSemaphoreTake(connection->respSema, 0); // reset to zero
    UAVTalkResetStats((UAVTalkConnection)connection);
    return (UAVTalkConnection)connection;
}

/**
 * Set the communication output stream
 * \param[in] connection UAVTalkConnection to be used
 * \param[in] outputStream Function pointer that is called to send a data buffer
 * \return 0 Success
 * \return -1 Failure
 */
int32_t UAVTalkSetOutputStream(UAVTalkConnection connectionHandle, UAVTalkOutputStream outputStream)
{
    UAVTalkConnectionData *connection;

    CHECKCONHANDLE(connectionHandle, connection, return -1);

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
UAVTalkOutputStream UAVTalkGetOutputStream(UAVTalkConnection connectionHandle)
{
    UAVTalkConnectionData *connection;

    CHECKCONHANDLE(connectionHandle, connection, return NULL);
    return connection->outStream;
}

/**
 * Get communication statistics counters
 * \param[in] connection UAVTalkConnection to be used
 * @param[out] statsOut Statistics counters
 */
void UAVTalkGetStats(UAVTalkConnection connectionHandle, UAVTalkStats *statsOut)
{
    UAVTalkConnectionData *connection;

    CHECKCONHANDLE(connectionHandle, connection, return );

    // Lock
    xSemaphoreTakeRecursive(connection->lock, portMAX_DELAY);

    // Copy stats
    memcpy(statsOut, &connection->stats, sizeof(UAVTalkStats));

    // Release lock
    xSemaphoreGiveRecursive(connection->lock);
}

/**
 * Get communication statistics counters
 * \param[in] connection UAVTalkConnection to be used
 * @param[out] statsOut Statistics counters
 */
void UAVTalkAddStats(UAVTalkConnection connectionHandle, UAVTalkStats *statsOut)
{
    UAVTalkConnectionData *connection;

    CHECKCONHANDLE(connectionHandle, connection, return );

    // Lock
    xSemaphoreTakeRecursive(connection->lock, portMAX_DELAY);

    // Copy stats
    statsOut->txBytes       += connection->stats.txBytes;
    statsOut->rxBytes       += connection->stats.rxBytes;
    statsOut->txObjectBytes += connection->stats.txObjectBytes;
    statsOut->rxObjectBytes += connection->stats.rxObjectBytes;
    statsOut->txObjects     += connection->stats.txObjects;
    statsOut->rxObjects     += connection->stats.rxObjects;
    statsOut->txErrors      += connection->stats.txErrors;
    statsOut->txErrors      += connection->stats.txErrors;

    // Release lock
    xSemaphoreGiveRecursive(connection->lock);
}

/**
 * Reset the statistics counters.
 * \param[in] connection UAVTalkConnection to be used
 */
void UAVTalkResetStats(UAVTalkConnection connectionHandle)
{
    UAVTalkConnectionData *connection;

    CHECKCONHANDLE(connectionHandle, connection, return );

    // Lock
    xSemaphoreTakeRecursive(connection->lock, portMAX_DELAY);

    // Clear stats
    memset(&connection->stats, 0, sizeof(UAVTalkStats));

    // Release lock
    xSemaphoreGiveRecursive(connection->lock);
}

/**
 * Accessor method to get the timestamp from the last UAVTalk message
 */
void UAVTalkGetLastTimestamp(UAVTalkConnection connectionHandle, uint16_t *timestamp)
{
    UAVTalkConnectionData *connection;

    CHECKCONHANDLE(connectionHandle, connection, return );

    UAVTalkInputProcessor *iproc = &connection->iproc;
    *timestamp = iproc->timestamp;
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
int32_t UAVTalkSendObjectRequest(UAVTalkConnection connectionHandle, UAVObjHandle obj, uint16_t instId, int32_t timeout)
{
    UAVTalkConnectionData *connection;

    CHECKCONHANDLE(connectionHandle, connection, return -1);

    return objectTransaction(connection, UAVTALK_TYPE_OBJ_REQ, obj, instId, timeout);
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
int32_t UAVTalkSendObject(UAVTalkConnection connectionHandle, UAVObjHandle obj, uint16_t instId, uint8_t acked, int32_t timeoutMs)
{
    UAVTalkConnectionData *connection;

    CHECKCONHANDLE(connectionHandle, connection, return -1);

    // Send object
    if (acked == 1) {
        return objectTransaction(connection, UAVTALK_TYPE_OBJ_ACK, obj, instId, timeoutMs);
    } else {
        return objectTransaction(connection, UAVTALK_TYPE_OBJ, obj, instId, timeoutMs);
    }
}

/**
 * Send the specified object through the telemetry link with a timestamp.
 * \param[in] connection UAVTalkConnection to be used
 * \param[in] obj Object to send
 * \param[in] instId The instance ID or UAVOBJ_ALL_INSTANCES for all instances.
 * \param[in] acked Selects if an ack is required (1:ack required, 0: ack not required)
 * \param[in] timeoutMs Time to wait for the ack, when zero it will return immediately
 * \return 0 Success
 * \return -1 Failure
 */
int32_t UAVTalkSendObjectTimestamped(UAVTalkConnection connectionHandle, UAVObjHandle obj, uint16_t instId, uint8_t acked, int32_t timeoutMs)
{
    UAVTalkConnectionData *connection;

    CHECKCONHANDLE(connectionHandle, connection, return -1);

    // Send object
    if (acked == 1) {
        return objectTransaction(connection, UAVTALK_TYPE_OBJ_ACK_TS, obj, instId, timeoutMs);
    } else {
        return objectTransaction(connection, UAVTALK_TYPE_OBJ_TS, obj, instId, timeoutMs);
    }
}

/**
 * Execute the requested transaction on an object.
 * \param[in] connection UAVTalkConnection to be used
 * \param[in] type Transaction type
 *                        UAVTALK_TYPE_OBJ: send object,
 *                        UAVTALK_TYPE_OBJ_REQ: request object update
 *                        UAVTALK_TYPE_OBJ_ACK: send object with an ack
 * \param[in] obj Object
 * \param[in] instId The instance ID of UAVOBJ_ALL_INSTANCES for all instances.
 * \param[in] timeoutMs Time to wait for the ack, when zero it will return immediately
 * \return 0 Success
 * \return -1 Failure
 */
static int32_t objectTransaction(UAVTalkConnectionData *connection, uint8_t type, UAVObjHandle obj, uint16_t instId, int32_t timeoutMs)
{
    int32_t respReceived;

    // Send object depending on if a response is needed
    if (type == UAVTALK_TYPE_OBJ_ACK || type == UAVTALK_TYPE_OBJ_ACK_TS || type == UAVTALK_TYPE_OBJ_REQ) {
        // Get transaction lock (will block if a transaction is pending)
        xSemaphoreTakeRecursive(connection->transLock, portMAX_DELAY);
        // Send object
        xSemaphoreTakeRecursive(connection->lock, portMAX_DELAY);
        // expected response type
        connection->respType   = (type == UAVTALK_TYPE_OBJ_REQ) ? UAVTALK_TYPE_OBJ : UAVTALK_TYPE_ACK;
        connection->respObjId  =  UAVObjGetID(obj);
        connection->respInstId = instId;
        sendObject(connection, type, UAVObjGetID(obj), instId, obj);
        xSemaphoreGiveRecursive(connection->lock);
        // Wait for response (or timeout)
        respReceived = xSemaphoreTake(connection->respSema, timeoutMs / portTICK_RATE_MS);
        // Check if a response was received
        if (respReceived == pdFALSE) {
            // Cancel transaction
            xSemaphoreTakeRecursive(connection->lock, portMAX_DELAY);
            // non blocking call to make sure the value is reset to zero (binary sema)
            xSemaphoreTake(connection->respSema, 0);
            connection->respObjId = 0;
            xSemaphoreGiveRecursive(connection->lock);
            xSemaphoreGiveRecursive(connection->transLock);
            return -1;
        } else {
            xSemaphoreGiveRecursive(connection->transLock);
            return 0;
        }
    } else if (type == UAVTALK_TYPE_OBJ || type == UAVTALK_TYPE_OBJ_TS) {
        xSemaphoreTakeRecursive(connection->lock, portMAX_DELAY);
        sendObject(connection, type, UAVObjGetID(obj), instId, obj);
        xSemaphoreGiveRecursive(connection->lock);
        return 0;
    } else {
        return -1;
    }
}

/**
 * Process an byte from the telemetry stream.
 * \param[in] connectionHandle UAVTalkConnection to be used
 * \param[in] rxbyte Received byte
 * \return UAVTalkRxState
 */
UAVTalkRxState UAVTalkProcessInputStreamQuiet(UAVTalkConnection connectionHandle, uint8_t rxbyte)
{
    UAVTalkConnectionData *connection;

    CHECKCONHANDLE(connectionHandle, connection, return -1);

    UAVTalkInputProcessor *iproc = &connection->iproc;

    ++connection->stats.rxBytes;

    if (iproc->state == UAVTALK_STATE_ERROR || iproc->state == UAVTALK_STATE_COMPLETE) {
        iproc->state = UAVTALK_STATE_SYNC;
    }

    if (iproc->rxPacketLength < 0xffff) {
        // update packet byte count
        iproc->rxPacketLength++;
    }

    // Receive state machine
    switch (iproc->state) {
    case UAVTALK_STATE_SYNC:

        if (rxbyte != UAVTALK_SYNC_VAL) {
            break;
        }

        // Initialize and update the CRC
        iproc->cs = PIOS_CRC_updateByte(0, rxbyte);

        iproc->rxPacketLength = 1;

        iproc->state = UAVTALK_STATE_TYPE;
        break;

    case UAVTALK_STATE_TYPE:

        // update the CRC
        iproc->cs = PIOS_CRC_updateByte(iproc->cs, rxbyte);

        if ((rxbyte & UAVTALK_TYPE_MASK) != UAVTALK_TYPE_VER) {
            iproc->state = UAVTALK_STATE_ERROR;
            break;
        }

        iproc->type = rxbyte;

        iproc->packet_size = 0;

        iproc->state = UAVTALK_STATE_SIZE;
        iproc->rxCount     = 0;
        break;

    case UAVTALK_STATE_SIZE:

        // update the CRC
        iproc->cs = PIOS_CRC_updateByte(iproc->cs, rxbyte);

        if (iproc->rxCount == 0) {
            iproc->packet_size += rxbyte;
            iproc->rxCount++;
            break;
        }

        iproc->packet_size += rxbyte << 8;

        if (iproc->packet_size < UAVTALK_MIN_HEADER_LENGTH || iproc->packet_size > UAVTALK_MAX_HEADER_LENGTH + UAVTALK_MAX_PAYLOAD_LENGTH) { // incorrect packet size
            iproc->state = UAVTALK_STATE_ERROR;
            break;
        }

        iproc->rxCount = 0;
        iproc->objId   = 0;
        iproc->state   = UAVTALK_STATE_OBJID;
        break;

    case UAVTALK_STATE_OBJID:

        // update the CRC
        iproc->cs     = PIOS_CRC_updateByte(iproc->cs, rxbyte);

        iproc->objId += rxbyte << (8 * (iproc->rxCount++));

        if (iproc->rxCount < 4) {
            break;
        }

        // Search for object.
        iproc->obj = UAVObjGetByID(iproc->objId);

        // Determine data length
        if (iproc->type == UAVTALK_TYPE_OBJ_REQ || iproc->type == UAVTALK_TYPE_ACK || iproc->type == UAVTALK_TYPE_NACK) {
            iproc->length = 0;
        } else {
            if (iproc->obj) {
                iproc->length = UAVObjGetNumBytes(iproc->obj);
            } else {
                iproc->length = iproc->packet_size - iproc->rxPacketLength;
            }
            iproc->timestampLength = (iproc->type & UAVTALK_TIMESTAMPED) ? 2 : 0;
        }

        // Check length and determine next state
        if (iproc->length >= UAVTALK_MAX_PAYLOAD_LENGTH) {
            // packet error - exceeded payload max length
            connection->stats.rxErrors++;
            iproc->state = UAVTALK_STATE_ERROR;
            break;
        }

        // Check the lengths match
        if ((iproc->rxPacketLength + UAVTALK_INSTANCE_LENGTH + iproc->timestampLength + iproc->length) != iproc->packet_size) {
            // packet error - mismatched packet size
            connection->stats.rxErrors++;
            iproc->state = UAVTALK_STATE_ERROR;
            break;
        }

        // Message always contain an instance ID
        iproc->rxCount = 0;
        iproc->instId = 0;
        iproc->state = UAVTALK_STATE_INSTID;
        break;

    case UAVTALK_STATE_INSTID:

        // update the CRC
        iproc->cs      = PIOS_CRC_updateByte(iproc->cs, rxbyte);

        iproc->instId += rxbyte << (8 * (iproc->rxCount++));

        if (iproc->rxCount < 2) {
            break;
        }

        iproc->rxCount = 0;

        // If there is a timestamp, get it
        if ((iproc->length > 0) && (iproc->type & UAVTALK_TIMESTAMPED)) {
            iproc->timestamp = 0;
            iproc->state     = UAVTALK_STATE_TIMESTAMP;
        }
        // If there is a payload get it, otherwise receive checksum
        else if (iproc->length > 0) {
            iproc->state = UAVTALK_STATE_DATA;
        } else {
            iproc->state = UAVTALK_STATE_CS;
        }
        break;

    case UAVTALK_STATE_TIMESTAMP:
        // update the CRC
        iproc->cs = PIOS_CRC_updateByte(iproc->cs, rxbyte);

        iproc->timestamp += rxbyte << (8 * (iproc->rxCount++));

        if (iproc->rxCount < 2) {
            break;
        }

        iproc->rxCount = 0;

        // If there is a payload get it, otherwise receive checksum
        if (iproc->length > 0) {
            iproc->state = UAVTALK_STATE_DATA;
        } else {
            iproc->state = UAVTALK_STATE_CS;
        }
        break;

    case UAVTALK_STATE_DATA:

        // update the CRC
        iproc->cs = PIOS_CRC_updateByte(iproc->cs, rxbyte);

        connection->rxBuffer[iproc->rxCount++] = rxbyte;
        if (iproc->rxCount < iproc->length) {
            break;
        }

        iproc->state   = UAVTALK_STATE_CS;
        iproc->rxCount = 0;
        break;

    case UAVTALK_STATE_CS:

        // the CRC byte
        if (rxbyte != iproc->cs) { // packet error - faulty CRC
            connection->stats.rxErrors++;
            iproc->state = UAVTALK_STATE_ERROR;
            break;
        }

        if (iproc->rxPacketLength != (iproc->packet_size + 1)) { // packet error - mismatched packet size
            connection->stats.rxErrors++;
            iproc->state = UAVTALK_STATE_ERROR;
            break;
        }

        connection->stats.rxObjectBytes += iproc->length;
        connection->stats.rxObjects++;

        iproc->state = UAVTALK_STATE_COMPLETE;
        break;

    default:
        connection->stats.rxErrors++;
        iproc->state = UAVTALK_STATE_ERROR;
    }

    // Done
    return iproc->state;
}

/**
 * Process an byte from the telemetry stream.
 * \param[in] connection UAVTalkConnection to be used
 * \param[in] rxbyte Received byte
 * \return UAVTalkRxState
 */
UAVTalkRxState UAVTalkProcessInputStream(UAVTalkConnection connectionHandle, uint8_t rxbyte)
{
    UAVTalkRxState state = UAVTalkProcessInputStreamQuiet(connectionHandle, rxbyte);

    if (state == UAVTALK_STATE_COMPLETE) {
        UAVTalkReceiveObject(connectionHandle);
    }

    return state;
}

/**
 * Send a parsed packet received on one connection handle out on a different connection handle.
 * The packet must be in a complete state, meaning it is completed parsing.
 * The packet is re-assembled from the component parts into a complete message and sent.
 * This can be used to relay packets from one UAVTalk connection to another.
 * \param[in] connection UAVTalkConnection to be used
 * \param[in] rxbyte Received byte
 * \return 0 Success
 * \return -1 Failure
 */
UAVTalkRxState UAVTalkRelayPacket(UAVTalkConnection inConnectionHandle, UAVTalkConnection outConnectionHandle)
{
    UAVTalkConnectionData *inConnection;

    CHECKCONHANDLE(inConnectionHandle, inConnection, return -1);
    UAVTalkInputProcessor *inIproc = &inConnection->iproc;

    // The input packet must be completely parsed.
    if (inIproc->state != UAVTALK_STATE_COMPLETE) {
        return -1;
    }

    UAVTalkConnectionData *outConnection;
    CHECKCONHANDLE(outConnectionHandle, outConnection, return -1);

    if (!outConnection->outStream) {
        return -1;
    }

    // Lock
    xSemaphoreTakeRecursive(outConnection->lock, portMAX_DELAY);

    // Setup sync byte
    outConnection->txBuffer[0] = UAVTALK_SYNC_VAL;
    // Setup type
    outConnection->txBuffer[1] = inIproc->type;
    // next 2 bytes are reserved for data length (inserted here later)
    int32_t dataOffset = 4;
    if (inIproc->objId != 0) {
        // Setup object ID
        outConnection->txBuffer[4] = (uint8_t)(inIproc->objId & 0xFF);
        outConnection->txBuffer[5] = (uint8_t)((inIproc->objId >> 8) & 0xFF);
        outConnection->txBuffer[6] = (uint8_t)((inIproc->objId >> 16) & 0xFF);
        outConnection->txBuffer[7] = (uint8_t)((inIproc->objId >> 24) & 0xFF);

        // Setup instance ID
        outConnection->txBuffer[8] = (uint8_t)(inIproc->instId & 0xFF);
        outConnection->txBuffer[9] = (uint8_t)((inIproc->instId >> 8) & 0xFF);
        dataOffset = 10;
    }

    // Add timestamp when the transaction type is appropriate
    if (inIproc->type & UAVTALK_TIMESTAMPED) {
        portTickType time = xTaskGetTickCount();
        outConnection->txBuffer[dataOffset]     = (uint8_t)(time & 0xFF);
        outConnection->txBuffer[dataOffset + 1] = (uint8_t)((time >> 8) & 0xFF);
        dataOffset += 2;
    }

    // Copy data (if any)
    memcpy(&outConnection->txBuffer[dataOffset], inConnection->rxBuffer, inIproc->length);

    // Store the packet length
    outConnection->txBuffer[2] = (uint8_t)((dataOffset + inIproc->length) & 0xFF);
    outConnection->txBuffer[3] = (uint8_t)(((dataOffset + inIproc->length) >> 8) & 0xFF);

    // Copy the checksum
    outConnection->txBuffer[dataOffset + inIproc->length] = inIproc->cs;

    // Send the buffer.
    int32_t rc = (*outConnection->outStream)(outConnection->txBuffer, inIproc->rxPacketLength);

    // Update stats
    outConnection->stats.txBytes += rc;

    // Release lock
    xSemaphoreGiveRecursive(outConnection->lock);

    // Done
    if (rc != inIproc->rxPacketLength) {
        return -1;
    }

    return 0;
}

/**
 * Complete receiving a UAVTalk packet.  This will cause the packet to be unpacked, acked, etc.
 * \param[in] connectionHandle UAVTalkConnection to be used
 * \return 0 Success
 * \return -1 Failure
 */
int32_t UAVTalkReceiveObject(UAVTalkConnection connectionHandle)
{
    UAVTalkConnectionData *connection;

    CHECKCONHANDLE(connectionHandle, connection, return -1);

    UAVTalkInputProcessor *iproc = &connection->iproc;
    if (iproc->state != UAVTALK_STATE_COMPLETE) {
        return -1;
    }

    receiveObject(connection, iproc->type, iproc->objId, iproc->instId, connection->rxBuffer, iproc->length);

    return 0;
}

/**
 * Get the object ID of the current packet.
 * \param[in] connectionHandle UAVTalkConnection to be used
 * \param[in] objId Object ID to send a NACK for
 * \return The object ID, or 0 on error.
 */
uint32_t UAVTalkGetPacketObjId(UAVTalkConnection connectionHandle)
{
    UAVTalkConnectionData *connection;

    CHECKCONHANDLE(connectionHandle, connection, return 0);

    return connection->iproc.objId;
}

/**
 * Receive an object. This function process objects received through the telemetry stream.
 *
 * Parser errors are considered as transmission errors and are not NACKed.
 * Some senders (GCS) can timeout and retry if the message is not answered by an ack or nack.
 *
 * Object handling errors are considered as application errors and are NACked.
 * In that case we want to nack as there is no point in the sender retrying to send invalid objects.
 *
 * \param[in] connection UAVTalkConnection to be used
 * \param[in] type Type of received message (UAVTALK_TYPE_OBJ, UAVTALK_TYPE_OBJ_REQ, UAVTALK_TYPE_OBJ_ACK, UAVTALK_TYPE_ACK, UAVTALK_TYPE_NACK)
 * \param[in] objId ID of the object to work on
 * \param[in] instId The instance ID of UAVOBJ_ALL_INSTANCES for all instances.
 * \param[in] data Data buffer
 * \param[in] length Buffer length
 * \return 0 Success
 * \return -1 Failure
 */
static int32_t receiveObject(UAVTalkConnectionData *connection,
                             uint8_t type,
                             uint32_t objId,
                             uint16_t instId,
                             uint8_t *data,
                             __attribute__((unused)) int32_t length)
{
    UAVObjHandle obj;
    int32_t ret = 0;

    // Lock
    xSemaphoreTakeRecursive(connection->lock, portMAX_DELAY);

    // Get the handle to the Object.
    // Will be zero if object does not exist.
    // Warning :
    // Here we ask for instance ID 0 without taking into account the provided instId
    // The provided instId will be used later when packing, unpacking, etc...
    // TODO the above should be fixed as it is cumbersome and error prone
    obj = UAVObjGetByID(objId);

    // Process message type
    switch (type) {
    case UAVTALK_TYPE_OBJ:
    case UAVTALK_TYPE_OBJ_TS:
        // All instances, not allowed for OBJ messages
        if (obj && (instId != UAVOBJ_ALL_INSTANCES)) {
            // Unpack object, if the instance does not exist it will be created!
            if (UAVObjUnpack(obj, instId, data)) {
                // Check if this object acks a pending OBJ_REQ message
                // any OBJ message can ack a pending OBJ_REQ message
                // even one that was not sent in response to the OBJ_REQ message
                updateAck(connection, type, objId, instId);
            }
            else {
                ret = -1;
            }
        } else {
            ret = -1;
        }
        break;

    case UAVTALK_TYPE_OBJ_ACK:
    case UAVTALK_TYPE_OBJ_ACK_TS:
        // All instances, not allowed for OBJ_ACK messages
        if (obj && (instId != UAVOBJ_ALL_INSTANCES)) {
            // Unpack object, if the instance does not exist it will be created!
            if (UAVObjUnpack(obj, instId, data) == 0) {
                // Object updated or created, transmit ACK
                sendObject(connection, UAVTALK_TYPE_ACK, objId, instId, NULL);
            } else {
                ret = -1;
            }
        } else {
            ret = -1;
        }
        if (ret == -1) {
            // failed to update object, transmit NACK
            sendObject(connection, UAVTALK_TYPE_NACK, objId, instId, NULL);
        }
        break;

    case UAVTALK_TYPE_OBJ_REQ:
        // Check if requested object exists
        if (obj && ((instId == UAVOBJ_ALL_INSTANCES) || instId < UAVObjGetNumInstances(obj))) {
            // Object found, transmit it
            // The sent object will ack the object request on the receiver side
            sendObject(connection, UAVTALK_TYPE_OBJ, objId, instId, obj);
        } else {
            ret = -1;
        }
        if (ret == -1) {
            // failed to send object, transmit NACK
            sendObject(connection, UAVTALK_TYPE_NACK, objId, instId, NULL);
        }
        break;

    case UAVTALK_TYPE_NACK:
        // Do nothing on flight side, let it time out.
        // TODO:
        // The transaction takes the result code of the "semaphore taking operation" into account to determine success.
        // If we give that semaphore in time, its "success" (ack received)
        // If we do not give that semaphore before the timeout it will return failure.
        // What would have to be done here is give the semaphore, but set a flag (for example connection->respFail=true)
        // that indicates failure and then above where it checks for the result code, have it behave as if it failed
        // if the explicit failure is set.
        break;

    case UAVTALK_TYPE_ACK:
        // All instances, not allowed for ACK messages
        if (obj && (instId != UAVOBJ_ALL_INSTANCES)) {
            // Check if an ACK is pending
            updateAck(connection, type, objId, instId);
        } else {
            ret = -1;
        }
        break;

    default:
        ret = -1;
    }

    // Unlock
    xSemaphoreGiveRecursive(connection->lock);

    // Done
    return ret;
}

/**
 * Check if an ack is pending on an object and give response semaphore
 * \param[in] connection UAVTalkConnection to be used
 * \param[in] obj Object
 * \param[in] instId The instance ID of UAVOBJ_ALL_INSTANCES for all instances.
 */
static void updateAck(UAVTalkConnectionData *connection, uint8_t type, uint32_t objId, uint16_t instId)
{
    if ((connection->respType == type) && (connection->respObjId == objId) && ((connection->respInstId == instId) || (connection->respInstId == UAVOBJ_ALL_INSTANCES))) {
        xSemaphoreGive(connection->respSema);
        connection->respObjId = 0;
    }
}

/**
 * Send an object through the telemetry link.
 * \param[in] connection UAVTalkConnection to be used
 * \param[in] type Transaction type
 * \param[in] objId The object ID
 * \param[in] instId The instance ID or UAVOBJ_ALL_INSTANCES for all instances
 * \param[in] obj Object handle to send (null when type is NACK)
 * \return 0 Success
 * \return -1 Failure
 */
static int32_t sendObject(UAVTalkConnectionData *connection, uint8_t type, uint32_t objId, uint16_t instId, UAVObjHandle obj)
{
    uint32_t numInst;
    uint32_t n;

    // Important note : obj can be null (when type is NACK for example) so protect all obj dereferences.

    // If all instances are requested and this is a single instance object, force instance ID to zero
    if ((obj != NULL) && (instId == UAVOBJ_ALL_INSTANCES) && UAVObjIsSingleInstance(obj)) {
        instId = 0;
    }

    // Process message type
    if (type == UAVTALK_TYPE_OBJ || type == UAVTALK_TYPE_OBJ_TS || type == UAVTALK_TYPE_OBJ_ACK || type == UAVTALK_TYPE_OBJ_ACK_TS) {
        if (instId == UAVOBJ_ALL_INSTANCES) {
            // Get number of instances
            numInst = UAVObjGetNumInstances(obj);
            // Send all instances in reverse order
            // This allows the receiver to detect when the last object has been received (i.e. when instance 0 is received)
            for (n = 0; n < numInst; ++n) {
                sendSingleObject(connection, type, objId, numInst - n - 1, obj);
            }
            return 0;
        } else {
            return sendSingleObject(connection, type, objId, instId, obj);
        }
    } else if (type == UAVTALK_TYPE_OBJ_REQ) {
        return sendSingleObject(connection, UAVTALK_TYPE_OBJ_REQ, objId, instId, obj);
    } else if (type == UAVTALK_TYPE_ACK || type == UAVTALK_TYPE_NACK) {
        if (instId != UAVOBJ_ALL_INSTANCES) {
            return sendSingleObject(connection, type, objId, instId, obj);
        } else {
            return -1;
        }
    } else {
        return -1;
    }
}

/**
 * Send an object through the telemetry link.
 * \param[in] connection UAVTalkConnection to be used
 * \param[in] type Transaction type
 * \param[in] objId The object ID
 * \param[in] instId The instance ID (can NOT be UAVOBJ_ALL_INSTANCES, use
() instead)
 * \param[in] obj Object handle to send (null when type is NACK)
 * \return 0 Success
 * \return -1 Failure
 */
static int32_t sendSingleObject(UAVTalkConnectionData *connection, uint8_t type, uint32_t objId, uint16_t instId, UAVObjHandle obj)
{
    int32_t length;
    int32_t dataOffset;

    // IMPORTANT : obj can be null (when type is NACK for example)

    if (!connection->outStream) {
        return -1;
    }

    // Setup sync byte
    connection->txBuffer[0] = UAVTALK_SYNC_VAL;
    // Setup type
    connection->txBuffer[1] = type;
    // next 2 bytes are reserved for data length (inserted here later)
    // Setup object ID
    connection->txBuffer[4] = (uint8_t)(objId & 0xFF);
    connection->txBuffer[5] = (uint8_t)((objId >> 8) & 0xFF);
    connection->txBuffer[6] = (uint8_t)((objId >> 16) & 0xFF);
    connection->txBuffer[7] = (uint8_t)((objId >> 24) & 0xFF);
    // Setup instance ID
    connection->txBuffer[8] = (uint8_t)(instId & 0xFF);
    connection->txBuffer[9] = (uint8_t)((instId >> 8) & 0xFF);
    dataOffset = 10;

    // Add timestamp when the transaction type is appropriate
    if (type & UAVTALK_TIMESTAMPED) {
        portTickType time = xTaskGetTickCount();
        connection->txBuffer[dataOffset]     = (uint8_t)(time & 0xFF);
        connection->txBuffer[dataOffset + 1] = (uint8_t)((time >> 8) & 0xFF);
        dataOffset += 2;
    }

    // Determine data length
    if (type == UAVTALK_TYPE_OBJ_REQ || type == UAVTALK_TYPE_ACK || type == UAVTALK_TYPE_NACK) {
        length = 0;
    } else {
        length = UAVObjGetNumBytes(obj);
    }

    // Check length
    if (length >= UAVTALK_MAX_PAYLOAD_LENGTH) {
        return -1;
    }

    // Copy data (if any)
    if (length > 0) {
        if (UAVObjPack(obj, instId, &connection->txBuffer[dataOffset]) < 0) {
            return -1;
        }
    }

    // Store the packet length
    connection->txBuffer[2] = (uint8_t)((dataOffset + length) & 0xFF);
    connection->txBuffer[3] = (uint8_t)(((dataOffset + length) >> 8) & 0xFF);

    // Calculate checksum
    connection->txBuffer[dataOffset + length] = PIOS_CRC_updateCRC(0, connection->txBuffer, dataOffset + length);

    uint16_t tx_msg_len = dataOffset + length + UAVTALK_CHECKSUM_LENGTH;
    int32_t rc = (*connection->outStream)(connection->txBuffer, tx_msg_len);

    if (rc == tx_msg_len) {
        // Update stats
        ++connection->stats.txObjects;
        connection->stats.txBytes += tx_msg_len;
        connection->stats.txObjectBytes += length;
    }

    // Done
    return 0;
}

/**
 * @}
 * @}
 */
