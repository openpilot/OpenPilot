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
//#include "../../libraries/inc/fifo_buffer.h"
#endif

#include <pios_instrumentation_helper.h>
PERF_DEFINE_COUNTER(counterBytesIn);
PERF_DEFINE_COUNTER(counterRate);
PERF_DEFINE_COUNTER(counterParse);
// ****************
// Private functions

static void gpsTask(void *parameters);
static void updateHwSettings(UAVObjEvent *ev);

#ifdef PIOS_GPS_SETS_HOMELOCATION
static void setHomeLocation(GPSPositionSensorData *gpsData);
static float GravityAccel(float latitude, float longitude, float altitude);
#endif

#ifdef PIOS_INCLUDE_GPS_UBX_PARSER
void AuxMagSettingsUpdatedCb(UAVObjEvent *ev);
#ifndef PIOS_GPS_MINIMAL
void updateGpsSettings(UAVObjEvent *ev);
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
        #define STACK_SIZE_BYTES   1024
#else
#if defined(PIOS_GPS_MINIMAL)
        #define GPS_READ_BUFFER    32

#ifdef PIOS_INCLUDE_GPS_NMEA_PARSER
        #define STACK_SIZE_BYTES   580 // NMEA
#else
        #define STACK_SIZE_BYTES   440 // UBX
#endif // PIOS_INCLUDE_GPS_NMEA_PARSER
#else
        #define STACK_SIZE_BYTES   650
#endif // PIOS_GPS_MINIMAL
#endif // PIOS_GPS_SETS_HOMELOCATION

#ifndef GPS_READ_BUFFER
#define GPS_READ_BUFFER            128
#endif

#define TASK_PRIORITY              (tskIDLE_PRIORITY + 1)

// ****************
// Private variables

static GPSSettingsData gpsSettings;

static uint32_t gpsPort;
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
        if (gpsPort) {
            // Start gps task
            xTaskCreate(gpsTask, "GPS", STACK_SIZE_BYTES / 4, NULL, TASK_PRIORITY, &gpsTaskHandle);
            PIOS_TASK_MONITOR_RegisterTask(TASKINFO_RUNNING_GPS, gpsTaskHandle);
            return 0;
        }
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
<<<<<<< HEAD
=======
    GPSSettingsDataProtocolOptions gpsProtocol;
>>>>>>> next

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
#else
    if (gpsPort && gpsEnabled) {
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

    if (gpsPort && gpsEnabled) {
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
    uint8_t c[GPS_READ_BUFFER];

    // Loop forever
    while (1) {
#if defined(PIOS_INCLUDE_GPS_UBX_PARSER) && !defined(PIOS_GPS_MINIMAL)
        if (gpsSettings.DataProtocol == GPSSETTINGS_DATAPROTOCOL_UBX) {
            char *buffer   = 0;
            uint16_t count = 0;

            ubx_autoconfig_run(&buffer, &count);
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
#endif

        // This blocks the task until there is something on the buffer (or 100ms? passes)
        uint16_t cnt;
        cnt = PIOS_COM_ReceiveBuffer(gpsPort, c, GPS_READ_BUFFER, xDelay);
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
                res = parse_ubx_stream(c, cnt, gps_rx_buffer, &gpspositionsensor, &gpsRxStats);
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
        vTaskDelayUntil(&xLastWakeTime, GPS_LOOP_DELAY_MS / portTICK_RATE_MS);
    }
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

   if (gpsPort) {
        HwSettingsGPSSpeedOptions speed;
 
#if defined(PIOS_INCLUDE_GPS_UBX_PARSER) && !defined(PIOS_GPS_MINIMAL)
       // use 9600 baud during startup if autoconfig is enabled
       // that is the flag that the code should assumes a factory default baud rate
       if (ev == NULL && gpsSettings.UbxAutoConfig != GPSSETTINGS_UBXAUTOCONFIG_DISABLED) {
           speed = HWSETTINGS_GPSSPEED_9600;
       }
       else
#endif
       {
           // Retrieve settings
           HwSettingsGPSSpeedGet(&speed);
       }
 
       // must always set the baud here, even if it looks like it is the same
       // e.g. startup sets 9600 here, ubx_autoconfig sets 57600 there, then the user selects a change back to 9600 here
 
       // on    startup (ev == NULL) we must set the Revo port baud rate
       // after startup (ev != NULL) we must set the Revo port baud rate only if autoconfig is disabled
       // always we must set the Revo port baud rate if not using UBX
#if defined(PIOS_INCLUDE_GPS_UBX_PARSER) && !defined(PIOS_GPS_MINIMAL)
       if (ev == NULL || gpsSettings.UbxAutoConfig == GPSSETTINGS_UBXAUTOCONFIG_DISABLED || gpsSettings.DataProtocol != GPSSETTINGS_DATAPROTOCOL_UBX)
#endif
       {
           // Set Revo port speed
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
           case HWSETTINGS_GPSSPEED_230400:
               PIOS_COM_ChangeBaud(gpsPort, 230400);
               break;
           }
#if defined(PIOS_INCLUDE_GPS_UBX_PARSER) && !defined(PIOS_GPS_MINIMAL)
           // even changing the baud rate will make it re-verify the sensor type
           // that way the user can just try some baud rates and it when the sensor type is valid, the baud rate is correct
           ubx_reset_sensor_type();
#endif
       }
#if defined(PIOS_INCLUDE_GPS_UBX_PARSER) && !defined(PIOS_GPS_MINIMAL)
       else {
           // it will never do this during startup because ev == NULL
           ubx_autoconfig_set(NULL);
       }
#endif
   }
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

    newconfig.navRate = gpsSettings.UbxRate;

    newconfig.autoconfigEnabled = gpsSettings.UbxAutoConfig == GPSSETTINGS_UBXAUTOCONFIG_DISABLED ? false : true;
    newconfig.storeSettings     = gpsSettings.UbxAutoConfig == GPSSETTINGS_UBXAUTOCONFIG_CONFIGUREANDSTORE;

    newconfig.dynamicModel = gpsSettings.UbxDynamicModel == GPSSETTINGS_UBXDYNAMICMODEL_PORTABLE ? UBX_DYNMODEL_PORTABLE :
                             gpsSettings.UbxDynamicModel == GPSSETTINGS_UBXDYNAMICMODEL_STATIONARY ? UBX_DYNMODEL_STATIONARY :
                             gpsSettings.UbxDynamicModel == GPSSETTINGS_UBXDYNAMICMODEL_PEDESTRIAN ? UBX_DYNMODEL_PEDESTRIAN :
                             gpsSettings.UbxDynamicModel == GPSSETTINGS_UBXDYNAMICMODEL_AUTOMOTIVE ? UBX_DYNMODEL_AUTOMOTIVE :
                             gpsSettings.UbxDynamicModel == GPSSETTINGS_UBXDYNAMICMODEL_SEA ? UBX_DYNMODEL_SEA :
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

    newconfig.SBASSats = gpsSettings.UbxSBASSats == GPSSETTINGS_UBXSBASSATS_WAAS ? UBX_SBAS_SATS_WAAS :
                         gpsSettings.UbxSBASSats == GPSSETTINGS_UBXSBASSATS_EGNOS ? UBX_SBAS_SATS_EGNOS :
                         gpsSettings.UbxSBASSats == GPSSETTINGS_UBXSBASSATS_MSAS ? UBX_SBAS_SATS_MSAS :
                         gpsSettings.UbxSBASSats == GPSSETTINGS_UBXSBASSATS_GAGAN ? UBX_SBAS_SATS_GAGAN :
                         gpsSettings.UbxSBASSats == GPSSETTINGS_UBXSBASSATS_SDCM ? UBX_SBAS_SATS_SDCM : UBX_SBAS_SATS_AUTOSCAN;

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

    ubx_autoconfig_set(&newconfig);
}
#endif /* if defined(PIOS_INCLUDE_GPS_UBX_PARSER) && !defined(PIOS_GPS_MINIMAL) */
/**
 * @}
 * @}
 */
