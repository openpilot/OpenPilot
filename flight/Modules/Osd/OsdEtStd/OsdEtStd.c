/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup OSDModule OSD Module
 * @brief On screen display support
 * @{ 
 *
 * @file       OsdEtStd.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Interfacing with EagleTree OSD Std module
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

#include "flightbatterystate.h"
#include "gpsposition.h"
#include "attitudeactual.h"
#include "baroaltitude.h"

//
// Configuration
//
#define DEBUG_PORT		PIOS_COM_GPS
//#define ENABLE_DEBUG_MSG
//#define USE_DEBUG_PINS
//#define DUMP_CONFIG           // Enable this do read and dump the OSD config

//
// Private constants
//

#ifdef ENABLE_DEBUG_MSG
#define DEBUG_MSG(format, ...) PIOS_COM_SendFormattedString(DEBUG_PORT, format, ## __VA_ARGS__)
#else
#define DEBUG_MSG(format, ...)
#endif

#define CONFIG_LENGTH 6726
#define MIN(a,b) ((a)<(b)?(a):(b))

#define SUPPORTED_VERSION	115


#define OSD_ADDRESS         0x30

#define OSDMSG_V_LS_IDX		10
#define OSDMSG_BALT_IDX1	11
#define OSDMSG_BALT_IDX2	12
#define OSDMSG_A_LS_IDX		17
#define OSDMSG_VA_MS_IDX	18
#define OSDMSG_LAT_IDX		33
#define OSDMSG_LON_IDX		37
#define OSDMSG_HOME_IDX		47
#define OSDMSG_ALT_IDX		49
#define OSDMSG_NB_SATS		58
#define OSDMSG_GPS_STAT		59

#define OSDMSG_GPS_STAT_NOFIX	0x03
#define OSDMSG_GPS_STAT_FIX		0x2B
#define OSDMSG_GPS_STAT_HB_FLAG	0x10

#ifdef USE_DEBUG_PINS
	#define	DEBUG_PIN_RUNNING	0
	#define	DEBUG_PIN_I2C		1
	#define DebugPinHigh(x) PIOS_DEBUG_PinHigh(x)
	#define DebugPinLow(x)	PIOS_DEBUG_PinLow(x)
#else
	#define DebugPinHigh(x)
	#define DebugPinLow(x)
#endif


static const char *UpdateConfFilePath = "/etosd/update.ocf";
#ifdef DUMP_CONFIG
static const char *DumpConfFilePath = "/etosd/dump.ocf";
#endif

//
// Private types
//

//
// Private variables
//

//                        | Header / cmd?|                                  | V  |      E3                          |  V |                                                                     | LAT: 37 57.0000   | LONG: 24 00.4590  |                             | Hom<-179| Alt 545.4 m       |                        |#sat|stat|
//                           00   01   02   03   04   05   06   07   08   09   10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25   26   27   28   29   30   31   32   33   34   35   36   37   38   39   40   41   42   43   44   45   46   47   48   49   50   51   52   53   54   55   56   57   58   59   60   61   62
uint8_t msg[63] =
    { 0x03, 0x3F, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x0A, 0x00, 0xE4, 0x30, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x60, 0x10, 0x02, 0x00, 0x00, 0x90, 0x00,
0x54, 0x54, 0x00, 0x00, 0x33, 0x28, 0x13, 0x00, 0x00, 0x08, 0x00, 0x00, 0x90, 0x0A };
static volatile bool newPosData = FALSE;
static volatile bool newBattData = FALSE;
static volatile bool newBaroData = FALSE;

static enum
{
	STATE_DETECT,
	STATE_UPDATE_CONF,
	STATE_DUMP_CONF,
	STTE_RUNNING

} state;

static UAVObjEvent ev;
static uint32_t version = 0;


//
// Private functions
//
static void WriteToMsg8(uint8_t index, uint8_t value)
{
	if (value > 100)
		value = 100;

	msg[index] = ((value / 10) << 4) + (value % 10);
}

static void WriteToMsg16(uint8_t index, uint16_t value)
{
	WriteToMsg8(index, value % 100);
	WriteToMsg8(index + 1, value / 100);
}

static void WriteToMsg24(uint8_t index, uint32_t value)
{
	WriteToMsg16(index, value % 10000);
	WriteToMsg8(index + 2, value / 10000);
}

static void WriteToMsg32(uint8_t index, uint32_t value)
{
	WriteToMsg16(index, value % 10000);
	WriteToMsg16(index + 2, value / 10000);
}

static void SetCoord(uint8_t index, uint32_t coord)
{

#define E7 10000000
	uint8_t deg = coord / E7;
	float sec = (float)(coord - deg*E7) / ((float)E7/(60.0*10000));

	WriteToMsg8(index + 3, deg);
	WriteToMsg24(index, sec);

}

static void SetCourse(uint16_t dir)
{
	WriteToMsg16(OSDMSG_HOME_IDX, dir);
}

static void SetBaroAltitude(int16_t altitudeMeter)
{
	// calculated formula
	// ET OSD uses first update as zeropoint and then +- from that
	altitudeMeter=(4571-altitudeMeter)/0.37;
	msg[OSDMSG_BALT_IDX1] = (uint8_t)(altitudeMeter&0x00FF);
	msg[OSDMSG_BALT_IDX2] = (altitudeMeter >> 8)&0x3F;
}


static void SetAltitude(uint32_t altitudeMeter)
{
	WriteToMsg32(OSDMSG_ALT_IDX, altitudeMeter * 10);
}

static void SetVoltage(uint32_t milliVolt)
{
	msg[OSDMSG_VA_MS_IDX] &= 0x0F;
	msg[OSDMSG_VA_MS_IDX] |= (milliVolt / 6444) << 4;
	msg[OSDMSG_V_LS_IDX] = (milliVolt % 6444) * 256 / 6444;
}

static void SetCurrent(uint32_t milliAmp)
{
	uint32_t value = (milliAmp * 16570 / 1000000) + 0x7FA;
	msg[OSDMSG_VA_MS_IDX] &= 0xF0;
	msg[OSDMSG_VA_MS_IDX] |= ((value >> 8) & 0x0F);
	msg[OSDMSG_A_LS_IDX] = (value & 0xFF);
}

static void SetNbSats(uint8_t nb)
{
	msg[OSDMSG_NB_SATS] = nb;
}

static void FlightBatteryStateUpdatedCb(UAVObjEvent * ev)
{
	newBattData = TRUE;
}

static void GPSPositionUpdatedCb(UAVObjEvent * ev)
{
	newPosData = TRUE;
}

static void BaroAltitudeUpdatedCb(UAVObjEvent * ev)
{
	newBaroData = TRUE;
}

static bool Read(uint32_t start, uint8_t length, uint8_t * buffer)
{
	uint8_t cmd[5];

	const struct pios_i2c_txn txn_list[] = {
		{
		 .addr = OSD_ADDRESS,
		 .rw = PIOS_I2C_TXN_WRITE,
		 .len = sizeof(cmd),
		 .buf = cmd,
		 }
		,
		{
		 .addr = OSD_ADDRESS,
		 .rw = PIOS_I2C_TXN_READ,
		 .len = length,
		 .buf = buffer,
		 }
		,
	};

	cmd[0] = 0x02;
	cmd[1] = 0x05;
	cmd[2] = (uint8_t) (start & 0xFF);
	cmd[3] = (uint8_t) (start >> 8);
	cmd[4] = length;

	return PIOS_I2C_Transfer(PIOS_I2C_MAIN_ADAPTER, txn_list, NELEMENTS(txn_list));
}

static bool Write(uint32_t start, uint8_t length, const uint8_t * buffer)
{
	uint8_t cmd[125];
	uint8_t ack[3];

	const struct pios_i2c_txn txn_list1[] = {
		{
		 .addr = OSD_ADDRESS,
		 .rw = PIOS_I2C_TXN_WRITE,
		 .len = sizeof(cmd),
		 .buf = cmd,
		 }
		,
		{
		 .addr = OSD_ADDRESS,
		 .rw = PIOS_I2C_TXN_READ,
		 .len = sizeof(ack),
		 .buf = ack,
		 }
		,
	};

	if (length + 5 > sizeof(cmd)) {
		// Too big
		return FALSE;
	}

	cmd[0] = 0x01;
	cmd[1] = 0x7D;
	cmd[2] = (uint8_t) (start & 0xFF);
	cmd[3] = (uint8_t) (start >> 8);
	cmd[4] = length;
	memcpy(&cmd[5], buffer, length);

	ack[0] = 0;

	//
	// FIXME: See OP-305, the driver seems to return FALSE while all is OK
	//
	PIOS_I2C_Transfer(PIOS_I2C_MAIN_ADAPTER, txn_list1, NELEMENTS(txn_list1));
//	if (PIOS_I2C_Transfer(PIOS_I2C_MAIN_ADAPTER, txn_list1, NELEMENTS(txn_list1))) {
		//DEBUG_MSG("ACK=%d ", ack[0]);
		if (ack[0] == 49) {
			return TRUE;
		}
//	}

	return FALSE;
}

static uint32_t ReadSwVersion(void)
{
	uint8_t buf[4];
	uint32_t version;

	if (Read(0, 4, buf)) {
		version = (buf[0] - '0') * 100;
		version += (buf[2] - '0') * 10;
		version += (buf[3] - '0');
	} else {
		version = 0;
	}

	return version;
}

static void UpdateConfig(void)
{
	uint8_t buf[120];
	uint32_t addr = 0;
	uint32_t n;
	FILEINFO file;
	uint32_t res;

	// Try to open the file that contains a new config
	res = DFS_OpenFile(&PIOS_SDCARD_VolInfo, (uint8_t *) UpdateConfFilePath, DFS_READ, PIOS_SDCARD_Sector, &file);
	if (res == DFS_OK) {
		uint32_t bytesRead;
		bool ok = TRUE;

		DEBUG_MSG("Updating Config ");

		// Write the config-data in blocks to OSD
		while (addr < CONFIG_LENGTH && ok) {
			n = MIN(CONFIG_LENGTH - addr, sizeof(buf));
			res = DFS_ReadFile(&file, PIOS_SDCARD_Sector, buf, &bytesRead, n);
			if (res == DFS_OK && bytesRead == n) {
				ok = Write(addr, n, buf);
				if (ok) {
					//DEBUG_MSG(" %d %d\n", addr, n);
					DEBUG_MSG(".");
					addr += n;
				}
			} else {
				DEBUG_MSG(" FILEREAD FAILED ");
			}
		}

		DEBUG_MSG(ok ? " OK\n" : "FAILURE\n");

		// If writing is OK, read the data back and verify
		if (ok) {
			DEBUG_MSG("Verify Config ");
			DFS_Seek(&file, 0, PIOS_SDCARD_Sector);

			addr = 0;
			while (addr < CONFIG_LENGTH && ok) {
				// First half of the buffer is used to store the data read from the OSD, the second half will contain the data from the file
				n = MIN(CONFIG_LENGTH - addr, sizeof(buf) / 2);
				ok = Read(addr, n, buf);
				if (ok) {
					uint32_t bytesRead;
					res = DFS_ReadFile(&file, PIOS_SDCARD_Sector, buf + sizeof(buf) / 2, &bytesRead, n);
					if (res == DFS_OK && bytesRead == n) {
						DEBUG_MSG(".");
						addr += n;

						if (memcmp(buf, buf + sizeof(buf) / 2, n) != 0) {
							DEBUG_MSG(" MISMATCH ");
							ok = FALSE;
						}
					}
				}
			}
			DEBUG_MSG(ok ? " OK\n" : "FAILURE\n");
		}

		DFS_Close(&file);

		// When the config was updated correctly, remove the config-file
		if (ok) {
			DFS_UnlinkFile(&PIOS_SDCARD_VolInfo, (uint8_t *) UpdateConfFilePath, PIOS_SDCARD_Sector);
		}

	}
}

static void DumpConfig(void)
{
#ifdef DUMP_CONFIG
	uint8_t buf[100];
	uint32_t addr = 0;
	uint32_t n;
	FILEINFO file;
	uint32_t res;

	DEBUG_MSG("Dumping Config ");

	res = DFS_OpenFile(&PIOS_SDCARD_VolInfo, (uint8_t *) DumpConfFilePath, DFS_WRITE, PIOS_SDCARD_Sector, &file);
	if (res == DFS_OK) {
		uint32_t bytesWritten;
		bool ok = TRUE;

		while (addr < CONFIG_LENGTH && ok) {
			n = MIN(CONFIG_LENGTH - addr, sizeof(buf));
			ok = Read(addr, n, buf);
			if (ok) {
				res = DFS_WriteFile(&file, PIOS_SDCARD_Sector, buf, &bytesWritten, n);
				if (res == DFS_OK && bytesWritten == n) {
					//DEBUG_MSG(" %d %d\n", addr, n);
					DEBUG_MSG(".");
					addr += n;
				}
			}
		}

		DEBUG_MSG(ok ? " OK\n" : "FAILURE\n");

		DFS_Close(&file);
	} else {
		DEBUG_MSG("Error Opening File %x\n", res);
	}
#endif
}

static void Run(void)
{
	static uint32_t cnt = 0;

	if (newBattData) {
		FlightBatteryStateData flightBatteryData;

		FlightBatteryStateGet(&flightBatteryData);

		//DEBUG_MSG("%5d Batt: V=%dmV\n\r", cnt, (uint32_t)(flightBatteryData.Voltage*1000));

		SetVoltage((uint32_t) (flightBatteryData.Voltage * 1000));
		SetCurrent((uint32_t) (flightBatteryData.Current * 1000));
		newBattData = FALSE;
	}

	if (newPosData) {
		GPSPositionData positionData;
		AttitudeActualData attitudeActualData;

		GPSPositionGet(&positionData);
		AttitudeActualGet(&attitudeActualData);

		//DEBUG_MSG("%5d Pos: #stat=%d #sats=%d alt=%d\n\r", cnt,
		//              positionData.Status, positionData.Satellites, (uint32_t)positionData.Altitude);

		// GPS Status
		if (positionData.Status == GPSPOSITION_STATUS_FIX3D)
			msg[OSDMSG_GPS_STAT] = OSDMSG_GPS_STAT_FIX;
		else
			msg[OSDMSG_GPS_STAT] = OSDMSG_GPS_STAT_NOFIX;
		msg[OSDMSG_GPS_STAT] |= OSDMSG_GPS_STAT_HB_FLAG;

		// GPS info
		SetCoord(OSDMSG_LAT_IDX, positionData.Latitude);
		SetCoord(OSDMSG_LON_IDX, positionData.Longitude);
		SetAltitude(positionData.Altitude);
		SetNbSats(positionData.Satellites);
		SetCourse(attitudeActualData.Yaw);

		newPosData = FALSE;
	} else {
		msg[OSDMSG_GPS_STAT] &= ~OSDMSG_GPS_STAT_HB_FLAG;
	}
	if (newBaroData) {
		BaroAltitudeData baroData;

		BaroAltitudeGet(&baroData);
		SetBaroAltitude(baroData.Altitude);

		newBaroData = FALSE;
	}

	DEBUG_MSG("SendMsg %d\n",cnt);
	{
		DebugPinHigh(DEBUG_PIN_I2C);
		const struct pios_i2c_txn txn_list[] = {
			{
			 .addr = OSD_ADDRESS,
			 .rw = PIOS_I2C_TXN_WRITE,
			 .len = sizeof(msg),
			 .buf = msg,
			 }
			,
		};

		PIOS_I2C_Transfer(PIOS_I2C_MAIN_ADAPTER, txn_list, NELEMENTS(txn_list));
		DebugPinLow(DEBUG_PIN_I2C);
	}

	cnt++;
}

static void onTimer(UAVObjEvent * ev)
{
	DebugPinHigh(DEBUG_PIN_RUNNING);

	#ifdef ENABLE_DEBUG_MSG
		PIOS_COM_ChangeBaud(DEBUG_PORT, 57600);
	#endif

	if (state == STATE_DETECT) {
		version = ReadSwVersion();
		DEBUG_MSG("SW: %d ", version);

		if (version == SUPPORTED_VERSION) {
			DEBUG_MSG("OK\n");
			state++;
		} else {
			DEBUG_MSG("INVALID\n");
		}
	} else if (state == STATE_UPDATE_CONF) {
		UpdateConfig();
		state++;
	} else if (state == STATE_DUMP_CONF) {
		DumpConfig();
		state++;
	} else if (state == STTE_RUNNING) {
		Run();
	} else {
		// should not happen..
		state = STATE_DETECT;
	}

	DebugPinLow(DEBUG_PIN_RUNNING);

}

//
// Public functions
//


/**
 * Initialise the module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
int32_t OsdEtStdInitialize(void)
{
	GPSPositionConnectCallback(GPSPositionUpdatedCb);
	FlightBatteryStateConnectCallback(FlightBatteryStateUpdatedCb);
	BaroAltitudeConnectCallback(BaroAltitudeUpdatedCb);

	memset(&ev,0,sizeof(UAVObjEvent));
	EventPeriodicCallbackCreate(&ev, onTimer, 100 / portTICK_RATE_MS);

	return 0;
}
