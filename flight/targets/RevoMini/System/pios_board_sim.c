/**
 ******************************************************************************
 *
 * @file       pios_board.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Defines board specific static initializers for hardware for the OpenPilot board.
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
#include <pios_com_priv.h>
#include <pios_tcp_priv.h>
#include <pios_udp_priv.h>
#include <openpilot.h>
#include <uavobjectsinit.h>

#include "accels.h"
#include "baroaltitude.h"
#include "gpsposition.h"
#include "gyros.h"
#include "gyrosbias.h"
#include "magnetometer.h"
#include "manualcontrolsettings.h"

#include "pios_rcvr_priv.h"
#include "pios_gcsrcvr_priv.h"

void Stack_Change() {
}

void Stack_Change_Weak() {
}


const struct pios_tcp_cfg pios_tcp_telem_cfg = {
  .ip = "0.0.0.0",
  .port = 9001,
};

const struct pios_udp_cfg pios_udp_telem_cfg = {
	.ip = "0.0.0.0",
	.port = 9001,
};

const struct pios_tcp_cfg pios_tcp_gps_cfg = {
  .ip = "0.0.0.0",
  .port = 9001,
};
const struct pios_tcp_cfg pios_tcp_debug_cfg = {
  .ip = "0.0.0.0",
  .port = 9002,
};

#ifdef PIOS_COM_AUX
/*
 * AUX USART
 */
const struct pios_tcp_cfg pios_tcp_aux_cfg = {
  .ip = "0.0.0.0",
  .port = 9003,
};
#endif

#define PIOS_COM_TELEM_RF_RX_BUF_LEN 192
#define PIOS_COM_TELEM_RF_TX_BUF_LEN 192
#define PIOS_COM_GPS_RX_BUF_LEN 96

/*
 * Board specific number of devices.
 */
/*
struct pios_udp_dev pios_udp_devs[] = {
#define PIOS_UDP_TELEM  0
  {
    .cfg = &pios_udp0_cfg,
  },
#define PIOS_UDP_GPS    1
  {
    .cfg = &pios_udp1_cfg,
  },
#define PIOS_UDP_LOCAL    2
  {
    .cfg = &pios_udp2_cfg,
  },
#ifdef PIOS_COM_AUX
#define PIOS_UDP_AUX    3
  {
    .cfg = &pios_udp3_cfg,
  },
#endif
};

uint8_t pios_udp_num_devices = NELEMENTS(pios_udp_devs);
*/
/*
 * COM devices
 */

/*
 * Board specific number of devices.
 */
extern const struct pios_com_driver pios_serial_com_driver;
extern const struct pios_com_driver pios_udp_com_driver;
extern const struct pios_com_driver pios_tcp_com_driver;

uint32_t pios_com_telem_rf_id;
uint32_t pios_com_telem_usb_id;
uint32_t pios_com_gps_id;
uint32_t pios_com_aux_id;
uint32_t pios_com_spectrum_id;
uint32_t pios_rcvr_group_map[MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE];

/**
 * PIOS_Board_Init()
 * initializes all the core systems on this specific hardware
 * called from System/openpilot.c
 */
void PIOS_Board_Init(void) {

	/* Delay system */
	PIOS_DELAY_Init();

	/* Initialize UAVObject libraries */
	EventDispatcherInitialize();
	UAVObjInitialize();
	UAVObjectsInitializeAll();

	AccelsInitialize();
	BaroAltitudeInitialize();
	MagnetometerInitialize();
	GPSPositionInitialize();
	GyrosInitialize();
	GyrosBiasInitialize();

	/* Initialize the alarms library */
	AlarmsInitialize();

	/* Initialize the task monitor library */
	TaskMonitorInitialize();

#if defined(PIOS_INCLUDE_COM)
#if defined(PIOS_INCLUDE_TELEMETRY_RF) && 1
	{
		uint32_t pios_tcp_telem_rf_id;
		if (PIOS_TCP_Init(&pios_tcp_telem_rf_id, &pios_tcp_telem_cfg)) {
			PIOS_Assert(0);
		}

		uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_RX_BUF_LEN);
		uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_TX_BUF_LEN);
		PIOS_Assert(rx_buffer);
		PIOS_Assert(tx_buffer);
		if (PIOS_COM_Init(&pios_com_telem_rf_id, &pios_tcp_com_driver, pios_tcp_telem_rf_id,
						  rx_buffer, PIOS_COM_TELEM_RF_RX_BUF_LEN,
						  tx_buffer, PIOS_COM_TELEM_RF_TX_BUF_LEN)) {
			PIOS_Assert(0);
		}
	}
#endif /* PIOS_INCLUDE_TELEMETRY_RF */

#if defined(PIOS_INCLUDE_TELEMETRY_RF) && 0
	{
		uint32_t pios_udp_telem_rf_id;
		if (PIOS_UDP_Init(&pios_udp_telem_rf_id, &pios_udp_telem_cfg)) {
			PIOS_Assert(0);
		}
		
		uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_RX_BUF_LEN);
		uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_TX_BUF_LEN);
		PIOS_Assert(rx_buffer);
		PIOS_Assert(tx_buffer);
		if (PIOS_COM_Init(&pios_com_telem_rf_id, &pios_udp_com_driver, pios_udp_telem_rf_id,
						  rx_buffer, PIOS_COM_TELEM_RF_RX_BUF_LEN,
						  tx_buffer, PIOS_COM_TELEM_RF_TX_BUF_LEN)) {
			PIOS_Assert(0);
		}
	}
#endif /* PIOS_INCLUDE_TELEMETRY_RF */


#if defined(PIOS_INCLUDE_GPS)
	{
		uint32_t pios_tcp_gps_id;
		if (PIOS_TCP_Init(&pios_tcp_gps_id, &pios_tcp_gps_cfg)) {
			PIOS_Assert(0);
		}
		uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_GPS_RX_BUF_LEN);
		PIOS_Assert(rx_buffer);
		if (PIOS_COM_Init(&pios_com_gps_id, &pios_tcp_com_driver, pios_tcp_gps_id,
				  rx_buffer, PIOS_COM_GPS_RX_BUF_LEN,
				  NULL, 0)) {
			PIOS_Assert(0);
		}
	}
#endif	/* PIOS_INCLUDE_GPS */
#endif

#if defined(PIOS_INCLUDE_GCSRCVR)
	GCSReceiverInitialize();
	uint32_t pios_gcsrcvr_id;
	PIOS_GCSRCVR_Init(&pios_gcsrcvr_id);
	uint32_t pios_gcsrcvr_rcvr_id;
	if (PIOS_RCVR_Init(&pios_gcsrcvr_rcvr_id, &pios_gcsrcvr_rcvr_driver, pios_gcsrcvr_id)) {
		PIOS_Assert(0);
	}
	pios_rcvr_group_map[MANUALCONTROLSETTINGS_CHANNELGROUPS_GCS] = pios_gcsrcvr_rcvr_id;
#endif	/* PIOS_INCLUDE_GCSRCVR */

}

/**
 * @}
 */
