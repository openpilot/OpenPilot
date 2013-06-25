/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup OSDOUTPUTModule OSDOutput Module
 * @brief On screen display support
 * @{
 *
 * @file       osdoutput.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Interfacing with OSD module
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

#if FLIGHTBATTERYSTATE_SUPPORTED
#include "flightbatterystate.h"
#endif

#if POSITIONACTUAL_SUPPORTED
#include "positionstate.h"
#endif

#include "systemalarms.h"
#include "attitudestate.h"
#include "hwsettings.h"
#include "flightstatus.h"

static bool osdoutputEnabled;

enum osd_hk_sync {
    OSD_HK_SYNC_A = 0xCB, OSD_HK_SYNC_B = 0x34,
};

enum osd_hk_pkt_type {
    OSD_HK_PKT_TYPE_MISC = 0, OSD_HK_PKT_TYPE_NAV = 1, OSD_HK_PKT_TYPE_MAINT = 2, OSD_HK_PKT_TYPE_ATT = 3, OSD_HK_PKT_TYPE_MODE = 4,
};

enum osd_hk_control_mode {
    OSD_HK_CONTROL_MODE_MANUAL = 0, OSD_HK_CONTROL_MODE_STABILIZED = 1, OSD_HK_CONTROL_MODE_AUTO = 2,
};

struct osd_hk_blob_misc {
    uint8_t  type; /* Always OSD_HK_PKT_TYPE_MISC */
    int16_t  roll;
    int16_t  pitch;
    // uint16_t home;		/* Big Endian */
    enum osd_hk_control_mode control_mode;
    uint8_t  low_battery;
    uint16_t current; /* Big Endian */
} __attribute__((packed));

struct osd_hk_blob_att {
    uint8_t type; /* Always OSD_HK_PKT_TYPE_ATT */
    int16_t roll;
    int16_t pitch;
    int16_t yaw;
    int16_t speed; /* Big Endian */
} __attribute__((packed));

struct osd_hk_blob_nav {
    uint8_t  type; /* Always OSD_HK_PKT_TYPE_NAV */
    uint32_t gps_lat; /* Big Endian */
    uint32_t gps_lon; /* Big Endian */
} __attribute__((packed));

struct osd_hk_blob_maint {
    uint8_t  type; /* Always OSD_HK_PKT_TYPE_MAINT */
    uint8_t  gps_speed;
    uint16_t gps_alt; /* Big Endian */
    uint16_t gps_dis; /* Big Endian */
    uint8_t  status;
    uint8_t  config;
    uint8_t  emerg;
} __attribute__((packed));

struct osd_hk_blob_mode {
    uint8_t  type; /* Always OSD_HK_PKT_TYPE_MODE */
    uint8_t  fltmode;
    uint16_t gps_alt; /* Big Endian */
    uint16_t gps_dis; /* Big Endian */
    uint8_t  armed;
    uint8_t  config;
    uint8_t  emerg;
} __attribute__((packed));

union osd_hk_pkt_blobs {
    struct osd_hk_blob_misc  misc;
    struct osd_hk_blob_nav   nav;
    struct osd_hk_blob_maint maint;
    struct osd_hk_blob_att   att;
    struct osd_hk_blob_mode  mode;
} __attribute__((packed));

struct osd_hk_msg {
    enum osd_hk_sync       sync;
    enum osd_hk_pkt_type   t;
    union osd_hk_pkt_blobs v;
} __attribute__((packed));

static struct osd_hk_msg osd_hk_msg_buf;

static volatile bool newPositionStateData = false;
static volatile bool newBattData     = false;
static volatile bool newAttitudeData = false;
static volatile bool newAlarmData    = false;

static uint32_t osd_hk_com_id;
static uint8_t osd_hk_msg_dropped;
static uint8_t osd_packet;

static void send_update(__attribute__((unused)) UAVObjEvent *ev)
{
    static enum osd_hk_sync sync = OSD_HK_SYNC_A;

    struct osd_hk_msg *msg = &osd_hk_msg_buf;
    union osd_hk_pkt_blobs *blob = &(osd_hk_msg_buf.v);

    /* Make sure we have a COM port bound */
    if (!osd_hk_com_id) {
        return;
    }

    FlightStatusData flightStatus;

    /*
     * Set up the message
     */
    msg->sync = sync;

    switch (osd_packet) {
    case OSD_HK_PKT_TYPE_MISC:
        break;
    case OSD_HK_PKT_TYPE_NAV:
        break;
    case OSD_HK_PKT_TYPE_MAINT:
        break;
    case OSD_HK_PKT_TYPE_ATT:
        msg->t = OSD_HK_PKT_TYPE_ATT;
        float roll;
        AttitudeStateRollGet(&roll);
        blob->att.roll = (int16_t)(roll * 10);

        float pitch;
        AttitudeStatePitchGet(&pitch);
        blob->att.pitch = (int16_t)(pitch * 10);

        float yaw;
        AttitudeStateYawGet(&yaw);
        blob->att.yaw = (int16_t)(yaw * 10);
        break;
    case OSD_HK_PKT_TYPE_MODE:
        msg->t = OSD_HK_PKT_TYPE_MODE;
        FlightStatusGet(&flightStatus);
        blob->mode.fltmode = flightStatus.FlightMode;
        blob->mode.armed   = flightStatus.Armed;
        break;
    default:
        break;
    }

    /* Field not supported yet */
// blob->misc.control_mode = 0;
    /*if (newAlarmData) {
       SystemAlarmsData alarms;
       SystemAlarmsGet(&alarms);

       switch (alarms.Alarm[SYSTEMALARMS_ALARM_BATTERY]) {
       case SYSTEMALARMS_ALARM_UNINITIALISED:
       case SYSTEMALARMS_ALARM_OK:
       blob->misc.low_battery = 0;
       break;
       case SYSTEMALARMS_ALARM_WARNING:
       case SYSTEMALARMS_ALARM_ERROR:
       case SYSTEMALARMS_ALARM_CRITICAL:
       default:
       blob->misc.low_battery = 1;
       break;
       }

       newAlarmData = false;
       }*/

#if FLIGHTBATTERYSUPPORTED
    if (newBattData) {
        float consumed_energy;
        FlightBatteryStateConsumedEnergyGet(&consumed_energy);

        uint16_t current = (uint16_t)(consumed_energy * 10);

        /* convert to big endian */
        blob->misc.current = (
            (current & 0xFF00 >> 8) |
            (current & 0x00FF << 8));

        newBattData = false;
    }
#else
// blob->misc.current = 0;
#endif

#if POSITIONACTUAL_SUPPORTED
    if (newPositionStateData) {
        PositionStateData position;
        PositionStateGet(&position);

        /* compute 3D distance */
        float d = sqrt(
            pow(position.North, 2) +
            pow(position.East, 2) +
            pow(position.Down, 2));
        /* convert from cm to dm (10ths of m) */
        uint16_t home = (uint16_t)(d / 10);

        /* convert to big endian */
        blob->misc.home = (
            (home & 0xFF00 >> 8) |
            (home & 0x00FF << 8));

        newPositionStateData = false;
    }
#else
// blob->misc.home = 0;
#endif /* if POSITIONACTUAL_SUPPORTED */

    if (!PIOS_COM_SendBufferNonBlocking(osd_hk_com_id, (uint8_t *)&osd_hk_msg_buf, sizeof(osd_hk_msg_buf))) {
        /* Sent a packet, flip to the opposite sync */
        if (sync == OSD_HK_SYNC_A) {
            sync = OSD_HK_SYNC_B;
        } else {
            sync = OSD_HK_SYNC_A;
        }
    } else {
        /* Failed to send this update */
        osd_hk_msg_dropped++;
    }
    osd_packet++;
    if (osd_packet > OSD_HK_PKT_TYPE_MODE) {
        osd_packet = OSD_HK_PKT_TYPE_MISC;
    }
}

static UAVObjEvent ev;

static int32_t osdoutputStart(void)
{
    if (osdoutputEnabled) {
        /* Start a periodic timer to kick sending of an update */
        EventPeriodicCallbackCreate(&ev, send_update, 25 / portTICK_RATE_MS);
        return 0;
    }
    return -1;
}

static int32_t osdoutputInitialize(void)
{
    osd_hk_com_id    = PIOS_COM_OSDHK;
#ifdef MODULE_OSDOUTPUT_BUILTIN
    osdoutputEnabled = 1;
#else
    HwSettingsInitialize();
    uint8_t optionalModules[HWSETTINGS_OPTIONALMODULES_NUMELEM];
    HwSettingsOptionalModulesGet(optionalModules);
    if (optionalModules[HWSETTINGS_OPTIONALMODULES_OSDHK] == HWSETTINGS_OPTIONALMODULES_ENABLED) {
        osdoutputEnabled = 1;
    } else {
        osdoutputEnabled = 0;
    }
#endif
    return 0;
}
MODULE_INITCALL(osdoutputInitialize, osdoutputStart);

/**
 * @}
 * @}
 */
