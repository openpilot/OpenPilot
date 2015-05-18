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

#include <openpilot.h>

#include "gpspositionsensor.h"
#include "homelocation.h"
#include "gpstime.h"
#include "gpssatellites.h"
#include "gpsvelocitysensor.h"
#include "gpssettings.h"
#include "taskinfo.h"
#include "hwsettings.h"
#include "auxmagsensor.h"
#include "WorldMagModel.h"
#include "CoordinateConversions.h"
#include <pios_com.h>

#include "GPS.h"
#include "NMEA.h"
#include "UBX.h"
#if defined(PIOS_INCLUDE_GPS_UBX_PARSER) && !defined(PIOS_GPS_MINIMAL)
#include "inc/ubx_autoconfig.h"
// #include "../../libraries/inc/fifo_buffer.h"
#endif

#include <pios_instrumentation_helper.h>
PERF_DEFINE_COUNTER(counterBytesIn);
PERF_DEFINE_COUNTER(counterRate);
PERF_DEFINE_COUNTER(counterParse);
// ****************
// Private functions

static void gpsTask(__attribute__((unused)) void *parameters);
static void updateHwSettings(__attribute__((unused)) UAVObjEvent *ev);

#ifdef PIOS_GPS_SETS_HOMELOCATION
static void setHomeLocation(GPSPositionSensorData *gpsData);
static float GravityAccel(float latitude, float longitude, float altitude);
#endif

#ifdef PIOS_INCLUDE_GPS_UBX_PARSER
void AuxMagSettingsUpdatedCb(__attribute__((unused)) UAVObjEvent *ev);
#ifndef PIOS_GPS_MINIMAL
void updateGpsSettings(__attribute__((unused)) UAVObjEvent *ev);
#endif
#endif

// ****************
// Private constants

#define GPS_TIMEOUT_MS             500
// delay from detecting HomeLocation.Set == False before setting new homelocation
// this prevent that a save with homelocation.Set = false triggered by gps ends saving
// the new location with Set = true.
#define GPS_HOMELOCATION_SET_DELAY 5000

#define GPS_LOOP_DELAY_MS          6

#ifdef PIOS_GPS_SETS_HOMELOCATION
// Unfortunately need a good size stack for the WMM calculation
        #define STACK_SIZE_BYTES   1024+64
#else
#if defined(PIOS_GPS_MINIMAL)
        #define GPS_READ_BUFFER    32

#ifdef PIOS_INCLUDE_GPS_NMEA_PARSER
        #define STACK_SIZE_BYTES   580+64 // NMEA
#else
        #define STACK_SIZE_BYTES   440+64 // UBX
#endif // PIOS_INCLUDE_GPS_NMEA_PARSER
#else
        #define STACK_SIZE_BYTES   650+64
#endif // PIOS_GPS_MINIMAL
#endif // PIOS_GPS_SETS_HOMELOCATION

#ifndef GPS_READ_BUFFER
#define GPS_READ_BUFFER            128
#endif

#define TASK_PRIORITY              (tskIDLE_PRIORITY + 1)

// ****************
// Private variables

static GPSSettingsData gpsSettings;

//static uint32_t gpsPort;
#define gpsPort PIOS_COM_GPS
static bool gpsEnabled = false;

static xTaskHandle gpsTaskHandle;

static char *gps_rx_buffer;

static uint32_t timeOfLastCommandMs;
static uint32_t timeOfLastUpdateMs;

#if defined(PIOS_INCLUDE_GPS_NMEA_PARSER) || defined(PIOS_INCLUDE_GPS_UBX_PARSER)
static struct GPS_RX_STATS gpsRxStats;
#endif

// ****************
/**
 * Initialise the gps module
 * \return -1 if initialisation failed
 * \return 0 on success
 */

int32_t GPSStart(void)
{
    if (gpsEnabled) {
//        if (gpsPort) {
            // Start gps task
            xTaskCreate(gpsTask, "GPS", STACK_SIZE_BYTES / 4, NULL, TASK_PRIORITY, &gpsTaskHandle);
            PIOS_TASK_MONITOR_RegisterTask(TASKINFO_RUNNING_GPS, gpsTaskHandle);
            return 0;
//        }
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
//    gpsPort = PIOS_COM_GPS;

    HwSettingsInitialize();
#ifdef MODULE_GPS_BUILTIN
    gpsEnabled = true;
#else
    HwSettingsOptionalModulesData optionalModules;

    HwSettingsOptionalModulesGet(&optionalModules);

    if (optionalModules.GPS == HWSETTINGS_OPTIONALMODULES_ENABLED) {
        gpsEnabled = true;
    } else {
        gpsEnabled = false;
    }
#endif

#if defined(REVOLUTION)
    // These objects MUST be initialized for Revolution
    // because the rest of the system expects to just
    // attach to their queues
    GPSPositionSensorInitialize();
    GPSVelocitySensorInitialize();
    GPSTimeInitialize();
    GPSSatellitesInitialize();
    HomeLocationInitialize();
#ifdef PIOS_INCLUDE_GPS_UBX_PARSER
    AuxMagSensorInitialize();
    AuxMagSettingsInitialize();
    GPSExtendedStatusInitialize();

    // Initialize mag parameters
    AuxMagSettingsUpdatedCb(NULL);
    AuxMagSettingsConnectCallback(AuxMagSettingsUpdatedCb);
#endif
    GPSSettingsInitialize();
    // updateHwSettings() uses gpsSettings
    GPSSettingsGet(&gpsSettings);
    // must updateHwSettings() before updateGpsSettings() so baud rate is set before GPS serial code starts running
    updateHwSettings(0);
#else /* if defined(REVOLUTION) */
//    if (gpsPort && gpsEnabled) {
    if (gpsEnabled) {
        GPSPositionSensorInitialize();
        GPSVelocitySensorInitialize();
#if !defined(PIOS_GPS_MINIMAL)
        GPSTimeInitialize();
        GPSSatellitesInitialize();
#endif
#if defined(PIOS_GPS_SETS_HOMELOCATION)
        HomeLocationInitialize();
#endif
        GPSSettingsInitialize();
        // updateHwSettings() uses gpsSettings
        GPSSettingsGet(&gpsSettings);
        // must updateHwSettings() before updateGpsSettings() so baud rate is set before GPS serial code starts running
        updateHwSettings(0);
    }
#endif /* if defined(REVOLUTION) */

//    if (gpsPort && gpsEnabled) {
    if (gpsEnabled) {
#if defined(PIOS_INCLUDE_GPS_NMEA_PARSER) && defined(PIOS_INCLUDE_GPS_UBX_PARSER)
        gps_rx_buffer = pios_malloc((sizeof(struct UBXPacket) > NMEA_MAX_PACKET_LENGTH) ? sizeof(struct UBXPacket) : NMEA_MAX_PACKET_LENGTH);
#elif defined(PIOS_INCLUDE_GPS_UBX_PARSER)
        gps_rx_buffer = pios_malloc(sizeof(struct UBXPacket));
#elif defined(PIOS_INCLUDE_GPS_NMEA_PARSER)
        gps_rx_buffer = pios_malloc(NMEA_MAX_PACKET_LENGTH);
#else
        gps_rx_buffer = NULL;
#endif
#if defined(PIOS_INCLUDE_GPS_NMEA_PARSER) || defined(PIOS_INCLUDE_GPS_UBX_PARSER)
        PIOS_Assert(gps_rx_buffer);
#endif
#if defined(PIOS_INCLUDE_GPS_UBX_PARSER) && !defined(PIOS_GPS_MINIMAL)
        HwSettingsConnectCallback(updateHwSettings); // allow changing baud rate even after startup
        GPSSettingsConnectCallback(updateGpsSettings);
#endif
        return 0;
    }

    return -1;
}

MODULE_INITCALL(GPSInitialize, GPSStart);

// ****************
/**
 * Main gps task. It does not return.
 */

static void gpsTask(__attribute__((unused)) void *parameters)
{
    portTickType xDelay = 100 / portTICK_RATE_MS;
    uint32_t timeNowMs  = xTaskGetTickCount() * portTICK_RATE_MS;

#ifdef PIOS_GPS_SETS_HOMELOCATION
    portTickType homelocationSetDelay = 0;
#endif
    GPSPositionSensorData gpspositionsensor;

    timeOfLastUpdateMs  = timeNowMs;
    timeOfLastCommandMs = timeNowMs;

    GPSPositionSensorGet(&gpspositionsensor);
#if defined(PIOS_INCLUDE_GPS_UBX_PARSER) && !defined(PIOS_GPS_MINIMAL)
    // this should be done in the task because it calls out to actually start the GPS serial reads
    updateGpsSettings(0);
#endif

    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    PERF_INIT_COUNTER(counterBytesIn, 0x97510001);
    PERF_INIT_COUNTER(counterRate, 0x97510002);
    PERF_INIT_COUNTER(counterParse, 0x97510003);
//    uint8_t c[GPS_READ_BUFFER];
uint8_t c[GPS_READ_BUFFER+8];
    uint16_t bytes_remaining = 0;

    // Loop forever
    while (1) {
if (gpsPort) {
#if defined(PIOS_INCLUDE_GPS_UBX_PARSER) && !defined(PIOS_GPS_MINIMAL)
        if (gpsSettings.DataProtocol == GPSSETTINGS_DATAPROTOCOL_UBX) {
            char *buffer   = 0;
            uint16_t count = 0;

            gps_ubx_autoconfig_run(&buffer, &count);
            // Something to send?
            if (count) {
                // clear ack/nak
                ubxLastAck.clsID = 0x00;
                ubxLastAck.msgID = 0x00;
                ubxLastNak.clsID = 0x00;
                ubxLastNak.msgID = 0x00;

                PIOS_COM_SendBuffer(gpsPort, (uint8_t *)buffer, count);
            }
        }

        // need to do this whether there is received data or not, or the status (e.g. gcs) is not always correct
        int32_t ac_status = ubx_autoconfig_get_status();
        static uint8_t lastStatus = GPSPOSITIONSENSOR_AUTOCONFIGSTATUS_DISABLED
                                    + GPSPOSITIONSENSOR_AUTOCONFIGSTATUS_RUNNING
                                    + GPSPOSITIONSENSOR_AUTOCONFIGSTATUS_DONE
                                    + GPSPOSITIONSENSOR_AUTOCONFIGSTATUS_ERROR;
        gpspositionsensor.AutoConfigStatus =
            ac_status == UBX_AUTOCONFIG_STATUS_DISABLED ? GPSPOSITIONSENSOR_AUTOCONFIGSTATUS_DISABLED :
            ac_status == UBX_AUTOCONFIG_STATUS_DONE ? GPSPOSITIONSENSOR_AUTOCONFIGSTATUS_DONE :
            ac_status == UBX_AUTOCONFIG_STATUS_ERROR ? GPSPOSITIONSENSOR_AUTOCONFIGSTATUS_ERROR :
            GPSPOSITIONSENSOR_AUTOCONFIGSTATUS_RUNNING;
        if (gpspositionsensor.AutoConfigStatus != lastStatus) {
            GPSPositionSensorAutoConfigStatusSet(&gpspositionsensor.AutoConfigStatus);
            lastStatus = gpspositionsensor.AutoConfigStatus;
        }
#endif /* if defined(PIOS_INCLUDE_GPS_UBX_PARSER) && !defined(PIOS_GPS_MINIMAL) */

        // This blocks the task until there is something on the buffer (or 100ms? passes)
        uint16_t cnt;
        uint16_t bytes_used;
        cnt = PIOS_COM_ReceiveBuffer(gpsPort, &c[bytes_remaining], GPS_READ_BUFFER-bytes_remaining, xDelay);
//        cnt = PIOS_COM_ReceiveBuffer(gpsPort, c, GPS_READ_BUFFER, xDelay);
        if (cnt > 0) {
            PERF_TIMED_SECTION_START(counterParse);
            PERF_TRACK_VALUE(counterBytesIn, cnt);
            PERF_MEASURE_PERIOD(counterRate);
            int res;
            switch (gpsSettings.DataProtocol) {
#if defined(PIOS_INCLUDE_GPS_NMEA_PARSER)
            case GPSSETTINGS_DATAPROTOCOL_NMEA:
                res = parse_nmea_stream(c, cnt, gps_rx_buffer, &gpspositionsensor, &gpsRxStats);
                break;
#endif
#if defined(PIOS_INCLUDE_GPS_UBX_PARSER)
            case GPSSETTINGS_DATAPROTOCOL_UBX:
                cnt += bytes_remaining;
                res = parse_ubx_stream(c, cnt, gps_rx_buffer, &gpspositionsensor, &gpsRxStats, &bytes_used);
//                res = parse_ubx_stream(c, cnt, gps_rx_buffer, &gpspositionsensor, &gpsRxStats, &bytes_used);
#if 1
                bytes_remaining = cnt - bytes_used;
                memmove(c, &c[bytes_used], bytes_remaining);
#endif
                break;
#endif
            default:
                res = NO_PARSER; // this should not happen
                break;
            }

            PERF_TIMED_SECTION_END(counterParse);
            if (res == PARSER_COMPLETE) {
                timeNowMs = xTaskGetTickCount() * portTICK_RATE_MS;
                timeOfLastUpdateMs = timeNowMs;
                timeOfLastCommandMs = timeNowMs;
            }
        }

        // Check for GPS timeout
        timeNowMs = xTaskGetTickCount() * portTICK_RATE_MS;
        if ((timeNowMs - timeOfLastUpdateMs) >= GPS_TIMEOUT_MS ||
            (gpsSettings.DataProtocol == GPSSETTINGS_DATAPROTOCOL_UBX && gpspositionsensor.AutoConfigStatus == GPSPOSITIONSENSOR_AUTOCONFIGSTATUS_ERROR)) {
            // we have not received any valid GPS sentences for a while.
            // either the GPS is not plugged in or a hardware problem or the GPS has locked up.
            GPSPositionSensorStatusOptions status = GPSPOSITIONSENSOR_STATUS_NOGPS;
            GPSPositionSensorStatusSet(&status);
            AlarmsSet(SYSTEMALARMS_ALARM_GPS, SYSTEMALARMS_ALARM_ERROR);
        } else {
            // we appear to be receiving GPS sentences OK, we've had an update
            // criteria for GPS-OK taken from this post...
            // http://forums.openpilot.org/topic/1523-professors-insgps-in-svn/page__view__findpost__p__5220
            if ((gpspositionsensor.PDOP < gpsSettings.MaxPDOP) && (gpspositionsensor.Satellites >= gpsSettings.MinSatellites) &&
                (gpspositionsensor.Status == GPSPOSITIONSENSOR_STATUS_FIX3D) &&
                (gpspositionsensor.Latitude != 0 || gpspositionsensor.Longitude != 0)) {
                AlarmsClear(SYSTEMALARMS_ALARM_GPS);
#ifdef PIOS_GPS_SETS_HOMELOCATION
                HomeLocationData home;
                HomeLocationGet(&home);

                if (home.Set == HOMELOCATION_SET_FALSE) {
                    if (homelocationSetDelay == 0) {
                        homelocationSetDelay = xTaskGetTickCount();
                    }
                    if (xTaskGetTickCount() - homelocationSetDelay > GPS_HOMELOCATION_SET_DELAY) {
                        setHomeLocation(&gpspositionsensor);
                        homelocationSetDelay = 0;
                    }
                } else {
                    homelocationSetDelay = 0;
                }
#endif
            } else if ((gpspositionsensor.Status == GPSPOSITIONSENSOR_STATUS_FIX3D) &&
                       (gpspositionsensor.Latitude != 0 || gpspositionsensor.Longitude != 0)) {
                AlarmsSet(SYSTEMALARMS_ALARM_GPS, SYSTEMALARMS_ALARM_WARNING);
            } else {
                AlarmsSet(SYSTEMALARMS_ALARM_GPS, SYSTEMALARMS_ALARM_CRITICAL);
            }
        }
} // if (gpsPort)
        vTaskDelayUntil(&xLastWakeTime, GPS_LOOP_DELAY_MS / portTICK_RATE_MS);
    } // while (1)
}

#ifdef PIOS_GPS_SETS_HOMELOCATION
/*
 * Estimate the acceleration due to gravity for a particular location in LLA
 */
static float GravityAccel(float latitude, __attribute__((unused)) float longitude, float altitude)
{
    /* WGS84 gravity model.  The effect of gravity over latitude is strong
     * enough to change the estimated accelerometer bias in those apps. */
    float sinsq = sinf(latitude);

    sinsq *= sinsq;
    /* Likewise, over the altitude range of a high-altitude balloon, the effect
     * due to change in altitude can also affect the model. */
    return 9.7803267714f * (1.0f + 0.00193185138639f * sinsq) / sqrtf(1.0f - 0.00669437999013f * sinsq)
           - 3.086e-6f * altitude;
}

// ****************

static void setHomeLocation(GPSPositionSensorData *gpsData)
{
    HomeLocationData home;

    HomeLocationGet(&home);
    GPSTimeData gps;
    GPSTimeGet(&gps);

    if (gps.Year >= 2000) {
        /* Store LLA */
        home.Latitude  = gpsData->Latitude;
        home.Longitude = gpsData->Longitude;
        home.Altitude  = gpsData->Altitude + gpsData->GeoidSeparation;

        /* Compute home ECEF coordinates and the rotation matrix into NED
         * Note that floats are used here - they should give enough precision
         * for this application.*/

        float LLA[3] = { (home.Latitude) / 10e6f, (home.Longitude) / 10e6f, (home.Altitude) };

        /* Compute magnetic flux direction at home location */
        if (WMM_GetMagVector(LLA[0], LLA[1], LLA[2], gps.Month, gps.Day, gps.Year, &home.Be[0]) == 0) {
            /*Compute local acceleration due to gravity.  Vehicles that span a very large
             * range of altitude (say, weather balloons) may need to update this during the
             * flight. */
            home.g_e = GravityAccel(LLA[0], LLA[1], LLA[2]);
            home.Set = HOMELOCATION_SET_TRUE;
            HomeLocationSet(&home);
        }
    }
}
#endif /* ifdef PIOS_GPS_SETS_HOMELOCATION */


uint32_t hwsettings_gpsspeed_enum_to_baud(uint8_t baud)
{
    switch (baud) {
    case HWSETTINGS_GPSSPEED_2400:
        return(2400);
    case HWSETTINGS_GPSSPEED_4800:
        return(4800);
    default:
    case HWSETTINGS_GPSSPEED_9600:
        return(9600);
    case HWSETTINGS_GPSSPEED_19200:
        return(19200);
    case HWSETTINGS_GPSSPEED_38400:
        return(38400);
    case HWSETTINGS_GPSSPEED_57600:
        return(57600);
    case HWSETTINGS_GPSSPEED_115200:
        return(115200);
    case HWSETTINGS_GPSSPEED_230400:
        return(230400);
    }
}


//void debugindex(__attribute__((unused)) int index, __attribute__((unused)) uint8_t c)
void debugindex(int index, uint8_t c)
{
//#if defined(PIOS_INCLUDE_GPS_UBX_PARSER) && !defined(PIOS_GPS_MINIMAL)
    static GPSExtendedStatusData olddata;
    static GPSExtendedStatusData data;

    if (index < 8) {
      data.FirmwareHash[index] = c;
    } else {
      data.BoardType[index-8] = c;
    }

    if (memcmp(&data, &olddata, sizeof(data))) {
        olddata = data;
        static uint8_t mutex; // = 0
        if (__sync_fetch_and_add(&mutex, 1) == 0) {
            GPSExtendedStatusSet(&data);
        }
        --mutex;
    }
//#endif
}


// having already set the GPS's baud rate with a serial command, set the local Revo port baud rate
void gps_set_fc_baud_from_arg(uint8_t baud, __attribute__((unused)) uint8_t addit)
{
    static uint8_t mutex; // = 0

    if (__sync_fetch_and_add(&mutex, 1) == 0) {
        // Set Revo port hwsettings_baud
        PIOS_COM_ChangeBaud(PIOS_COM_GPS, hwsettings_gpsspeed_enum_to_baud(baud));
        GPSPositionSensorCurrentBaudRateSet(&baud);
    } else {
        static uint8_t c;
        debugindex(9, ++c);
        if (c > 254) c=254;
    }
    --mutex;

    debugindex(6, baud);
    {
        static uint8_t c;
        debugindex(7, ++c);
    }
}


#if 0
void gps_set_fc_baud_from_settings()
{
    HwSettingsGPSSpeedOptions speed;

    // Retrieve settings
    HwSettingsGPSSpeedGet(&speed);

    gps_set_fc_baud_from_arg((uint8_t) speed);
}
#endif


/**
 * Update the GPS settings, called on startup.
 * FIXME: This should be in the GPSSettings object. But objects have
 * too much overhead yet. Also the GPS has no any specific settings
 * like protocol, etc. Thus the HwSettings object which contains the
 * GPS port speed is used for now.
 */

// must updateHwSettings() before updateGpsSettings() so baud rate is set before GPS serial code starts running
static void updateHwSettings(UAVObjEvent __attribute__((unused)) *ev)
{
    // with these changes, no one (not even OP board makers) should ever need u-center (the complex UBlox configuration program)
    // no booting of Revo should be required during any setup or testing, just Send the settings you want to play with
    // with autoconfig enabled, just change the baud rate in HwSettings and it will change the GPS internal baud and then the Revo port
    // with autoconfig disabled, it will only change the Revo port, so you can try to find the current GPS baud rate if you don't know it
    // GPSPositionSensor.SensorType and .UbxAutoConfigStatus now give correct values at all times, so use .SensorType to prove it is
    // connected, even to an incorrectly configured (e.g. factory default) GPS

    // Use cases:
    // - the user can plug in a default GPS, use autoconfig-store once and then always use autoconfig-disabled
    // - the user can plug in a default GPS that can't store settings (defaults to 9600 baud) and always use autoconfig-nostore
    // - - this is one reason for coding this: cheap eBay GPS's that loose their settings sometimes
    // - the user can plug in a default GPS that _can_ store settings and always use autoconfig-nostore with that too
    // - - if settings change in newer releases of OP, this will make sure to use the correct settings with whatever code you are running
    // - the user can plug in a correctly configured GPS and use autoconfig-disabled
    // - the user can recover an old or incorrectly configured GPS (internal GPS baud rate or other GPS settings wrong)
    // - - disable UBX GPS autoconfig
    // - - set HwSettings GPS baud rate to the current GPS internal baud rate to get it to connect at current (especially non-9600) baud rate
    // - - - you can tell the baud rate is correct when GPSPositionSensor.SensorType is known (not "Unknown") (e.g. UBX6)
    // - - - the SensorType and AutoConfigStatus may fail at 9600 and will fail at 4800 or lower if the GPS is configured to send OP data
    // - - - (way more than default) at 4800 baud or slower
    // - - enable autoconfig-nostore GPSSettings to set correct messages and enable further magic baud rate settings
    // - - change the baud rate to what you want in HwSettings (it will change the baud rate in the GPS and then in the Revo port)
    // - - wait for the baud rate change to complete, GPSPositionSensor.AutoConfigStatus will say "DONE"
    // - - enable autoconfig-store and it will save the new baud rate and the correct message configuration
    // - - wait for the store to complete, GPSPositionSensor.AutoConfigStatus will say "DONE"
    // - - for the new baud rate, the user should either choose 9600 for use with autoconfig-nostore or can choose 57600 for use with autoconfig-disabled
    // - the user (Dave :)) can configure a whole bunch of default GPS's with no intervention by using autoconfig-store as a saved Revo setting
    // - - just plug the default (9600 baud) GPS in to an unpowered Revo, power the Revo/GPS through the servo connector, wait some time, unplug
    // - - or with this code, OP could and even should just ship GPS's with default settings

    // if we add an "initial baud rate" instead of assuming 9600 at boot up for autoconfig-nostore/store
    // - the user can use the same GPS with both an older release of OP (e.g. GPS settings at 38400) and the current release (autoconfig-nostore)
    // - the 57600 could be fixed instead of the 9600 as part of autoconfig-store/nostore and the HwSettings.GPSSpeed be the initial baud rate?

    // About using UBlox GPS's with default settings (9600 baud and NEMA data):
    // - the default uBlox settings (different than OP settings) are NEMA and 9600 baud
    // - - and that is OK and you use autoconfig-nostore
    // - - I personally feel that this is the best way to set it up because if OP dev changes GPS settings,
    // - - you get them automatically changed to match whatever version of OP code you are running
    // - - but 9600 is only OK for this autoconfig startup
    // - by my testing, the 9600 initial to 57600 final baud startup takes:
    // - - 0:10 if the GPS has been configured to send OP data at 9600
    // - - 0:06 if the GPS has default data settings (NEMA) at 9600
    // - - reminder that even 0:10 isn't that bad.  You need to wait for the GPS to acquire satellites,

    // Some things you want to think about if you want to play around with this:
    // - don't play with low baud rates, with OP data settings (lots of data) it can take a long time to auto-configure
    // - - at 2400 it could take a long time to autoconfig or error out
    // - - at 9600 or lower the autoconfig is skipped and only the baud rate is changed
    // - if you autoconfigure an OP configuration at 9600 baud or lower
    // - - it will actually make a factory default configuration (not an OP configuration) at that baud rate
    // - remember that there are two baud rates (inside the GPS and the Revo port) and you can get them out of sync
    // - - rebooting the Revo from the Ground Control Station (without powering the GPS down too)
    // - - can leave the baud rates out of sync if you are using autoconfig
    // - - just power off and on both the Revo and the GPS
    // - my OP GPS #2 will NOT save 115200 baud or higher, but it will use all bauds, even 230400
    // - - so you could use autoconfig.nostore with this high baud rate, but not autoconfig.store (once) followed by autoconfig.disabled
    // - - I haven't tested other GPS's in regard to this

    // since 9600 baud and lower are not usable, and are best left at NEMA, I could have coded it to do a factory reset
    // - if set to 9600 baud (or lower)

    // if GPS (UBX or NEMA) is enabled at all
    static uint32_t previousGpsPort=0xf0f0f0f0; // = 0 means deconfigured and at power up it is deconfigured
    if (gpsPort && gpsEnabled) {
//we didn't have a gpsPort when we booted, and disabled did not find GPS's
//but booting with gpsPort valid immediately worked
//oh, PIOS_COM_GPS is certainly set during init and not elsewhere
//and that means changing the GPS port will not be recognized till reboot
/*
it autobauds even if autoconfig is disabled
  initial thought is disable that, but is it a good thing to leave in?
  we could have several levels, autobaud, autoconfig, save
  on one hand, always autobaud is good for user
  on other hand, always autobaud is somewhat disruptive to experimentation?  I can't think why.  To test simulated GPS failure without rewiring?

gps at 2400 baud with autoconfig disabled appears to not have autobauded correctly
  and autoconfig runs forever if it gets enabled
  yes it did work, but why is SensorType Unknown?
    is it running the first step over and over which resets sensor type?
    no date so no updates means alarmset()
    and all it takes is a MON_VER to make it OK
    but why did it work at 57600 and 38400 autobauds?
      makes me wonder if 2400 is so slow that it never completes before timeout
      so try 9600 etc.
    gpstime is working, so baud rate is correct at least part of the time
      is it possible that once it gets 2400, it can't get out?
  GPSPositionSensor.Status jumps around from NoGPS to NoFix to Fix3D
  all else fails try putting gpsPort back the way it was for a test
    could be that alarm is disabling gpsPort?  that would be stupid

Oh!!!!
  It got stuck because I configured it to 57600 and then let it autobaud to 2400 so it has all messages at 2400
    so autobaud is not good without deconfiguring for low baud rates
  the previous procedure was, first change to a new baud rate (assuming it was a higher rate)
    then ALWAYS configure (including removing messages for 9600 and lower)
    so the code would never let it get it in a bad way (2400 with OP messages)
      how about starting at 57600 and going to 2400?
      doesn't seem like that should work in the old code

So do we need to disable messages somehow at start of config?
  maybe disable ubx in port
  maybe disable known messages
    could use a flag with same tables and don't wait for ack???

will MON_VER work if outProtoMask = 0?
  if so, then maybe all rates <= 9600 (19200?) should set outProtoMask = 0
  remember that autoconfig=disabled and u-center is an option for dev experimentation
  I really want to let them try 19200 with sentences

do we want baud rate to autobaud always?
  is then there any reason for manual setting of baud?

well these fixes maybe make it so it won't get into trouble
  but will it get someone out of their own trouble?

first version of new code
  4800 works but 2400 does not
  2400 works if no data

if increasing timeouts fixes it
  maybe we can make timeouts proportional to inverse baud rate
  doubling timeouts did not fix it

IMPORTANT
with autoconfig disabled, when you change the baud rate
  you just get autobauded back to the original baud rate, but don't know it
  that is confusing
  it needs one extra step to set desired baud rate after autobauding?
  changing baud without configuring at all is dangerous
    changing from 57600 to 2400 and leaving messages enabled
    if we disable messages, then it is not baud only and it is not baud + configure
      and that is confusing

it appears that outprotocol=0 means the MON_VER request doesn't reply
  after setting baud to 2400??? with 0 and flashing, it did not get MON_VER

it will go into 2400 but won't come out

it still might be possible to use outprotocol=0 if we set it back to 1
  but that is no good, MON_VER will still be bad then
  well it won't come out of 2400 with messages, even with outprotocol=0
  it truely never gets started on the FSM because MON_VER is blocked
  well if MON_VER responds, we could set outprotocol=0 just before MON_VER???

so the big problem is MOV_VER
  but there are some messages coming back and it must be MON_VER?
  could try commenting out MON_VER send and see if messages go away

it still bounces NoGPS to NoFix to Fix3D even with MON_VER turned off
  well this is 2400 with messages so that makes sense

How to get it out of 2400 with messages
  maybe the problem is that it took a bunch of retries before
  and now it takes 10 times as many because it has to try all baud rates
  maybe we can change the cycle factor and make all messages come every 99 cycles at 1000ms per cycle
    as part of the MON_VER packet
  maybe only allow autobaud if autoconfig is enabled

test old version

test version where it only uses hwsettings_baud (and thus autobaud is disabled)
  maybe it is a loop to set gpssettings.ubxautoconfig from a task awakened by gpssettings callback
where did 38400 come from
  38400 is boot up HwSettings.GPSSpeed
  but I had set HwSettings.GPSSpeed to 2400
  it toggled back and forth between 2400 and 38400 and then locked on 38400
    it locked at the time the SensorType went to UBX7

There are no GPSSatellites messages being received for UBX7

Is there a reentrancy?
  maybe we need to just set flags in one thread and do it all from the other thread

maybe from ubx_autoconfig.c I should just set hwsettings and let GPS.c do the real baud rate set.
  and for gps_ubx_reset_sensor_type() I can just skip it (just GPSPositionSensorSensorTypeSet()) if already being called

nobody set the baud, but it quit working when I changed baud in GCS
  that might imply that calling HwSettingsGPSSpeedSet() actually change the baud rate.
  if that is true, (and it probably is) then it is not possible to use the gcs HwSettings.GPSSpeed to change first the GPS and then the port.
  we need a NewGpsSpeed in GpsSettings.

if we already have a baud rate and configure is enabled,
  we don't want autobaud running to mess up the acquired baud rate
  OTOH we need changing the baud to reaquire in case they changed GPS's?
    we could say that you must reboot after changing a GPS or unplugging/plugging a cable

should autobaud connect also set the GPS baud to HwSettings?
  autobaud connect (set Revo to GPS baud rate)
  autobaud change  (set GPS to HwSettings.GPSSpeed)
  configure
  save
  disable

should disable still do autobaud connect on startup and changes?

get GPS at 2400 with sentences
  see what it does now
  after baud is correct and we set GPSSpeed, it resets sensortype
    which causes it to set baud to set baud to GPSSpeed before FSM ran to set GPS baud

design the flow, disabled does nothing
  if disabled
    callback sets baud
    callback resets sensortype
    ac sees invalid sensor type and ...

how to handle "connection" i.e. when baud is right or when it is wrong
  want to skip some things in some cases when connection is already correct
    that will make baud changes quick
  want to do   some things in some cases when connection is already correct
    that will allow them to know real status
  at startup, don't assume connection is valid, test it
    autobaud does bauds in probability order
    starting with hwsettings.gpsspeed so it almost isn't even an autobaud
  once connection is made, we need to avoid resetting sensortype
    unless what ... 'what' is
      maybe monitor MON_VER somehow to see if cable gets unplugged
      maybe don't bother and tell them not to disconnect cables
      and when something new goes bad, start over with autobaud
  should we monitor MON_VER to know when connection goes down?
    if so, we need a better way than resetting sensor type
    a simple ack would be good.

reset sensor type
  reduce to just startup and error
  no, it isn't a problem, it is the baud rate setting that is now associated with it
  yes, reduce it, it should not be popping back and forth between good and bad just to poll it
  baud_to_try_index is reset there, where should it be reset?

at least have a procedure for starting over without reboot
  like when plugging in a different GPS
  currently the sensortype global is what drives the autoconfig process, so it must be validated
  as part of the FSM
  so it is initially detected outside FSM, and then for each configure, it is detected again, but without resetting it
  that can be done with an FSM wait after sending MON_VER again.
  error will reset sensor type

how to set baud rate in callback without causing unsyncing
  for synced baud rate and when allowing autobaud in ...  huh?

IMPORTANT
the ubx parser doesn't handle partially received buffers

*/
//new code: at startup does it always need to be HwSettings.GPSSpeed? yes

// must always set the baud here, even if it looks like it is the same
// e.g. startup sets 9600 here, ubx_autoconfig sets 57600 there, then the user selects a change back to 9600 here

        // on first use of this port (previousGpsPort != gpsPort) we must set the Revo port baud rate
        // after startup (previousGpsPort == gpsPort) we must set the Revo port baud rate only if autoconfig is disabled
        // always we must set the Revo port baud rate if not using UBX
#if defined(PIOS_INCLUDE_GPS_UBX_PARSER) && !defined(PIOS_GPS_MINIMAL)
        if (ev == NULL
// test prevport
|| previousGpsPort != gpsPort
            || gpsSettings.UbxAutoConfig == GPSSETTINGS_UBXAUTOCONFIG_DISABLED
// this needs to be disabled if we allow it to autobaud when autoconfig is disabled
            || gpsSettings.DataProtocol  != GPSSETTINGS_DATAPROTOCOL_UBX)
#endif
        {
#if 1
            uint8_t speed;
            // Retrieve settings
            HwSettingsGPSSpeedGet(&speed);
            // set fc baud
            gps_set_fc_baud_from_arg(speed, 1);
debugindex(2, speed);
{
static uint8_t c;
debugindex(3, ++c);
}
#endif
#if defined(PIOS_INCLUDE_GPS_UBX_PARSER) && !defined(PIOS_GPS_MINIMAL)
            // even changing the baud rate will make it re-verify the sensor type
            // that way the user can just try some baud rates and when the sensor type is valid, the baud rate is correct
            gps_ubx_reset_sensor_type();
//careful here if the sensor type is connected to autobaud

// make sure that normal autoconfig still works
// that you can set just Revo baud manually and both automatically
#endif
        }
#if defined(PIOS_INCLUDE_GPS_UBX_PARSER) && !defined(PIOS_GPS_MINIMAL)
        else {
            // it will never do this during startup because ev == NULL
            gps_ubx_autoconfig_set(NULL);
        }
#endif
    }
    previousGpsPort = gpsPort;
}

#if defined(PIOS_INCLUDE_GPS_UBX_PARSER) && !defined(PIOS_GPS_MINIMAL)
void AuxMagSettingsUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    load_mag_settings();
}

void updateGpsSettings(__attribute__((unused)) UAVObjEvent *ev)
{
    ubx_autoconfig_settings_t newconfig;

    GPSSettingsGet(&gpsSettings);

#if 0
    // if GPS is enabled and UBX GPS is enabled
    if (gpsPort && gpsEnabled && gpsSettings.UbxAutoConfig != GPSSETTINGS_UBXAUTOCONFIG_DISABLED) {
        newconfig.UbxAutoConfig = gpsSettings.UbxAutoConfig;
    } else {
        newconfig.UbxAutoConfig = GPSSETTINGS_UBXAUTOCONFIG_DISABLED;
    }
#else
    // it's OK that ubx auto config is inited and ready to go, when GPS is disabled or running NEMA
    // because ubx auto config never gets called
    // setting it up completely means that if we switch from initial NEMA to UBX or disabled to enabled, that it will start normally
    newconfig.UbxAutoConfig = gpsSettings.UbxAutoConfig;
#endif

    newconfig.navRate = gpsSettings.UbxRate;

//    newconfig.autoconfigEnabled     = gpsSettings.UbxAutoConfig != GPSSETTINGS_UBXAUTOCONFIG_DISABLED;
//    newconfig.storeSettings         = gpsSettings.UbxAutoConfig == GPSSETTINGS_UBXAUTOCONFIG_CONFIGUREANDSTORE;
//    newconfig.configStoreAndDisable = gpsSettings.UbxAutoConfig == GPSSETTINGS_UBXAUTOCONFIG_CONFIGSTOREANDDISABLE;

    newconfig.dynamicModel = gpsSettings.UbxDynamicModel == GPSSETTINGS_UBXDYNAMICMODEL_PORTABLE   ? UBX_DYNMODEL_PORTABLE   :
                             gpsSettings.UbxDynamicModel == GPSSETTINGS_UBXDYNAMICMODEL_STATIONARY ? UBX_DYNMODEL_STATIONARY :
                             gpsSettings.UbxDynamicModel == GPSSETTINGS_UBXDYNAMICMODEL_PEDESTRIAN ? UBX_DYNMODEL_PEDESTRIAN :
                             gpsSettings.UbxDynamicModel == GPSSETTINGS_UBXDYNAMICMODEL_AUTOMOTIVE ? UBX_DYNMODEL_AUTOMOTIVE :
                             gpsSettings.UbxDynamicModel == GPSSETTINGS_UBXDYNAMICMODEL_SEA        ? UBX_DYNMODEL_SEA        :
                             gpsSettings.UbxDynamicModel == GPSSETTINGS_UBXDYNAMICMODEL_AIRBORNE1G ? UBX_DYNMODEL_AIRBORNE1G :
                             gpsSettings.UbxDynamicModel == GPSSETTINGS_UBXDYNAMICMODEL_AIRBORNE2G ? UBX_DYNMODEL_AIRBORNE2G :
                             gpsSettings.UbxDynamicModel == GPSSETTINGS_UBXDYNAMICMODEL_AIRBORNE4G ? UBX_DYNMODEL_AIRBORNE4G :
                             UBX_DYNMODEL_AIRBORNE1G;

    switch ((GPSSettingsUbxSBASModeOptions)gpsSettings.UbxSBASMode) {
    case GPSSETTINGS_UBXSBASMODE_RANGINGCORRECTION:
    case GPSSETTINGS_UBXSBASMODE_CORRECTION:
    case GPSSETTINGS_UBXSBASMODE_RANGINGCORRECTIONINTEGRITY:
    case GPSSETTINGS_UBXSBASMODE_CORRECTIONINTEGRITY:
        newconfig.SBASCorrection = true;
        break;
    default:
        newconfig.SBASCorrection = false;
    }

    switch ((GPSSettingsUbxSBASModeOptions)gpsSettings.UbxSBASMode) {
    case GPSSETTINGS_UBXSBASMODE_RANGING:
    case GPSSETTINGS_UBXSBASMODE_RANGINGCORRECTION:
    case GPSSETTINGS_UBXSBASMODE_RANGINGINTEGRITY:
    case GPSSETTINGS_UBXSBASMODE_RANGINGCORRECTIONINTEGRITY:
        newconfig.SBASRanging = true;
        break;
    default:
        newconfig.SBASRanging = false;
    }

    switch ((GPSSettingsUbxSBASModeOptions)gpsSettings.UbxSBASMode) {
    case GPSSETTINGS_UBXSBASMODE_INTEGRITY:
    case GPSSETTINGS_UBXSBASMODE_RANGINGINTEGRITY:
    case GPSSETTINGS_UBXSBASMODE_RANGINGCORRECTIONINTEGRITY:
    case GPSSETTINGS_UBXSBASMODE_CORRECTIONINTEGRITY:
        newconfig.SBASIntegrity = true;
        break;
    default:
        newconfig.SBASIntegrity = false;
    }

    newconfig.SBASChannelsUsed = gpsSettings.UbxSBASChannelsUsed;

    newconfig.SBASSats = gpsSettings.UbxSBASSats == GPSSETTINGS_UBXSBASSATS_WAAS  ? UBX_SBAS_SATS_WAAS  :
                         gpsSettings.UbxSBASSats == GPSSETTINGS_UBXSBASSATS_EGNOS ? UBX_SBAS_SATS_EGNOS :
                         gpsSettings.UbxSBASSats == GPSSETTINGS_UBXSBASSATS_MSAS  ? UBX_SBAS_SATS_MSAS  :
                         gpsSettings.UbxSBASSats == GPSSETTINGS_UBXSBASSATS_GAGAN ? UBX_SBAS_SATS_GAGAN :
                         gpsSettings.UbxSBASSats == GPSSETTINGS_UBXSBASSATS_SDCM  ? UBX_SBAS_SATS_SDCM  : UBX_SBAS_SATS_AUTOSCAN;

    switch (gpsSettings.UbxGNSSMode) {
    case GPSSETTINGS_UBXGNSSMODE_GPSGLONASS:
        newconfig.enableGPS     = true;
        newconfig.enableGLONASS = true;
        newconfig.enableBeiDou  = false;
        break;
    case GPSSETTINGS_UBXGNSSMODE_GLONASS:
        newconfig.enableGPS     = false;
        newconfig.enableGLONASS = true;
        newconfig.enableBeiDou  = false;
        break;
    case GPSSETTINGS_UBXGNSSMODE_GPS:
        newconfig.enableGPS     = true;
        newconfig.enableGLONASS = false;
        newconfig.enableBeiDou  = false;
        break;
    case GPSSETTINGS_UBXGNSSMODE_GPSBEIDOU:
        newconfig.enableGPS     = true;
        newconfig.enableGLONASS = false;
        newconfig.enableBeiDou  = true;
        break;
    case GPSSETTINGS_UBXGNSSMODE_GLONASSBEIDOU:
        newconfig.enableGPS     = false;
        newconfig.enableGLONASS = true;
        newconfig.enableBeiDou  = true;
        break;
    default:
        newconfig.enableGPS     = false;
        newconfig.enableGLONASS = false;
        newconfig.enableBeiDou  = false;
        break;
    }

    gps_ubx_autoconfig_set(&newconfig);
}
#endif /* if defined(PIOS_INCLUDE_GPS_UBX_PARSER) && !defined(PIOS_GPS_MINIMAL) */
/**
 * @}
 * @}
 */
