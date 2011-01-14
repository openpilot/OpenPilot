/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup MKSerialModule MK Serial Control Module
 * @brief Connect to MK module
 * @{ 
 *
 * @file       MKSerial.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Interfacing with MK via serial port
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
#include "MkSerial.h"

#include "attitudeactual.h"	// object that will be updated by the module
#include "positionactual.h"
#include "flightbatterystate.h"

//
// Configuration
//
#define PORT			PIOS_COM_AUX
#define DEBUG_PORT		PIOS_COM_GPS
#define STACK_SIZE		1024
#define TASK_PRIORITY	(tskIDLE_PRIORITY + 3)
#define MAX_NB_PARS 	100
//#define ENABLE_DEBUG_MSG
//#define GENERATE_BATTERY_INFO         // The MK can report battery voltage, but normally the current sensor will be used, so this module should not report battery state

#if PORT == PIOS_COM_AUX
#ifndef PIOS_ENABLE_AUX_UART
#error "This module cannot be included without the AUX UART enabled"
#endif
#endif
//
// Private constants
//
#define MSGCMD_ANY				0
#define MSGCMD_GET_DEBUG		'd'
#define MSGCMD_DEBUG			'D'
#define MSGCMD_GET_VERSION		'v'
#define MSGCMD_VERSION			'V'
#define MSGCMD_GET_OSD			'o'
#define MSGCMD_OSD				'O'

#define DEBUG_MSG_NICK_IDX	(2+2*2)
#define DEBUG_MSG_ROLL_IDX	(2+3*2)

#define OSD_MSG_CURRPOS_IDX		1
#define OSD_MSG_NB_SATS_IDX 	50
#define OSD_MSG_BATT_IDX		57
#define OSD_MSG_GNDSPEED_IDX    58
#define OSD_MSG_COMPHEADING_IDX	62
#define OSD_MSG_NICK_IDX		64
#define OSD_MSG_ROLL_IDX		65

#ifdef ENABLE_DEBUG_MSG
#define DEBUG_MSG(format, ...) PIOS_COM_SendFormattedString(DEBUG_PORT, format, ## __VA_ARGS__)
#else
#define DEBUG_MSG(format, ...)
#endif

//
// Private types
//
typedef struct {
	uint8_t address;
	uint8_t cmd;
	uint8_t nbPars;
	uint8_t pars[MAX_NB_PARS];
} MkMsg_t;

typedef struct {
	float longitute;
	float latitude;
	float altitude;
	uint8_t status;
} GpsPosition_t;

enum {
	MK_ADDR_ALL = 0,
	MK_ADDR_FC = 1,
	MK_ADDR_NC = 2,
	MK_ADDR_MAG = 3,
};

//
// Private variables
//

//
// Private functions
//
static void OnError(int line);
//static void PrintMsg(const MkMsg_t* msg);
static int16_t Par2Int16(const MkMsg_t * msg, uint8_t index);
static int32_t Par2Int32(const MkMsg_t * msg, uint8_t index);
static int8_t Par2Int8(const MkMsg_t * msg, uint8_t index);
static void GetGpsPos(const MkMsg_t * msg, uint8_t index, GpsPosition_t * pos);
static uint8_t WaitForBytes(uint8_t * buf, uint8_t nbBytes, portTickType xTicksToWait);
static bool WaitForMsg(uint8_t cmd, MkMsg_t * msg, portTickType xTicksToWait);
static void SendMsg(const MkMsg_t * msg);
static void SendMsgParNone(uint8_t address, uint8_t cmd);
static void SendMsgPar8(uint8_t address, uint8_t cmd, uint8_t par0);
static void MkSerialTask(void *parameters);

static void OnError(int line)
{
	DEBUG_MSG("MKProcol error %d\n\r", line);
}

#if 0
static void PrintMsg(const MkMsg_t * msg)
{
	switch (msg->address) {
	case MK_ADDR_ALL:
		DEBUG_MSG("ALL ");
		break;
	case MK_ADDR_FC:
		DEBUG_MSG("FC  ");
		break;
	case MK_ADDR_NC:
		DEBUG_MSG("NC  ");
		break;
	case MK_ADDR_MAG:
		DEBUG_MSG("MAG ");
		break;
	default:
		DEBUG_MSG("??? ");
		break;
	}

	DEBUG_MSG("%c ", msg->cmd);

	for (int i = 0; i < msg->nbPars; i++) {
		DEBUG_MSG("%02x ", msg->pars[i]);
	}
	DEBUG_MSG("\n\r");
}
#endif

static int16_t Par2Int16(const MkMsg_t * msg, uint8_t index)
{
	int16_t res;

	res = (int)(msg->pars[index + 1]) * 256 + msg->pars[index];
	if (res > 0xFFFF / 2)
		res -= 0xFFFF;
	return res;
}

static int32_t Par2Int32(const MkMsg_t * msg, uint8_t index)
{
	uint32_t val = 0;

	val = (((int)msg->pars[index]) << 0) + (((int)msg->pars[index + 1]) << 8);
	val += (((int)msg->pars[index + 2]) << 16) + ((int)msg->pars[index + 3] << 24);
	if (val > 0xFFFFFFFF / 2)
		val -= 0xFFFFFFFF;
	return (int32_t) val;
}

static int8_t Par2Int8(const MkMsg_t * msg, uint8_t index)
{
	if (msg->pars[index] > 127)
		return msg->pars[index] - 256;
	else
		return msg->pars[index];
}

static void GetGpsPos(const MkMsg_t * msg, uint8_t index, GpsPosition_t * pos)
{
	pos->longitute = (float)Par2Int32(msg, index) * (float)1e-7;
	pos->latitude = (float)Par2Int32(msg, index + 4) * (float)1e-7;
	pos->altitude = (float)Par2Int32(msg, index + 8) * (float)1e-3;
	pos->status = msg->pars[index + 12];
}

static uint8_t WaitForBytes(uint8_t * buf, uint8_t nbBytes, portTickType xTicksToWait)
{
	uint8_t nbBytesLeft = nbBytes;
	xTimeOutType xTimeOut;

	vTaskSetTimeOutState(&xTimeOut);

	// Loop until
	// - all bytes are received
	// - \r is seen
	// - Timeout occurs
	do {
		// Check if timeout occured
		if (xTaskCheckForTimeOut(&xTimeOut, &xTicksToWait))
			break;

		// Check if there are some bytes
		if (PIOS_COM_ReceiveBufferUsed(PORT)) {
			*buf = PIOS_COM_ReceiveBuffer(PORT);

			nbBytesLeft--;
			if (buf[0] == '\r')
				break;
			buf++;
		} else {
			// Avoid tight loop
			// FIXME: should be blocking
			vTaskDelay(5);
		}
	} while (nbBytesLeft);

	return nbBytes - nbBytesLeft;
}

static bool WaitForMsg(uint8_t cmd, MkMsg_t * msg, portTickType xTicksToWait)
{
	uint8_t buf[10];
	uint8_t n;
	bool done = FALSE;
	bool error = FALSE;
	unsigned int checkVal;
	xTimeOutType xTimeOut;

	vTaskSetTimeOutState(&xTimeOut);

	while (!done && !error) {
		// When we are here, it means we did not encounter the message we are waiting for
		// Check if we did not timeout yet.

		// Wait for start
		buf[0] = 0;
		do {
			if (xTaskCheckForTimeOut(&xTimeOut, &xTicksToWait)) {
				return FALSE;
			}
			WaitForBytes(buf, 1, 100 / portTICK_RATE_MS);
		} while (buf[0] != '#');

		// Wait for cmd and address
		if (WaitForBytes(buf, 2, 10 / portTICK_RATE_MS) != 2) {
			OnError(__LINE__);
			continue;
		}
		// Is this the command we are waiting for?
		if (cmd == 0 || cmd == buf[1]) {
			// OK follow this message to the end
			msg->address = buf[0] - 'a';
			msg->cmd = buf[1];

			checkVal = '#' + buf[0] + buf[1];

			// Parse parameters
			msg->nbPars = 0;
			while (!done && !error) {
				n = WaitForBytes(buf, 4, 10 / portTICK_RATE_MS);
				if (n > 0 && buf[n - 1] == '\r') {
					n--;
					// This is the end of the message
					// Get check bytes
					if (n >= 2) {
						unsigned int msgCeckVal;
						msgCeckVal = (buf[n - 1] - '=') + (buf[n - 2] - '=') * 64;
						//printf("%x %x\n", msgCeckVal, checkVal&0xFFF);
						n -= 2;

						if (msgCeckVal == (checkVal & 0xFFF)) {
							done = TRUE;
						} else {
							OnError(__LINE__);
							error = TRUE;
						}
					} else {
						OnError(__LINE__);
						error = TRUE;
					}
				} else if (n == 4) {
					// Parse parameters
					int i;
					for (i = 0; i < 4; i++) {
						checkVal += buf[i];
						buf[i] -= '=';
					}
					if (msg->nbPars < MAX_NB_PARS) {
						msg->pars[msg->nbPars] = (((buf[0] << 2) & 0xFF) | ((buf[1] >> 4)));
						msg->nbPars++;
					}
					if (msg->nbPars < MAX_NB_PARS) {
						msg->pars[msg->nbPars] = ((buf[1] & 0x0F) << 4 | (buf[2] >> 2));
						msg->nbPars++;
					}
					if (msg->nbPars < MAX_NB_PARS) {
						msg->pars[msg->nbPars] = ((buf[2] & 0x03) << 6 | buf[3]);
						msg->nbPars++;
					}
				} else {
					OnError(__LINE__);
					error = TRUE;
				}
			}
		}
	}

	return (done && !error);
}

static void SendMsg(const MkMsg_t * msg)
{
	uint8_t buf[10];
	uint16_t checkVal;
	uint8_t nbParsRemaining;
	const uint8_t *pPar;

	// Header
	buf[0] = '#';
	buf[1] = msg->address + 'a';
	buf[2] = msg->cmd;
	PIOS_COM_SendBuffer(PORT, buf, 3);
	checkVal = (unsigned int)'#' + buf[1] + buf[2];

	// Parameters
	nbParsRemaining = msg->nbPars;
	pPar = msg->pars;
	while (nbParsRemaining) {
		uint8_t a, b, c;

		a = *pPar;
		b = 0;
		c = 0;

		nbParsRemaining--;
		pPar++;
		if (nbParsRemaining) {
			b = *pPar;
			nbParsRemaining--;
			pPar++;
			if (nbParsRemaining) {
				c = *pPar;
				nbParsRemaining--;
				pPar++;
			}
		}

		buf[0] = (a >> 2) + '=';
		buf[1] = (((a & 0x03) << 4) | ((b & 0xf0) >> 4)) + '=';
		buf[2] = (((b & 0x0f) << 2) | ((c & 0xc0) >> 6)) + '=';
		buf[3] = (c & 0x3f) + '=';
		checkVal += buf[0];
		checkVal += buf[1];
		checkVal += buf[2];
		checkVal += buf[3];

		PIOS_COM_SendBuffer(PORT, buf, 4);
	}

	checkVal &= 0xFFF;
	buf[0] = (checkVal / 64) + '=';
	buf[1] = (checkVal % 64) + '=';
	buf[2] = '\r';
	PIOS_COM_SendBuffer(PORT, buf, 3);
}

static void SendMsgParNone(uint8_t address, uint8_t cmd)
{
	MkMsg_t msg;

	msg.address = address;
	msg.cmd = cmd;
	msg.nbPars = 0;

	SendMsg(&msg);
}

static void SendMsgPar8(uint8_t address, uint8_t cmd, uint8_t par0)
{
	MkMsg_t msg;

	msg.address = address;
	msg.cmd = cmd;
	msg.nbPars = 1;
	msg.pars[0] = par0;

	SendMsg(&msg);
}

static uint16_t VersionMsg_GetVersion(const MkMsg_t * msg)
{
	return msg->pars[0] * 100 + msg->pars[1];
}

static void DoConnectedToFC(void)
{
	AttitudeActualData attitudeData;
	MkMsg_t msg;

	DEBUG_MSG("FC\n\r");

	memset(&attitudeData, 0, sizeof(attitudeData));

	// Configure FC for fast reporting of the debug-message
	SendMsgPar8(MK_ADDR_ALL, MSGCMD_GET_DEBUG, 10);

	while (TRUE) {
		if (WaitForMsg(MSGCMD_DEBUG, &msg, 500 / portTICK_RATE_MS)) {
			int16_t nick;
			int16_t roll;

			//PrintMsg(&msg);
			nick = Par2Int16(&msg, DEBUG_MSG_NICK_IDX);
			roll = Par2Int16(&msg, DEBUG_MSG_ROLL_IDX);

			DEBUG_MSG("Att: Nick=%5d Roll=%5d\n\r", nick, roll);

			attitudeData.Pitch = -(float)nick / 10;
			attitudeData.Roll = -(float)roll / 10;
			AttitudeActualSet(&attitudeData);
		} else {
			DEBUG_MSG("TO\n\r");
			break;
		}
	}
}

static void DoConnectedToNC(void)
{
	MkMsg_t msg;
	GpsPosition_t pos;
	AttitudeActualData attitudeData;
	PositionActualData positionData;
	FlightBatteryStateData flightBatteryData;
#ifdef GENERATE_BATTERY_INFO
	uint8_t battStateCnt = 0;
#endif

	DEBUG_MSG("NC\n\r");

	memset(&attitudeData, 0, sizeof(attitudeData));
	memset(&positionData, 0, sizeof(positionData));
	memset(&flightBatteryData, 0, sizeof(flightBatteryData));

	// Configure NC for fast reporting of the osd-message
	SendMsgPar8(MK_ADDR_ALL, MSGCMD_GET_OSD, 10);

	while (TRUE) {
		if (WaitForMsg(MSGCMD_OSD, &msg, 500 / portTICK_RATE_MS)) {
			//PrintMsg(&msg);
			GetGpsPos(&msg, OSD_MSG_CURRPOS_IDX, &pos);

#if 0
			DEBUG_MSG("Bat=%d\n\r", msg.pars[OSD_MSG_BATT_IDX]);
			DEBUG_MSG("Nick=%d Roll=%d\n\r", Par2Int8(&msg, OSD_MSG_NICK_IDX), Par2Int8(&msg, OSD_MSG_ROLL_IDX));
			DEBUG_MSG("POS #Sats=%d stat=%d lat=%d lon=%d alt=%d\n\r", msg.pars[OSD_MSG_NB_SATS_IDX], pos.status, (int)pos.latitude,
				  (int)pos.longitute, (int)pos.altitude);
#else
			DEBUG_MSG(".");
#endif
			attitudeData.Pitch = -Par2Int8(&msg, OSD_MSG_NICK_IDX);
			attitudeData.Roll = -Par2Int8(&msg, OSD_MSG_ROLL_IDX);
			AttitudeActualSet(&attitudeData);

			positionData.Longitude = pos.longitute;
			positionData.Latitude = pos.latitude;
			positionData.Altitude = pos.altitude;
			positionData.Satellites = msg.pars[OSD_MSG_NB_SATS_IDX];
			positionData.Heading = Par2Int16(&msg, OSD_MSG_COMPHEADING_IDX);
			positionData.Groundspeed = ((float)Par2Int16(&msg, OSD_MSG_GNDSPEED_IDX)) / 100 /* cm/s => m/s */ ;
			if (positionData.Satellites < 5) {
				positionData.Status = POSITIONACTUAL_STATUS_NOFIX;
			} else {
				positionData.Status = POSITIONACTUAL_STATUS_FIX3D;
			}
			PositionActualSet(&positionData);

#if GENERATE_BATTERY_INFO
			if (++battStateCnt > 2) {
				flightBatteryData.Voltage = (float)msg.pars[OSD_MSG_BATT_IDX] / 10;
				FlightBatteryStateSet(&flightBatteryData);
				battStateCnt = 0;
			}
#endif
		} else {
			DEBUG_MSG("TO\n\r");
			break;
		}
	}
}

static void MkSerialTask(void *parameters)
{
	MkMsg_t msg;
	uint32_t version;
	bool connectionOk = FALSE;

	PIOS_COM_ChangeBaud(PORT, 57600);
	PIOS_COM_ChangeBaud(DEBUG_PORT, 57600);

	DEBUG_MSG("MKSerial Started\n\r");

	while (1) {
		// Wait until we get version from MK
		while (!connectionOk) {
			SendMsgParNone(MK_ADDR_ALL, MSGCMD_GET_VERSION);
			DEBUG_MSG("Version... ");
			if (WaitForMsg(MSGCMD_VERSION, &msg, 250 / portTICK_RATE_MS)) {
				version = VersionMsg_GetVersion(&msg);
				DEBUG_MSG("%d\n\r", version);
				connectionOk = TRUE;
			} else {
				DEBUG_MSG("TO\n\r");
			}
		}

		// Dependent on version, decide it we are connected to NC or FC
                // TODO: use slave-addr to distinguish FC/NC -> much safer 
		if (version < 60) {
			DoConnectedToNC();	// Will only return after an error
		} else {
			DoConnectedToFC();	// Will only return after an error
		}

		connectionOk = FALSE;

		vTaskDelay(250 / portTICK_RATE_MS);
	}
}

/**
 * Initialise the module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
int32_t MKSerialInitialize(void)
{
	// Start gps task
	xTaskCreate(MkSerialTask, (signed char *)"MkSerial", STACK_SIZE, NULL, TASK_PRIORITY, NULL);

	return 0;
}

/** 
  * @}
 * @}
 */
