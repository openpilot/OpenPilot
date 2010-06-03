/**
 ******************************************************************************
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

#include "attitudeactual.h" // object that will be updated by the module

//
// Private constants
//
#define PORT			COM_USART1
#define STACK_SIZE		1024
#define TASK_PRIORITY	(tskIDLE_PRIORITY + 3)
#define MAX_NB_PARS 	40

#define DEBUG_PORT		COM_USART2

#define DEBUG_MSG(format, ...) PIOS_COM_SendFormattedString(DEBUG_PORT, format, ## __VA_ARGS__)

#define MSGCMD_ANY				0
#define MSGCMD_GET_DEBUG		'd'
#define MSGCMD_DEBUG			'D'
#define MSGCMD_GET_VERSION		'v'
#define MSGCMD_VERSION			'V'

#define DEBUG_MSG_NICK_IDX	(2+2*2)
#define DEBUG_MSG_ROLL_IDX	(2+3*2)


//
// Private types
//
typedef struct
{
	uint8_t address;
	uint8_t cmd;
	uint8_t nbPars;
	uint8_t pars[MAX_NB_PARS];
} MkMsg_t;

enum
{
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
static uint8_t WaitForBytes(uint8_t* buf, uint8_t nbBytes, portTickType xTicksToWait);
static void MkSerialTask(void* parameters);

static void OnError(int line)
{
	DEBUG_MSG("MKProcol error %d\n", line);
}

void PrintMsg(const MkMsg_t* msg)
{
	switch (msg->address)
	{
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

	for (int i = 0; i < msg->nbPars; i++)
	{
		DEBUG_MSG("%02x ", msg->pars[i]);
	}
	DEBUG_MSG("\n");

}

static int16_t Par2SignedInt(const MkMsg_t* msg, uint8_t index)
{
	int16_t res;

	res = (int) (msg->pars[index + 1]) * 256 + msg->pars[index];
	if (res > 0xFFFF / 2)
		res -= 0xFFFF;
	return res;
}

static uint8_t WaitForBytes(uint8_t* buf, uint8_t nbBytes, portTickType xTicksToWait)
{
	uint8_t nbBytesLeft = nbBytes;
	xTimeOutType xTimeOut;

	vTaskSetTimeOutState(&xTimeOut);

	// Loop until
	// - all bytes are received
	// - \r is seen
	// - Timeout occurs
	do
	{
		// Check if timeout occured
		if (xTaskCheckForTimeOut(&xTimeOut, &xTicksToWait))
			break;

		// Check if there are some bytes
		if (PIOS_COM_ReceiveBufferUsed(PORT))
		{
			*buf = PIOS_COM_ReceiveBuffer(PORT);

			nbBytesLeft--;
			if (buf[0] == '\r')
				break;
			buf++;
		}
		else
		{
			// Avoid tight loop
			// FIXME: should be blocking
			vTaskDelay(5);
		}
	} while (nbBytesLeft);

	return nbBytes - nbBytesLeft;
}

bool WaitForMsg(uint8_t cmd, MkMsg_t* msg)
{
	uint8_t buf[10];
	uint8_t n;
	bool done = FALSE;
	bool error = FALSE;
	unsigned int checkVal;


	while (!done && !error)
	{
		// Wait for start
		buf[0] = 0;
		do
		{
			WaitForBytes(buf, 1, 100 / portTICK_RATE_MS);
			// TODO: check to TO
		} while (buf[0] != '#');

		// Wait for cmd and address
		if (WaitForBytes(buf, 2, 10 / portTICK_RATE_MS) != 2)
		{
			OnError(__LINE__);
			continue;
		}

		// Is this the command we are waiting for?
		if (cmd == 0 || cmd == buf[1])
		{
			// OK follow this message to the end
			msg->address = buf[0] - 'a';
			msg->cmd = buf[1];

			checkVal = '#' + buf[0] + buf[1];

			// Parse parameters
			msg->nbPars = 0;
			while (!done && !error)
			{
				n = WaitForBytes(buf, 4, 10 / portTICK_RATE_MS);
				if (n > 0 && buf[n - 1] == '\r')
				{
					n--;
					// This is the end of the message
					// Get check bytes
					if (n >= 2)
					{
						unsigned int msgCeckVal;
						msgCeckVal = (buf[n-1]-'=') + (buf[n-2]-'=')*64;
						//printf("%x %x\n", msgCeckVal, checkVal&0xFFF);
						n -= 2;

						if (msgCeckVal == (checkVal & 0xFFF))
						{
							done = TRUE;
						}
						else
						{
							OnError(__LINE__);
							error = TRUE;
						}
					}
					else
					{
						OnError(__LINE__);
						error = TRUE;
					}
				}
				else if (n == 4)
				{
					// Parse parameters
					int i;
					for (i = 0; i < 4; i++)
					{
						checkVal += buf[i];
						buf[i] -= '=';
					}
					if (msg->nbPars < MAX_NB_PARS)
					{
						msg->pars[msg->nbPars] = (((buf[0]<<2)&0xFF) | ((buf[1] >> 4)));
						msg->nbPars++;
					}
					if (msg->nbPars < MAX_NB_PARS)
					{
						msg->pars[msg->nbPars] = ((buf[1] & 0x0F)<< 4 | (buf[2] >> 2));
						msg->nbPars++;
					}
					if (msg->nbPars < MAX_NB_PARS)
					{
						msg->pars[msg->nbPars] = ((buf[2] & 0x03)<< 6 | buf[3]);
						msg->nbPars++;
					}
				}
				else
				{
					OnError(__LINE__);
					error = TRUE;
				}
			}
		}
	}

	return (done && !error);
}

void SendMsg(const MkMsg_t* msg)
{
	uint8_t buf[10];
	uint16_t checkVal;
	uint8_t nbParsRemaining;
	const uint8_t* pPar;

	// Header
	buf[0] = '#';
	buf[1] = msg->address + 'a';
	buf[2] = msg->cmd;
	PIOS_COM_SendBuffer(PORT, buf, 3);
	checkVal = (unsigned int) '#' + buf[1] + buf[2];

	// Parameters
	nbParsRemaining = msg->nbPars;
	pPar = msg->pars;
	while (nbParsRemaining)
	{
		uint8_t a, b, c;

		a = *pPar;
		b = 0;
		c = 0;

		nbParsRemaining--;
		pPar++;
		if (nbParsRemaining)
		{
			b = *pPar;
			nbParsRemaining--;
			pPar++;
			if (nbParsRemaining)
			{
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

void SendMsgParNone(uint8_t address, uint8_t cmd)
{
	MkMsg_t msg;

	msg.address = address;
	msg.cmd = cmd;
	msg.nbPars = 0;

	SendMsg(&msg);
}

void SendMsgPar8(uint8_t address, uint8_t cmd, uint8_t par0)
{
	MkMsg_t msg;

	msg.address = address;
	msg.cmd = cmd;
	msg.nbPars = 1;
	msg.pars[0] = par0;

	SendMsg(&msg);
}

uint16_t VersionMsg_GetVersion(const MkMsg_t* msg)
{
	return msg->pars[0] * 100 + msg->pars[1];
}

/**
 * Initialise the module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
int32_t MKSerialInitialize(void)
{
	// Start gps task
	xTaskCreate(MkSerialTask, (signed char*) "MkSerial", STACK_SIZE, NULL,
			TASK_PRIORITY, NULL);

	return 0;
}

/**
 * gps task. Processes input buffer. It does not return.
 */
static void MkSerialTask(void* parameters)
{
	MkMsg_t msg;
	AttitudeActualData data;

	PIOS_COM_ChangeBaud(PORT, 57600);
	PIOS_COM_ChangeBaud(DEBUG_PORT, 57600);

	DEBUG_MSG("MKSerial Started\n");

	SendMsgParNone(MK_ADDR_ALL, MSGCMD_GET_VERSION);
	if (WaitForMsg(MSGCMD_VERSION, &msg))
	{
		//PrintMsg(&msg);
		DEBUG_MSG("Version = %d\n", VersionMsg_GetVersion(&msg));
	}

	SendMsgPar8(MK_ADDR_ALL, MSGCMD_GET_DEBUG, 10);

	memset(&data, 0, sizeof(data));

	while (1)
	{
		if (WaitForMsg(MSGCMD_DEBUG, &msg))
		{
			uint16_t nick;
			uint16_t roll;

			//PrintMsg(&msg);
			nick = Par2SignedInt(&msg, DEBUG_MSG_NICK_IDX);
			roll = Par2SignedInt(&msg, DEBUG_MSG_ROLL_IDX);

			DEBUG_MSG("Att: Nick=%5d Roll=%5d\n", nick, roll);

			data.seq++;
			data.Pitch = (float)nick/10;
			data.Roll = (float)roll/10;
			AttitudeActualSet(&data);
		}
		else
		{
			DEBUG_MSG("NoMsg\n");
		}
	}
}
