/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup Radio Input / Output Module
 * @brief Read and Write packets from/to a radio device.
 * @{ 
 *
 * @file       radio.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Bridges selected Com Port to the COM VCP emulated serial port
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
#include <pios_board_info.h>
#include <openpilot.h>
#include <gcsreceiver.h>
#include <hwsettings.h>
#include <pipxsettings.h>
#include <pipxstatus.h>
#include <packet_handler.h>
#include <pios_com_priv.h>
#include <pios_rfm22b_priv.h>
#include <radio.h>

// ****************
// Private constants

#define STACK_SIZE_BYTES 200
#define TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define PACKET_QUEUE_SIZE PIOS_PH_WIN_SIZE
#define MAX_PORT_DELAY 200
#define STATS_UPDATE_PERIOD_MS 500
#define RADIOSTATS_UPDATE_PERIOD_MS 250
#define MAX_LOST_CONTACT_TIME 4
#define PACKET_MAX_DELAY 50

#ifndef LINK_LED_ON
#define LINK_LED_ON
#define LINK_LED_OFF
#endif

// ****************
// Private types

typedef struct {
	uint32_t pairID;
	uint16_t retries;
	uint16_t errors;
	uint16_t uavtalk_errors;
	uint16_t resets;
	uint16_t dropped;
	int8_t rssi;
	uint8_t lastContact;
} PairStats;

typedef struct {

	// The task handles.
	xTaskHandle radioReceiveTaskHandle;
	xTaskHandle radioStatusTaskHandle;

	// Queue handles.
	xQueueHandle radioPacketQueue;

	// Error statistics.
	uint32_t radioTxErrors;
	uint32_t radioRxErrors;
	uint16_t txBytes;
	uint16_t rxBytes;

	// External error statistics
	uint32_t droppedPackets;
	uint32_t comTxRetries;
	uint32_t UAVTalkErrors;

	// The destination ID
	uint32_t destination_id;

	// Track other radios that are in range.
	PairStats pairStats[PIPXSTATUS_PAIRIDS_NUMELEM];

	// The RSSI of the last packet received.
	int8_t RSSI;

} RadioData;

// ****************
// Private functions

static void radioReceiveTask(void *parameters);
static void radioStatusTask(void *parameters);
static void StatusHandler(PHStatusPacketHandle p, int8_t rssi, int8_t afc);
static int32_t transmitPacket(PHPacketHandle packet);
static void PPMHandler(uint16_t *channels);

// ****************
// Private variables

static RadioData *data = 0;

// ****************
// Global variables
uint32_t pios_rfm22b_id = 0;
uint32_t pios_com_rfm22b_id = 0;
uint32_t pios_packet_handler = 0;
const struct pios_rfm22b_cfg *pios_rfm22b_cfg;

// ***************
// External functions
extern const struct pios_rfm22b_cfg * PIOS_BOARD_HW_DEFS_GetRfm22Cfg (uint32_t board_revision);

/**
 * Start the module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
static int32_t RadioStart(void)
{
	if (!data)
		return -1;

	// Start the tasks.
	xTaskCreate(radioReceiveTask, (signed char *)"RadioReceive", STACK_SIZE_BYTES, NULL, TASK_PRIORITY, &(data->radioReceiveTaskHandle));
	xTaskCreate(radioStatusTask, (signed char *)"RadioStatus", STACK_SIZE_BYTES * 2, NULL, TASK_PRIORITY, &(data->radioStatusTaskHandle));

	// Install the monitors
	TaskMonitorAdd(TASKINFO_RUNNING_MODEMRX, data->radioReceiveTaskHandle);
	TaskMonitorAdd(TASKINFO_RUNNING_MODEMSTAT, data->radioStatusTaskHandle);

	// Register the watchdog timers.
#ifdef PIOS_WDG_RADIORECEIVE
	PIOS_WDG_RegisterFlag(PIOS_WDG_RADIORECEIVE);
#endif /* PIOS_WDG_RADIORECEIVE */

	return 0;
}


/**
 * Initialise the module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
static int32_t RadioInitialize(void)
{

	// See if this module is enabled.
#ifndef RADIO_BUILTIN
	HwSettingsInitialize();
	uint8_t optionalModules[HWSETTINGS_OPTIONALMODULES_NUMELEM];
	HwSettingsOptionalModulesGet(optionalModules);
	if (optionalModules[HWSETTINGS_OPTIONALMODULES_RADIO] != HWSETTINGS_OPTIONALMODULES_ENABLED) {
		pios_packet_handler = 0;
		return -1;
	}
#endif

	// Initalize out UAVOs
	PipXSettingsInitialize();
	PipXStatusInitialize();

	PipXSettingsData pipxSettings;
	PipXSettingsGet(&pipxSettings);

	/* Retrieve hardware settings. */
	const struct pios_board_info * bdinfo = &pios_board_info_blob;
	pios_rfm22b_cfg = PIOS_BOARD_HW_DEFS_GetRfm22Cfg(bdinfo->board_rev);

	/* Initalize the RFM22B radio COM device. */
	if (PIOS_RFM22B_Init(&pios_rfm22b_id, PIOS_RFM22_SPI_PORT, pios_rfm22b_cfg->slave_num, pios_rfm22b_cfg))
		return -1;

	// Set the maximum radio RF power.
	switch (pipxSettings.MaxRFPower)
	{
	case PIPXSETTINGS_MAXRFPOWER_125:
		PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_0);
		break;
	case PIPXSETTINGS_MAXRFPOWER_16:
		PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_1);
		break;
	case PIPXSETTINGS_MAXRFPOWER_316:
		PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_2);
		break;
	case PIPXSETTINGS_MAXRFPOWER_63:
		PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_3);
		break;
	case PIPXSETTINGS_MAXRFPOWER_126:
		PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_4);
		break;
	case PIPXSETTINGS_MAXRFPOWER_25:
		PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_5);
		break;
	case PIPXSETTINGS_MAXRFPOWER_50:
		PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_6);
		break;
	case PIPXSETTINGS_MAXRFPOWER_100:
		PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_7);
		break;
	}

	switch (pipxSettings.RFSpeed) {
	case PIPXSETTINGS_RFSPEED_2400:
		RFM22_SetDatarate(pios_rfm22b_id, RFM22_datarate_2000, true);
		break;
	case PIPXSETTINGS_RFSPEED_4800:
		RFM22_SetDatarate(pios_rfm22b_id, RFM22_datarate_4000, true);
		break;
	case PIPXSETTINGS_RFSPEED_9600:
		RFM22_SetDatarate(pios_rfm22b_id, RFM22_datarate_9600, true);
		break;
	case PIPXSETTINGS_RFSPEED_19200:
		RFM22_SetDatarate(pios_rfm22b_id, RFM22_datarate_19200, true);
		break;
	case PIPXSETTINGS_RFSPEED_38400:
		RFM22_SetDatarate(pios_rfm22b_id, RFM22_datarate_32000, true);
		break;
	case PIPXSETTINGS_RFSPEED_57600:
		RFM22_SetDatarate(pios_rfm22b_id, RFM22_datarate_64000, true);
		break;
	case PIPXSETTINGS_RFSPEED_115200:
		RFM22_SetDatarate(pios_rfm22b_id, RFM22_datarate_128000, true);
		break;
	}

	// Set the radio destination ID.
	PIOS_RFM22B_SetDestinationId(pios_rfm22b_id, pipxSettings.PairID);

	// Initialize the packet handler
	PacketHandlerConfig pios_ph_cfg = {
		.default_destination_id = 0xffffffff, // Broadcast
		.source_id = PIOS_RFM22B_DeviceID(pios_rfm22b_id),
		.win_size = PIOS_PH_WIN_SIZE,
		.max_connections = PIOS_PH_MAX_CONNECTIONS,
	};
	pios_packet_handler = PHInitialize(&pios_ph_cfg);

	// allocate and initialize the static data storage only if module is enabled
	data = (RadioData *)pvPortMalloc(sizeof(RadioData));
	if (!data)
		return -1;

	// Initialize the statistics.
	data->radioTxErrors = 0;
	data->radioRxErrors = 0;
	data->droppedPackets = 0;
	data->comTxRetries = 0;
	data->UAVTalkErrors = 0;
	data->RSSI = -127;

	// Initialize the detected device statistics.
	for (uint8_t i = 0; i < PIPXSTATUS_PAIRIDS_NUMELEM; ++i)
	{
		data->pairStats[i].pairID = 0;
		data->pairStats[i].rssi = -127;
		data->pairStats[i].retries = 0;
		data->pairStats[i].errors = 0;
		data->pairStats[i].uavtalk_errors = 0;
		data->pairStats[i].resets = 0;
		data->pairStats[i].dropped = 0;
		data->pairStats[i].lastContact = 0;
	}
	// The first slot is reserved for our current pairID
	PipXSettingsPairIDGet(&(data->pairStats[0].pairID));
	data->destination_id = data->pairStats[0].pairID ? data->pairStats[0].pairID : 0xffffffff;

	// Register the callbacks with the packet handler
	PHRegisterStatusHandler(pios_packet_handler, StatusHandler);
	PHRegisterOutputStream(pios_packet_handler, transmitPacket);
	PHRegisterPPMHandler(pios_packet_handler, PPMHandler);

	return 0;
}
MODULE_INITCALL(RadioInitialize, RadioStart)

/**
 * The task that receives packets from the radio.
 */
static void radioReceiveTask(void *parameters)
{
	PHPacketHandle p = NULL;

	/* Handle radio -> usart/usb direction */
	while (1) {
		uint32_t rx_bytes;

#ifdef PIOS_WDG_RADIORECEIVE
		// Update the watchdog timer.
		PIOS_WDG_UpdateFlag(PIOS_WDG_RADIORECEIVE);
#endif /* PIOS_INCLUDE_WDG */

		// Receive data from the radio port
		p = NULL;
		rx_bytes = PIOS_RFM22B_Receive_Packet(pios_rfm22b_id, &p, MAX_PORT_DELAY);
		if(rx_bytes == 0)
			continue;
		data->rxBytes += rx_bytes;
		PHReceivePacket(pios_packet_handler, p);
		p = NULL;
	}
}

/**
 * Transmit a packet to the radio port.
 * \param[in] buf Data buffer to send
 * \param[in] length Length of buffer
 * \return -1 on failure
 * \return number of bytes transmitted on success
 */
static int32_t transmitPacket(PHPacketHandle p)
{
	uint16_t len = PH_PACKET_SIZE(p);
	data->txBytes += len;
	if (!PIOS_RFM22B_Send_Packet(pios_rfm22b_id, p, PACKET_MAX_DELAY))
		return -1;
	return len;
}

/**
 * Receive a status packet
 * \param[in] status The status structure
 */
static void StatusHandler(PHStatusPacketHandle status, int8_t rssi, int8_t afc)
{
	uint32_t id = status->header.source_id;
	bool found = false;
	// Have we seen this device recently?
	uint8_t id_idx = 0;
	for ( ; id_idx < PIPXSTATUS_PAIRIDS_NUMELEM; ++id_idx)
		if(data->pairStats[id_idx].pairID == id)
		{
			found = true;
			break;
		}

	// If we have seen it, update the RSSI and reset the last contact couter
	if(found)
	{
		data->pairStats[id_idx].rssi = rssi;
		data->pairStats[id_idx].retries = status->retries;
		data->pairStats[id_idx].errors = status->errors;
		data->pairStats[id_idx].uavtalk_errors = status->uavtalk_errors;
		data->pairStats[id_idx].resets = status->resets;
		data->pairStats[id_idx].dropped = status->dropped;
		data->pairStats[id_idx].lastContact = 0;
	}

	// If we haven't seen it, find a slot to put it in.
	if (!found)
	{
		uint32_t pairID;
		PipXSettingsPairIDGet(&pairID);

		uint8_t min_idx = 0;
		if(id != pairID)
		{
			int8_t min_rssi = data->pairStats[0].rssi;
			for (id_idx = 1; id_idx < PIPXSTATUS_PAIRIDS_NUMELEM; ++id_idx)
			{
				if(data->pairStats[id_idx].rssi < min_rssi)
				{
					min_rssi = data->pairStats[id_idx].rssi;
					min_idx = id_idx;
				}
			}
		}
		data->pairStats[min_idx].pairID = id;
		data->pairStats[min_idx].rssi = rssi;
		data->pairStats[min_idx].retries = status->retries;
		data->pairStats[min_idx].errors = status->errors;
		data->pairStats[min_idx].uavtalk_errors = status->uavtalk_errors;
		data->pairStats[min_idx].resets = status->resets;
		data->pairStats[min_idx].dropped = status->dropped;
		data->pairStats[min_idx].lastContact = 0;
	}
}

/**
 * The stats update task.
 */
static void radioStatusTask(void *parameters)
{
	while (1) {
		PipXStatusData pipxStatus;
		uint32_t pairID;

		// Get object data
		PipXStatusGet(&pipxStatus);
		PipXSettingsPairIDGet(&pairID);

		// Update the status
		pipxStatus.DeviceID = PIOS_RFM22B_DeviceID(pios_rfm22b_id);
		pipxStatus.Retries = data->comTxRetries;
		pipxStatus.LinkQuality = PIOS_RFM22B_LinkQuality(pios_rfm22b_id);
		pipxStatus.UAVTalkErrors = data->UAVTalkErrors;
		pipxStatus.Dropped = data->droppedPackets;
		pipxStatus.Resets = PIOS_RFM22B_Resets(pios_rfm22b_id);
		pipxStatus.TXRate = (uint16_t)((float)(data->txBytes * 1000) / STATS_UPDATE_PERIOD_MS);
		data->txBytes = 0;
		pipxStatus.RXRate = (uint16_t)((float)(data->rxBytes * 1000) / STATS_UPDATE_PERIOD_MS);
		data->rxBytes = 0;
		pipxStatus.LinkState = PIPXSTATUS_LINKSTATE_DISCONNECTED;
		pipxStatus.RSSI = PIOS_RFM22B_LinkQuality(pios_rfm22b_id);
		LINK_LED_OFF;

		// Update the potential pairing contacts
		for (uint8_t i = 0; i < PIPXSTATUS_PAIRIDS_NUMELEM; ++i)
		{
			pipxStatus.PairIDs[i] = data->pairStats[i].pairID;
			pipxStatus.PairSignalStrengths[i] = data->pairStats[i].rssi;
			data->pairStats[i].lastContact++;
			// Remove this device if it's stale.
			if(data->pairStats[i].lastContact > MAX_LOST_CONTACT_TIME)
			{
				data->pairStats[i].pairID = 0;
				data->pairStats[i].rssi = -127;
				data->pairStats[i].retries = 0;
				data->pairStats[i].errors = 0;
				data->pairStats[i].uavtalk_errors = 0;
				data->pairStats[i].resets = 0;
				data->pairStats[i].dropped = 0;
				data->pairStats[i].lastContact = 0;
			}
			// Add the paired devices statistics to ours.
			if(pairID && (data->pairStats[i].pairID == pairID) && (data->pairStats[i].rssi > -127))
			{
				pipxStatus.Retries += data->pairStats[i].retries;
				pipxStatus.UAVTalkErrors += data->pairStats[i].uavtalk_errors;
				pipxStatus.Dropped += data->pairStats[i].dropped;
				pipxStatus.Resets += data->pairStats[i].resets;
				pipxStatus.Dropped += data->pairStats[i].dropped;
				pipxStatus.LinkState = PIPXSTATUS_LINKSTATE_CONNECTED;
				LINK_LED_ON;
			}
		}

		// Update the object
		PipXStatusSet(&pipxStatus);

		vTaskDelay(STATS_UPDATE_PERIOD_MS / portTICK_RATE_MS);
	}
}

/**
 * Receive a ppm packet
 * \param[in] channels The ppm channels
 */
static void PPMHandler(uint16_t *channels)
{
	GCSReceiverData rcvr;

	// Copy the receiver channels into the GCSReceiver object.
	for (uint8_t i = 0; i < GCSRECEIVER_CHANNEL_NUMELEM; ++i)
		rcvr.Channel[i] = channels[i];

	// Set the GCSReceiverData object.
	GCSReceiverSet(&rcvr);
}
