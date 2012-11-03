/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup GSPModule GPS Module
 * @brief Process GPS information
 * @{ 
 *
 * @file       GPS.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      GPS module, handles GPS and NMEA stream
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

// ****************

#include "openpilot.h"
#include "GPS.h"

#include "gpsposition.h"
#include "homelocation.h"
#include "gpstime.h"
#include "gpssatellites.h"
#include "gpsvelocity.h"
#include "systemsettings.h"
#include "WorldMagModel.h"
#include "CoordinateConversions.h"
#include "hwsettings.h"

#include "NMEA.h"
#include "UBX.h"


// ****************
// Private functions

static void gpsTask(void *parameters);
static void updateSettings();

#ifdef PIOS_GPS_SETS_HOMELOCATION
static void setHomeLocation(GPSPositionData * gpsData);
static float GravityAccel(float latitude, float longitude, float altitude);
#endif

// ****************
// Private constants

#define GPS_TIMEOUT_MS                  500


#ifdef PIOS_GPS_SETS_HOMELOCATION
// Unfortunately need a good size stack for the WMM calculation
	#define STACK_SIZE_BYTES            850
#else
#if defined(PIOS_GPS_MINIMAL)
	#define STACK_SIZE_BYTES            500
#else
	#define STACK_SIZE_BYTES            650
#endif // PIOS_GPS_MINIMAL
#endif // PIOS_GPS_SETS_HOMELOCATION

#define TASK_PRIORITY                   (tskIDLE_PRIORITY + 1)

// ****************
// Private variables

static uint32_t gpsPort;
static bool gpsEnabled = false;

static xTaskHandle gpsTaskHandle;

static char* gps_rx_buffer;

static uint32_t timeOfLastCommandMs;
static uint32_t timeOfLastUpdateMs;

static struct GPS_RX_STATS gpsRxStats;

// ****************
/**
 * Initialise the gps module
 * \return -1 if initialisation failed
 * \return 0 on success
 */

int32_t GPSStart(void)
{
	if (gpsEnabled) {
		if (gpsPort) {
			// Start gps task
			xTaskCreate(gpsTask, (signed char *)"GPS", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &gpsTaskHandle);
			TaskMonitorAdd(TASKINFO_RUNNING_GPS, gpsTaskHandle);
			return 0;
		}

		AlarmsSet(SYSTEMALARMS_ALARM_GPS, SYSTEMALARMS_ALARM_CRITICAL);
	}
	return -1;
}

/**
 * Initialise the gps module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
int32_t GPSInitialize(void)
{
	gpsPort = PIOS_COM_GPS;
	uint8_t	gpsProtocol;

#ifdef MODULE_GPS_BUILTIN
	gpsEnabled = true;
#else
	HwSettingsInitialize();
	uint8_t optionalModules[HWSETTINGS_OPTIONALMODULES_NUMELEM];

	HwSettingsOptionalModulesGet(optionalModules);

	if (optionalModules[HWSETTINGS_OPTIONALMODULES_GPS] == HWSETTINGS_OPTIONALMODULES_ENABLED)
		gpsEnabled = true;
	else
		gpsEnabled = false;
#endif

#if defined(REVOLUTION)
	// These objects MUST be initialized for Revolution
	// because the rest of the system expects to just
	// attach to their queues
	GPSPositionInitialize();
	GPSVelocityInitialize();
	GPSTimeInitialize();
	GPSSatellitesInitialize();
	HomeLocationInitialize();
	updateSettings();

#else
	if (gpsPort && gpsEnabled) {
		GPSPositionInitialize();
		GPSVelocityInitialize();
#if !defined(PIOS_GPS_MINIMAL)
		GPSTimeInitialize();
		GPSSatellitesInitialize();
#endif
#ifdef PIOS_GPS_SETS_HOMELOCATION
		HomeLocationInitialize();
#endif
		updateSettings();
	}
#endif

	if (gpsPort && gpsEnabled) {
		SystemSettingsInitialize();
		SystemSettingsGPSDataProtocolGet(&gpsProtocol);
		switch (gpsProtocol) {
			case SYSTEMSETTINGS_GPSDATAPROTOCOL_NMEA:
				gps_rx_buffer = pvPortMalloc(NMEA_MAX_PACKET_LENGTH);
				break;
			case SYSTEMSETTINGS_GPSDATAPROTOCOL_UBX:
				gps_rx_buffer = pvPortMalloc(sizeof(struct UBXPacket));
				break;
			default:
				gps_rx_buffer = NULL;
		}
		PIOS_Assert(gps_rx_buffer);

		return 0;
	}

	return -1;
}

MODULE_INITCALL(GPSInitialize, GPSStart)

// ****************
/**
 * Main gps task. It does not return.
 */

static void gpsTask(void *parameters)
{
	portTickType xDelay = 100 / portTICK_RATE_MS;
	uint32_t timeNowMs = xTaskGetTickCount() * portTICK_RATE_MS;

	GPSPositionData gpsposition;
	uint8_t	gpsProtocol;

	SystemSettingsGPSDataProtocolGet(&gpsProtocol);

	timeOfLastUpdateMs = timeNowMs;
	timeOfLastCommandMs = timeNowMs;

	GPSPositionGet(&gpsposition);
	// Loop forever
	while (1)
	{
		uint8_t c;

		// This blocks the task until there is something on the buffer
		while (PIOS_COM_ReceiveBuffer(gpsPort, &c, 1, xDelay) > 0)
		{
			int res;
			switch (gpsProtocol) {
#if defined(PIOS_INCLUDE_GPS_NMEA_PARSER)
				case SYSTEMSETTINGS_GPSDATAPROTOCOL_NMEA:
					res = parse_nmea_stream (c,gps_rx_buffer, &gpsposition, &gpsRxStats);
					break;
#endif
#if defined(PIOS_INCLUDE_GPS_UBX_PARSER)
				case SYSTEMSETTINGS_GPSDATAPROTOCOL_UBX:
					res = parse_ubx_stream (c,gps_rx_buffer, &gpsposition, &gpsRxStats);
					break;
#endif
				default:
					res = NO_PARSER; // this should not happen
					break;
			}

			if (res == PARSER_COMPLETE) {
				timeNowMs = xTaskGetTickCount() * portTICK_RATE_MS;
				timeOfLastUpdateMs = timeNowMs;
				timeOfLastCommandMs = timeNowMs;
			}
		}

		// Check for GPS timeout
		timeNowMs = xTaskGetTickCount() * portTICK_RATE_MS;
		if ((timeNowMs - timeOfLastUpdateMs) >= GPS_TIMEOUT_MS) {
			// we have not received any valid GPS sentences for a while.
			// either the GPS is not plugged in or a hardware problem or the GPS has locked up.
			uint8_t status = GPSPOSITION_STATUS_NOGPS;
			GPSPositionStatusSet(&status);
			AlarmsSet(SYSTEMALARMS_ALARM_GPS, SYSTEMALARMS_ALARM_ERROR);
		} else {
			// we appear to be receiving GPS sentences OK, we've had an update
			//criteria for GPS-OK taken from this post...
			//http://forums.openpilot.org/topic/1523-professors-insgps-in-svn/page__view__findpost__p__5220
			if ((gpsposition.PDOP < 3.5) && (gpsposition.Satellites >= 7) &&
					(gpsposition.Status == GPSPOSITION_STATUS_FIX3D)) {
				AlarmsClear(SYSTEMALARMS_ALARM_GPS);
#ifdef PIOS_GPS_SETS_HOMELOCATION
				HomeLocationData home;
				HomeLocationGet(&home);

				if (home.Set == HOMELOCATION_SET_FALSE)
					setHomeLocation(&gpsposition);
#endif
			} else if (gpsposition.Status == GPSPOSITION_STATUS_FIX3D)
						AlarmsSet(SYSTEMALARMS_ALARM_GPS, SYSTEMALARMS_ALARM_WARNING);
					else
						AlarmsSet(SYSTEMALARMS_ALARM_GPS, SYSTEMALARMS_ALARM_CRITICAL);
		}

	}
}

#ifdef PIOS_GPS_SETS_HOMELOCATION
/*
 * Estimate the acceleration due to gravity for a particular location in LLA
 */
static float GravityAccel(float latitude, float longitude, float altitude)
{
	// WGS84 gravity model.  The effect of gravity over latitude is strong
	// enough to change the estimated accelerometer bias in those apps.
	double sinsq = sin((double)latitude);
	sinsq *= sinsq;
	// Likewise, over the altitude range of a high-altitude balloon, the effect
	// due to change in altitude can also affect the model.
	return (float)(9.7803267714 * (1 + 0.00193185138639*sinsq) / sqrt(1 - 0.00669437999013*sinsq)
		- 3.086e-6*altitude);
}

// ****************

static void setHomeLocation(GPSPositionData * gpsData)
{
	HomeLocationData home;
	HomeLocationGet(&home);
	GPSTimeData gps;
	GPSTimeGet(&gps);

	if (gps.Year >= 2000)
	{
		// Store LLA
		home.Latitude = gpsData->Latitude;
		home.Longitude = gpsData->Longitude;
		home.Altitude = gpsData->Altitude + gpsData->GeoidSeparation;

		// Compute home ECEF coordinates and the rotation matrix into NED
		double LLA[3] = { ((double)home.Latitude) / 10e6, ((double)home.Longitude) / 10e6, ((double)home.Altitude) };

		// Compute magnetic flux direction at home location
		if (WMM_GetMagVector(LLA[0], LLA[1], LLA[2], gps.Month, gps.Day, gps.Year, &home.Be[0]) >= 0)
		{   // calculations appeared to go OK

			// Compute local acceleration due to gravity.  Vehicles that span a very large
			// range of altitude (say, weather balloons) may need to update this during the
			// flight.
			home.g_e = GravityAccel(LLA[0], LLA[1], LLA[2]);
			home.Set = HOMELOCATION_SET_TRUE;
			HomeLocationSet(&home);
		}
	}
}
#endif

/**
 * Update the GPS settings, called on startup.
 * FIXME: This should be in the GPSSettings object. But objects have
 * too much overhead yet. Also the GPS has no any specific settings
 * like protocol, etc. Thus the HwSettings object which contains the
 * GPS port speed is used for now.
 */
static void updateSettings()
{
	if (gpsPort) {

		// Retrieve settings
		uint8_t speed;
		HwSettingsGPSSpeedGet(&speed);

		// Set port speed
		switch (speed) {
		case HWSETTINGS_GPSSPEED_2400:
			PIOS_COM_ChangeBaud(gpsPort, 2400);
			break;
		case HWSETTINGS_GPSSPEED_4800:
			PIOS_COM_ChangeBaud(gpsPort, 4800);
			break;
		case HWSETTINGS_GPSSPEED_9600:
			PIOS_COM_ChangeBaud(gpsPort, 9600);
			break;
		case HWSETTINGS_GPSSPEED_19200:
			PIOS_COM_ChangeBaud(gpsPort, 19200);
			break;
		case HWSETTINGS_GPSSPEED_38400:
			PIOS_COM_ChangeBaud(gpsPort, 38400);
			break;
		case HWSETTINGS_GPSSPEED_57600:
			PIOS_COM_ChangeBaud(gpsPort, 57600);
			break;
		case HWSETTINGS_GPSSPEED_115200:
			PIOS_COM_ChangeBaud(gpsPort, 115200);
			break;
		}
	}
}

/** 
  * @}
  * @}
  */
