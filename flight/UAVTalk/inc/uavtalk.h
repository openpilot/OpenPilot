/**
 ******************************************************************************
 * @addtogroup OpenPilotSystem OpenPilot System
 * @{
 * @addtogroup OpenPilotLibraries OpenPilot System Libraries
 * @{
 * @file       uavtalk.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Include file of the UAVTalk library
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

#ifndef UAVTALK_H
#define UAVTALK_H

// Public constants
#define UAVTALK_WAITFOREVER -1
#define UAVTALK_NOWAIT 0
#define UAVTALK_MIN_HEADER_LENGTH	8	// sync(1), type (1), size (2), object ID (4)
#define UAVTALK_MAX_HEADER_LENGTH	10	// sync(1), type (1), size (2), object ID (4), instance ID (2, not used in single objects)

#define UAVTALK_CHECKSUM_LENGTH		1

#define UAVTALK_MAX_PAYLOAD_LENGTH	256

#define UAVTALK_MAX_PACKET_LENGTH	(UAVTALK_MAX_HEADER_LENGTH + UAVTALK_MAX_PAYLOAD_LENGTH + UAVTALK_CHECKSUM_LENGTH)


// Public types
typedef int32_t (*UAVTalkOutputStream)(uint8_t* data, int32_t length);

typedef enum {UAVTALK_STATE_SYNC, UAVTALK_STATE_TYPE, UAVTALK_STATE_SIZE, UAVTALK_STATE_OBJID, UAVTALK_STATE_INSTID, UAVTALK_STATE_DATA, UAVTALK_STATE_CS} UAVTalkRxState;

typedef struct {
    uint32_t txBytes;
    uint32_t rxBytes;
    uint32_t txObjectBytes;
    uint32_t rxObjectBytes;
    uint32_t rxObjects;
    uint32_t txObjects;
    uint32_t txErrors;
    uint32_t rxErrors;
} UAVTalkStats;

typedef struct {
    UAVObjHandle obj;
    uint8_t type;
    uint16_t packet_size;
    uint32_t objId;
    uint16_t instId;
    uint32_t length;
    uint8_t cs;
    int32_t rxCount;
    UAVTalkRxState state;
    uint16_t rxPacketLength;
} UAVTalkInputProcessor;

typedef struct {
    UAVTalkOutputStream outStream;
    xSemaphoreHandle lock;
    xSemaphoreHandle transLock;
    xSemaphoreHandle respSema;
    UAVObjHandle respObj;
    uint16_t respInstId;
    uint8_t rxBuffer[UAVTALK_MAX_PACKET_LENGTH];
    uint8_t txBuffer[UAVTALK_MAX_PACKET_LENGTH];
    UAVTalkStats stats;
    UAVTalkInputProcessor iproc;
} UAVTalkConnection;

// Public functions
int32_t UAVTalkInitialize(UAVTalkConnection *connection, UAVTalkOutputStream outputStream);
int32_t UAVTalkSetOutputStream(UAVTalkConnection *connection, UAVTalkOutputStream outputStream);
UAVTalkOutputStream UAVTalkGetOutputStream(UAVTalkConnection *connection);
int32_t UAVTalkSendObject(UAVTalkConnection *connection, UAVObjHandle obj, uint16_t instId, uint8_t acked, int32_t timeoutMs);
int32_t UAVTalkSendObjectRequest(UAVTalkConnection *connection, UAVObjHandle obj, uint16_t instId, int32_t timeoutMs);
int32_t UAVTalkProcessInputStream(UAVTalkConnection *connection, uint8_t rxbyte);
void UAVTalkGetStats(UAVTalkConnection *connection, UAVTalkStats* stats);
void UAVTalkResetStats(UAVTalkConnection *connection);

#endif // UAVTALK_H
/**
 * @}
 * @}
 */
