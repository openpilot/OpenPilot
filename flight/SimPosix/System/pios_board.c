/**
 ******************************************************************************
 * @addtogroup Revolution Revolution configuration files
 * @{
 * @brief Configures the revolution board
 * @{
 *
 * @file       pios_board.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      Defines board specific static initializers for hardware for the Revolution board.
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

#include <openpilot.h>
#include <uavobjectsinit.h>
#include "hwsettings.h"
#include "manualcontrolsettings.h"

#include "board_hw_defs.c"

/**
 * Sensor configurations 
 */

/* One slot per selectable receiver group.
 *  eg. PWM, PPM, GCS, SPEKTRUM1, SPEKTRUM2, SBUS
 * NOTE: No slot in this map for NONE.
 */
uint32_t pios_rcvr_group_map[MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE];

#define PIOS_COM_TELEM_RF_RX_BUF_LEN 512
#define PIOS_COM_TELEM_RF_TX_BUF_LEN 512

#define PIOS_COM_GPS_RX_BUF_LEN 32

#define PIOS_COM_TELEM_USB_RX_BUF_LEN 65
#define PIOS_COM_TELEM_USB_TX_BUF_LEN 65

#define PIOS_COM_BRIDGE_RX_BUF_LEN 65
#define PIOS_COM_BRIDGE_TX_BUF_LEN 12

#define PIOS_COM_AUX_RX_BUF_LEN 512
#define PIOS_COM_AUX_TX_BUF_LEN 512

uint32_t pios_com_aux_id = 0;
uint32_t pios_com_gps_id = 0;
uint32_t pios_com_telem_usb_id = 0;
uint32_t pios_com_telem_rf_id = 0;
uint32_t pios_com_bridge_id = 0;

/* 
 * Setup a com port based on the passed cfg, driver and buffer sizes. tx size of -1 make the port rx only
 */
static void PIOS_Board_configure_com(const struct pios_udp_cfg *usart_port_cfg, size_t rx_buf_len, size_t tx_buf_len,
		const struct pios_com_driver *com_driver, uint32_t *pios_com_id) 
{
	uint32_t pios_usart_id;
	if (PIOS_UDP_Init(&pios_usart_id, usart_port_cfg)) {
		PIOS_Assert(0);
	}
	
	uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(rx_buf_len);
	PIOS_Assert(rx_buffer);
	if(tx_buf_len!= -1){ // this is the case for rx/tx ports
		uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(tx_buf_len);
		PIOS_Assert(tx_buffer);
		
		if (PIOS_COM_Init(pios_com_id, com_driver, pios_usart_id,
				rx_buffer, rx_buf_len,
				tx_buffer, tx_buf_len)) {
			PIOS_Assert(0);
		}
	}
	else{ //rx only port
		if (PIOS_COM_Init(pios_com_id, com_driver, pios_usart_id,
				rx_buffer, rx_buf_len,
				NULL, 0)) {
			PIOS_Assert(0);
		}
	}
}

/**
 * PIOS_Board_Init()
 * initializes all the core subsystems on this specific hardware
 * called from System/openpilot.c
 */
void PIOS_Board_Init(void) {
	
	/* Delay system */
	PIOS_DELAY_Init();

	/* Initialize UAVObject libraries */
	EventDispatcherInitialize();
	UAVObjInitialize();

	HwSettingsInitialize();
	
	UAVObjectsInitializeAll();
	
	/* Initialize the alarms library */
	AlarmsInitialize();

	/* Initialize the task monitor library */
	TaskMonitorInitialize();

	/* Configure IO ports */
	
	/* Configure Telemetry port */
	uint8_t hwsettings_rv_telemetryport;
	HwSettingsRV_TelemetryPortGet(&hwsettings_rv_telemetryport);

	switch (hwsettings_rv_telemetryport){
		case HWSETTINGS_RV_TELEMETRYPORT_DISABLED:
			break;
		case HWSETTINGS_RV_TELEMETRYPORT_TELEMETRY:
			PIOS_Board_configure_com(&pios_udp_telem_cfg, PIOS_COM_TELEM_RF_RX_BUF_LEN, PIOS_COM_TELEM_RF_TX_BUF_LEN, &pios_udp_com_driver, &pios_com_telem_rf_id);
			break;
		case HWSETTINGS_RV_TELEMETRYPORT_COMAUX:
			PIOS_Board_configure_com(&pios_udp_telem_cfg, PIOS_COM_AUX_RX_BUF_LEN, PIOS_COM_AUX_TX_BUF_LEN, &pios_udp_com_driver, &pios_com_aux_id);
			break;
			
	} /* 	hwsettings_rv_telemetryport */

	/* Configure GPS port */
	uint8_t hwsettings_rv_gpsport;
	HwSettingsRV_GPSPortGet(&hwsettings_rv_gpsport);
	switch (hwsettings_rv_gpsport){
		case HWSETTINGS_RV_GPSPORT_DISABLED:
			break;
			
		case HWSETTINGS_RV_GPSPORT_TELEMETRY:
			PIOS_Board_configure_com(&pios_udp_gps_cfg, PIOS_COM_TELEM_RF_RX_BUF_LEN, PIOS_COM_TELEM_RF_TX_BUF_LEN, &pios_udp_com_driver, &pios_com_telem_rf_id);
			break;
			
		case HWSETTINGS_RV_GPSPORT_GPS:
			PIOS_Board_configure_com(&pios_udp_gps_cfg, PIOS_COM_GPS_RX_BUF_LEN, -1,  &pios_udp_com_driver, &pios_com_gps_id);
			break;
		
		case HWSETTINGS_RV_GPSPORT_COMAUX:
			PIOS_Board_configure_com(&pios_udp_gps_cfg, PIOS_COM_AUX_RX_BUF_LEN, PIOS_COM_AUX_TX_BUF_LEN, &pios_udp_com_driver, &pios_com_aux_id);
			break;
			
	}/* hwsettings_rv_gpsport */

	/* Configure AUXPort */
	uint8_t hwsettings_rv_auxport;
	HwSettingsRV_AuxPortGet(&hwsettings_rv_auxport);

	switch (hwsettings_rv_auxport) {
		case HWSETTINGS_RV_AUXPORT_DISABLED:
			break;
			
		case HWSETTINGS_RV_AUXPORT_TELEMETRY:
			PIOS_Board_configure_com(&pios_udp_aux_cfg, PIOS_COM_TELEM_RF_RX_BUF_LEN, PIOS_COM_TELEM_RF_TX_BUF_LEN, &pios_udp_com_driver, &pios_com_telem_rf_id);
			break;
			
		case HWSETTINGS_RV_AUXPORT_COMAUX:
			PIOS_Board_configure_com(&pios_udp_aux_cfg, PIOS_COM_AUX_RX_BUF_LEN, PIOS_COM_AUX_TX_BUF_LEN, &pios_udp_com_driver, &pios_com_aux_id);
			break;
			break;
	} /* hwsettings_rv_auxport */
}

/**
 * @}
 * @}
 */

