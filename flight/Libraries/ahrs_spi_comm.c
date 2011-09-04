/**
 ******************************************************************************
 *
 * @file       ahrs_spi_comm.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      AHRS SPI communications.
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
#include <pios.h>
#include "ahrs_spi_comm.h"
#include "ahrs_spi_program.h"

#ifdef IN_AHRS
#include <string.h>
#include "pios_debug.h"
#include "pios_spi.h"
#include "pios_irq.h"
#include "ahrs_spi_program_slave.h"
//#include "STM32103CB_AHRS.h"
#endif

/*transmit and receive packet magic numbers.
These numbers are chosen to be very unlikely to occur due to noise.
CRC8 does not always catch noise from cross-coupling between data lines.
*/
#ifdef IN_AHRS
#define TXMAGIC 0xA55AF0C3
#define RXMAGIC 0x3C5AA50F
#else
#define RXMAGIC 0xA55AF0C3
#define TXMAGIC 0x3C5AA50F
#endif

//packet types
typedef enum { COMMS_NULL, COMMS_OBJECT } COMMSCOMMAND;

//The maximum number of objects that can be updated in one cycle.
//Currently the link is capable of sending 3 packets per cycle but 2 is enough
#define MAX_UPDATE_OBJECTS 2

//Number of transmissions + 1 before we expect to see the data acknowledge
//This is controlled by the SPI hardware.
#define ACK_LATENCY 4

/** All data for one object
*/
typedef struct {
	uint8_t done;
	uint8_t index;
	AhrsSharedData object;
} ObjectPacketData;

/** One complete packet.
Other packet types are allowed for. The frame size will be the size of this
structure.
*/
typedef struct {
	uint32_t magicNumber;
	COMMSCOMMAND command;
	AhrsEndStatus status;
	union {			//allow for expansion to other packet types.
		ObjectPacketData objects[MAX_UPDATE_OBJECTS];
	};
	uint8_t dummy; //For some reason comms trashes the last byte
} CommsDataPacket;

static void FillObjectPacket();
static void CommsCallback(uint8_t crc_ok, uint8_t crc_val);
static void SetObjectDirty(const int idx);
static void HandleObjectPacket();
static void HandleRxPacket();
static void PollEvents();
#ifndef IN_AHRS
static void SendPacket(void);
static void AhrsUpdatedCb(AhrsObjHandle handle);
#endif

/** Receive data buffer
*/
static CommsDataPacket rxPacket;

/** Transmit data buffer
*/

static CommsDataPacket txPacket;
/** Objects that have changed and so should be transmitted
*/
static unsigned int dirtyObjects[MAX_AHRS_OBJECTS];

/** Objects that have been updated at startup
*/
static bool readyObjects[MAX_AHRS_OBJECTS];

/** List of event callbacks
*/
static AhrsEventCallback objCallbacks[MAX_AHRS_OBJECTS];

/** True for objects for which new data is received and callback needs to be called
*/
static bool callbackPending[MAX_AHRS_OBJECTS];

//More than this number of errors in a row will indicate the link is down
#define MAX_CRC_ERRORS 50
//At least this number of good frames are needed to indicate the link is up.
#define MIN_OK_FRAMES 50
//At least this number of empty objects are needed before the initial flood of events is over.
#define MIN_EMPTY_OBJECTS 10

static uint8_t linkOk = false;
static int okCount = MIN_OK_FRAMES;
static int emptyCount = MIN_EMPTY_OBJECTS;
static bool programReceive = false;

void AhrsInitComms(void)
{
	programReceive = false;
	AhrsInitHandles();
	memset(objCallbacks, 0, sizeof(AhrsEventCallback) * MAX_AHRS_OBJECTS);
	memset(callbackPending, 0, sizeof(bool) * MAX_AHRS_OBJECTS);
	memset(dirtyObjects, 0, sizeof(unsigned int) * MAX_AHRS_OBJECTS);
	memset(&txPacket, 0, sizeof(txPacket));
	memset(&rxPacket, 0, sizeof(rxPacket));
	memset(&readyObjects, 0, sizeof(bool) * MAX_AHRS_OBJECTS);
	txPacket.command = COMMS_NULL;
	rxPacket.command = COMMS_NULL;
}

static uint32_t opahrs_spi_id;
void AhrsConnect(uint32_t spi_id)
{
	/* Bind this comms layer to the appropriate SPI id */
	opahrs_spi_id = spi_id;
#ifndef IN_AHRS
	/* Comms already init in OP code */
	for (int ct = 0; ct < MAX_AHRS_OBJECTS; ct++) {
		AhrsObjHandle hdl = AhrsFromIndex(ct);
		if (hdl) {
			AhrsConnectCallBack(hdl, AhrsUpdatedCb);
		}
	}
#endif
}

int32_t AhrsSetData(AhrsObjHandle obj, const void *dataIn)
{
	if (obj == NULL || dataIn == NULL || obj->data == NULL) {
		return (-1);
	}
	if (memcmp(obj->data, dataIn, obj->size) == 0) {	//nothing to do, don't generate an event
		return (0);
	}
	memcpy(obj->data, dataIn, obj->size);
	SetObjectDirty(obj->index);
	return (0);
}

int32_t AhrsGetData(AhrsObjHandle obj, void *dataOut)
{

	if (obj == NULL || dataOut == NULL || obj->data == NULL) {
		return (-1);
	}
	memcpy(dataOut, obj->data, obj->size);
	return (0);
}

/** Mark an object to be sent
*/
void SetObjectDirty(const int idx)
{
	if (idx < 0 || idx >= MAX_AHRS_OBJECTS) {
		return;
	}
	dirtyObjects[idx] = ACK_LATENCY;
}
/** Work out what data needs to be sent.
	If an object was not sent it will be retried 4 frames later
*/
static void FillObjectPacket()
{
	txPacket.command = COMMS_OBJECT;
	txPacket.magicNumber = TXMAGIC;
	int idx = 0;
	for (int ct = 0; ct < MAX_UPDATE_OBJECTS; ct++) {
		txPacket.objects[ct].index = AHRS_NO_OBJECT;
		for (; idx < MAX_AHRS_OBJECTS; idx++) {
			if (dirtyObjects[idx] > 0) {
				if (dirtyObjects[idx] == ACK_LATENCY) {
					dirtyObjects[idx]--;
					txPacket.objects[ct].index = idx;
					AhrsObjHandle hdl = AhrsFromIndex(idx);
					if (hdl) {
						memcpy(&txPacket.objects[ct].object, hdl->data, hdl->size);
						break;
					}
				} else {
					dirtyObjects[idx]--;
					if (dirtyObjects[idx] == 0) {	//timed out
						dirtyObjects[idx] = ACK_LATENCY;
						txPacket.status.retries++;
					}
				}
			}
		}
	}
	for (; idx < MAX_AHRS_OBJECTS; idx++) {
		if (dirtyObjects[idx] > 0 && dirtyObjects[idx] != ACK_LATENCY) {
			dirtyObjects[idx]--;
			if (dirtyObjects[idx] == 0) {	//timed out
				dirtyObjects[idx] = ACK_LATENCY;
				txPacket.status.retries++;
			}
		}
	}
}

/** Process a received packet
*/
static void HandleRxPacket()
{
	switch (rxPacket.command) {
	case COMMS_NULL:
	    // Empty packet, nothing to do
		break;

	case COMMS_OBJECT:
		HandleObjectPacket();
		break;

	default:
		txPacket.status.invalidPacket++;
	}
}

/** Process a received UAVObject packet
*/
static void HandleObjectPacket()
{
	for (int ct = 0; ct < MAX_UPDATE_OBJECTS; ct++) {
	    uint8_t idx;

		// Flag objects that have been successfully received at the other end
		idx = rxPacket.objects[ct].done;
        txPacket.objects[ct].done = AHRS_NO_OBJECT;
		if (idx < MAX_AHRS_OBJECTS) {

			if (dirtyObjects[idx] == 1) {	//this ack is the correct one for the last send
				dirtyObjects[idx] = 0;
			}
		}

		// Handle received object if there is one in this packet
		idx = rxPacket.objects[ct].index;
		if (idx == AHRS_NO_OBJECT) {
			if (emptyCount > 0) {
				emptyCount--;
			}
			continue;
		}
		AhrsObjHandle obj = AhrsFromIndex(idx);
		if (obj) {
			memcpy(obj->data, &rxPacket.objects[ct].object, obj->size);
			txPacket.objects[ct].done = idx;
			callbackPending[idx] = true;    // New data available, call callback
			readyObjects[idx] = true;
		} else {
			txPacket.status.invalidPacket++;
		}
	}
#ifdef IN_AHRS
	FillObjectPacket();	//ready for the next frame
#endif
}

int32_t AhrsConnectCallBack(AhrsObjHandle obj, AhrsEventCallback cb)
{
	if (obj == NULL || obj->data == NULL) {
		return (-1);
	}
	objCallbacks[obj->index] = cb;
	return (0);
}

void AhrsGetStatus(AhrsCommStatus * status)
{
	status->remote = rxPacket.status;
	status->local = txPacket.status;
	status->linkOk = linkOk;
}


/** Function called after an SPI transfer
 */
static void CommsCallback(uint8_t crc_ok, uint8_t crc_val)
{
#ifndef IN_AHRS
	PIOS_SPI_RC_PinSet(opahrs_spi_id, 1);	//signal the end of the transfer
#endif
	txPacket.command = COMMS_NULL;	//we must send something so default to null

	// While the crc is ok, there is a magic value in the received data for extra security
	if (rxPacket.magicNumber != RXMAGIC) {
		crc_ok = false;
	}


	if (crc_ok) {
	    // The received data is OK, update link state and handle data
		if (!linkOk && okCount > 0) {
			okCount--;
			if (okCount == 0) {
				linkOk = true;
				okCount = MAX_CRC_ERRORS;
				emptyCount = MIN_EMPTY_OBJECTS;
			}
		}
		HandleRxPacket();
	} else {
	    // The received data is incorrect, update state
#ifdef IN_AHRS			//AHRS - do we neeed to enter program mode?
		if (memcmp(&rxPacket, SPI_PROGRAM_REQUEST, SPI_PROGRAM_REQUEST_LENGTH) == 0)
		{
			rxPacket.magicNumber = 0;
			programReceive = true; //flag it to be executed in program space
			return;
		}
#endif
		txPacket.status.crcErrors++;
		if (linkOk && okCount > 0) {
			okCount--;
			if (okCount == 0) {
				linkOk = false;
				okCount = MIN_OK_FRAMES;
			}
		}
	}
	rxPacket.magicNumber = 0;
#ifdef IN_AHRS
	/*queue next frame
	   If PIOS_SPI_TransferBlock() fails for any reason, comms will stop working.
	   In that case, AhrsPoll() should kick start things again.
	 */
	PIOS_SPI_TransferBlock(opahrs_spi_id, (uint8_t *) & txPacket, (uint8_t *) & rxPacket, sizeof(CommsDataPacket), &CommsCallback);
#endif
}

/** Call callbacks for object where new data is received
 */
static void PollEvents(void)
{
	for (int idx = 0; idx < MAX_AHRS_OBJECTS; idx++) {
		if (objCallbacks[idx]) {
			PIOS_IRQ_Disable();
			if (callbackPending[idx]) {
				callbackPending[idx] = false;
				PIOS_IRQ_Enable();
				objCallbacks[idx] (AhrsFromIndex(idx));
			} else {
				PIOS_IRQ_Enable();
			}
		}
	}
}

#ifdef IN_AHRS
void AhrsPoll()
{
	if(programReceive)
	{
		AhrsProgramReceive(opahrs_spi_id);
		programReceive = false;
	}
	PollEvents();
	if (PIOS_SPI_Busy(opahrs_spi_id) != 0) {	//Everything is working correctly
		return;
	}
	txPacket.status.kickStarts++;
//comms have broken down - try kick starting it.
	txPacket.command = COMMS_NULL;	//we must send something so default to null
	PIOS_SPI_TransferBlock(opahrs_spi_id, (uint8_t *) & txPacket, (uint8_t *) & rxPacket, sizeof(CommsDataPacket), &CommsCallback);
}

bool AhrsLinkReady(void)
{

	for (int ct = 0; ct < MAX_AHRS_OBJECTS; ct++) {
		if (!readyObjects[ct]) {
			return (false);
		}
	}
	return (linkOk);
}

#else

void AhrsSendObjects(void)
{
	static bool oldLink = false;

	PollEvents();

	if (oldLink != linkOk) {
		oldLink = linkOk;
		if (linkOk) {
			for (int ct = 0; ct < MAX_AHRS_OBJECTS; ct++) {
				AhrsObjHandle hdl = AhrsFromIndex(ct);
				if (!hdl) {	//paranoid - shouldn't ever happen
					continue;
				}
				AhrsSharedData data;
				UAVObjGetData(hdl->uavHandle, &data);
				AhrsSetData(hdl, &data);
				SetObjectDirty(ct);	//force even unchanged data to be sent
			}
		}
	}
	FillObjectPacket();
	SendPacket();
}

void SendPacket(void)
{
#ifndef IN_AHRS
	PIOS_SPI_RC_PinSet(opahrs_spi_id, 0);
#endif
	//no point checking if this failed. There isn't much we could do about it if it did fail
	PIOS_SPI_TransferBlock(opahrs_spi_id, (uint8_t *) & txPacket, (uint8_t *) & rxPacket, sizeof(CommsDataPacket), &CommsCallback);
}

static void AhrsUpdatedCb(AhrsObjHandle handle)
{
	UAVObjSetData(handle->uavHandle, handle->data);
	return;
}
#endif
