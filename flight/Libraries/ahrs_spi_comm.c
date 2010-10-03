#include "ahrs_spi_comm.h"
#include "ahrs_spi_program.h"

#ifdef IN_AHRS
#include <string.h>
#include "pios_debug.h"
#include "pios_spi.h"
#include "pios_irq.h"
#include "ahrs_spi_program_slave.h"
#include "STM32103CB_AHRS.h"
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
typedef enum { COMMS_NULL, COMMS_OBJECT, COMMS_PROGRAM } COMMSCOMMAND;

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

/** Pending events.
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

void AhrsInitComms(void)
{
	AhrsInitHandles();
	memset(objCallbacks, 0, sizeof(AhrsEventCallback) * MAX_AHRS_OBJECTS);
	memset(callbackPending, 0, sizeof(bool) * MAX_AHRS_OBJECTS);
	memset(dirtyObjects, 0, sizeof(unsigned int) * MAX_AHRS_OBJECTS);
	memset(&txPacket, 0, sizeof(txPacket));
	memset(&rxPacket, 0, sizeof(rxPacket));
	memset(&readyObjects, 0, sizeof(bool) * MAX_AHRS_OBJECTS);
	txPacket.command = COMMS_NULL;
	rxPacket.command = COMMS_NULL;

#ifdef IN_AHRS
	PIOS_SPI_Init();
#else
	PIOS_SPI_SetClockSpeed(PIOS_OPAHRS_SPI, PIOS_SPI_PRESCALER_8);	//36MHz/16 = 2.25MHz
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
void FillObjectPacket()
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
void HandleRxPacket()
{
	switch (rxPacket.command) {
	case COMMS_NULL:
		break;

	case COMMS_OBJECT:
		HandleObjectPacket();
		break;

	case COMMS_PROGRAM:	//TODO: programming protocol
		break;

	default:
		txPacket.status.invalidPacket++;
	}
}

/** Process a received UAVObject packet

*/
void HandleObjectPacket()
{
	for (int ct = 0; ct < MAX_UPDATE_OBJECTS; ct++) {

		//Flag objects that have been successfully received at the other end
		uint8_t idx = rxPacket.objects[ct].done;
		if (idx < MAX_AHRS_OBJECTS) {

			if (dirtyObjects[idx] == 1) {	//this ack is the correct one for the last send
				dirtyObjects[idx] = 0;
			}
		}

		txPacket.objects[ct].done = AHRS_NO_OBJECT;
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
			callbackPending[idx] = true;
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

AhrsCommStatus AhrsGetStatus()
{
	AhrsCommStatus status;
	status.remote = rxPacket.status;
	status.local = txPacket.status;
	status.linkOk = linkOk;
	return (status);
}

void CommsCallback(uint8_t crc_ok, uint8_t crc_val)
{
#ifndef IN_AHRS
	PIOS_SPI_RC_PinSet(PIOS_OPAHRS_SPI, 1);	//signal the end of the transfer
#endif
	txPacket.command = COMMS_NULL;	//we must send something so default to null
	if (rxPacket.magicNumber != RXMAGIC) {
		crc_ok = false;
	}


	rxPacket.magicNumber = 0;
	if (crc_ok) {
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
#ifdef IN_AHRS			//AHRS - do we neeed to enter program mode?
		if (memcmp(&rxPacket, SPI_PROGRAM_REQUEST, SPI_PROGRAM_REQUEST_LENGTH) == 0) {
			AhrsProgramReceive();
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
#ifdef IN_AHRS
	/*queue next frame
	   If PIOS_SPI_TransferBlock() fails for any reason, comms will stop working.
	   In that case, AhrsPoll() should kick start things again.
	 */
	PIOS_SPI_TransferBlock(PIOS_SPI_OP, (uint8_t *) & txPacket, (uint8_t *) & rxPacket, sizeof(CommsDataPacket), &CommsCallback);
#endif
}

void PollEvents(void)
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
	PollEvents();
	if (PIOS_SPI_Busy(PIOS_SPI_OP) != 0) {	//Everything is working correctly
		return;
	}
	txPacket.status.kickStarts++;
//comms have broken down - try kick starting it.
	txPacket.command = COMMS_NULL;	//we must send something so default to null
	PIOS_SPI_TransferBlock(PIOS_SPI_OP, (uint8_t *) & txPacket, (uint8_t *) & rxPacket, sizeof(CommsDataPacket), &CommsCallback);
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
	PIOS_SPI_RC_PinSet(PIOS_OPAHRS_SPI, 0);
	//no point checking if this failed. There isn't much we could do about it if it did fail
	PIOS_SPI_TransferBlock(PIOS_OPAHRS_SPI, (uint8_t *) & txPacket, (uint8_t *) & rxPacket, sizeof(CommsDataPacket), &CommsCallback);
}

static void AhrsUpdatedCb(AhrsObjHandle handle)
{
	UAVObjSetData(handle->uavHandle, handle->data);
	return;
}
#endif
