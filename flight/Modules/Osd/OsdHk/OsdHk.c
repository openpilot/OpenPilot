/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup HKOSDModule HK OSD Module
 * @brief On screen display support
 * @{ 
 *
 * @file       OsdHk.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Interfacing with HobbyKing OSD module
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
#include "positionactual.h"
#endif

#include "systemalarms.h"
#include "attitudeactual.h"
#include "hwsettings.h"

static bool osdhkEnabled;


enum osd_hk_sync {
	OSD_HK_SYNC_A = 0xCB,
	OSD_HK_SYNC_B = 0x34,
};

enum osd_hk_pkt_type {
	OSD_HK_PKT_TYPE_MISC  = 0,
	OSD_HK_PKT_TYPE_NAV   = 1,
	OSD_HK_PKT_TYPE_MAINT = 2,
	OSD_HK_PKT_TYPE_ATT = 3,
};

enum osd_hk_control_mode {
	OSD_HK_CONTROL_MODE_MANUAL     = 0,
	OSD_HK_CONTROL_MODE_STABILIZED = 1,
	OSD_HK_CONTROL_MODE_AUTO       = 2,
};

struct osd_hk_blob_misc {
	uint8_t  type;		/* Always OSD_HK_PKT_TYPE_MISC */
	int16_t   roll;
	int16_t   pitch;
	//uint16_t home;		/* Big Endian */
	enum osd_hk_control_mode control_mode;
	uint8_t  low_battery;
	uint16_t current;	/* Big Endian */
} __attribute__((packed));

struct osd_hk_blob_att {
	uint8_t  type;		/* Always OSD_HK_PKT_TYPE_ATT */
	int16_t   roll;
	int16_t   pitch;
	int16_t yaw;
	int16_t speed;	/* Big Endian */
} __attribute__((packed));

struct osd_hk_blob_nav {
	uint8_t  type;		/* Always OSD_HK_PKT_TYPE_NAV */
	uint32_t gps_lat;	/* Big Endian */
	uint32_t gps_lon;	/* Big Endian */
} __attribute__((packed));

struct osd_hk_blob_maint {
	uint8_t  type;		/* Always OSD_HK_PKT_TYPE_MAINT */
	uint8_t  gps_speed;
	uint16_t gps_alt;	/* Big Endian */
	uint16_t gps_dis;	/* Big Endian */
	uint8_t  status;
	uint8_t  config;
	uint8_t  emerg;
} __attribute__((packed));

union osd_hk_pkt_blobs
{
	struct osd_hk_blob_misc  misc;
	struct osd_hk_blob_nav   nav;
	struct osd_hk_blob_maint maint;
	struct osd_hk_blob_att att;
} __attribute__((packed));

struct osd_hk_msg {
	enum osd_hk_sync sync;
	enum osd_hk_pkt_type t;
	union osd_hk_pkt_blobs v;
} __attribute__((packed));

#if 0
#if sizeof(struct osd_hk_msg) != 11
#error struct osd_hk_msg is the wrong size, something is wrong with the definition
#endif
#endif

static struct osd_hk_msg osd_hk_msg_buf;

static volatile bool newPositionActualData = false;
static volatile bool newBattData = false;
static volatile bool newAttitudeData = false;
static volatile bool newAlarmData = false;

static uint32_t osd_hk_com_id;
static uint8_t osd_hk_msg_dropped;

static void send_update(UAVObjEvent * ev)
{
	static enum osd_hk_sync sync = OSD_HK_SYNC_A;

	struct osd_hk_msg * msg = &osd_hk_msg_buf;
	union osd_hk_pkt_blobs * blob = &(osd_hk_msg_buf.v);

	/* Make sure we have a COM port bound */
	if (!osd_hk_com_id) {
		return;
	}

	/* 
	 * Set up the message
	 *
	 * NOTE: Only packet type 0 (MISC) is supported so far
	 */
	msg->sync = sync;
	msg->t = OSD_HK_PKT_TYPE_ATT;

	if (newAttitudeData) {
		float roll;
		AttitudeActualRollGet(&roll);
		//blob->misc.roll = (int16_t) roll;
		blob->att.roll = (int16_t) roll;
		
		float pitch;
		AttitudeActualPitchGet(&pitch);
		//blob->misc.pitch = (int16_t) pitch;
		blob->att.pitch = (int16_t) pitch;

		float yaw;
		AttitudeActualYawGet(&yaw);
		blob->att.yaw = (int16_t) yaw;
	}

	/* Field not supported yet */
	//blob->misc.control_mode = 0;

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
	//blob->misc.current = 0;
#endif

#if POSITIONACTUAL_SUPPORTED
	if (newPositionActualData) {
		PositionActualData position;
		PositionActualGet(&position);

		/* compute 3D distance */
		float d = sqrt(
			pow(position.North,2) +
			pow(position.East,2)  +
			pow(position.Down,2));
		/* convert from cm to dm (10ths of m) */
		uint16_t home = (uint16_t)(d / 10);

		/* convert to big endian */
		blob->misc.home = (
			(home & 0xFF00 >> 8) |
			(home & 0x00FF << 8));

		newPositionActualData = false;
	}
#else
	//blob->misc.home = 0;
#endif

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
}

#if FLIGHTBATTERYSTATE_SUPPORTED
static void FlightBatteryStateUpdatedCb(UAVObjEvent * ev)
{
	newBattData = true;
}
#endif

#if POSITIONACTUAL_SUPPORTED
static void PositionActualUpdatedCb(UAVObjEvent * ev)
{
	newPositionActualData = true;
}
#endif

static void AttitudeActualUpdatedCb(UAVObjEvent * ev)
{
	newAttitudeData = true;
}

static void SystemAlarmsUpdatedCb(UAVObjEvent * ev)
{
	newAlarmData = true;
}

static UAVObjEvent ev;

static int32_t OsdHkStart(void)
{
	if (osdhkEnabled) {
		// Start main task
		AttitudeActualConnectCallback(AttitudeActualUpdatedCb);

		SystemAlarmsConnectCallback(SystemAlarmsUpdatedCb);

#if FLIGHTBATTERYSTATE_SUPPORTED
		FlightBatteryStateConnectCallback(FlightBatteryStateUpdatedCb);
#endif

#if POSITIONACTUAL_SUPPORTED
		PositionActualConnectCallback(PositionActualUpdatedCb);
#endif

		/* Start a periodic timer to kick sending of an update */
		EventPeriodicCallbackCreate(&ev, send_update, 25 / portTICK_RATE_MS);
		return 0;
	}
	return -1;
}

static int32_t OsdHkInitialize(void)
{
	osd_hk_com_id = PIOS_COM_OSDHK;
#ifdef MODULE_OsdHk_BUILTIN
	osdhkEnabled = 1;
#else
	HwSettingsInitialize();
	uint8_t optionalModules[HWSETTINGS_OPTIONALMODULES_NUMELEM];
	HwSettingsOptionalModulesGet(optionalModules);
	if (optionalModules[HWSETTINGS_OPTIONALMODULES_OSDHK] == HWSETTINGS_OPTIONALMODULES_ENABLED) {
		osdhkEnabled = 1;
	} else {
		osdhkEnabled = 0;
	}
#endif
	return 0;
}
MODULE_INITCALL(OsdHkInitialize, OsdHkStart)

/**
 * @}
 * @}
 */
