/**
 ******************************************************************************

 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup RadioComBridgeModule Com Port to Radio Bridge Module
 * @brief Bridge Com and Radio ports
 * @{
 *
 * @file       RadioComBridge.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012-2013.
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

// ****************

#include <openpilot.h>
#include <radiocombridge.h>
#include <oplinkstatus.h>
#include <objectpersistence.h>
#include <oplinksettings.h>
#include <oplinkreceiver.h>
#include <radiocombridgestats.h>
#include <uavtalk_priv.h>
#include <pios_rfm22b.h>
#include <ecc.h>
#if defined(PIOS_INCLUDE_FLASH_EEPROM)
#include <pios_eeprom.h>
#endif

#include <stdbool.h>

// ****************
// Private constants

#define STACK_SIZE_BYTES  150
#define TASK_PRIORITY     (tskIDLE_PRIORITY + 1)
#define MAX_RETRIES       2
#define RETRY_TIMEOUT_MS  20
#define EVENT_QUEUE_SIZE  10
#define MAX_PORT_DELAY    200
#define SERIAL_RX_BUF_LEN 100
#define PPM_INPUT_TIMEOUT 100


// ****************
// Private types

typedef struct {
    // The task handles.
    xTaskHandle telemetryTxTaskHandle;
    xTaskHandle telemetryRxTaskHandle;
    xTaskHandle radioTxTaskHandle;
    xTaskHandle radioRxTaskHandle;
    xTaskHandle PPMInputTaskHandle;
    xTaskHandle serialRxTaskHandle;

    // The UAVTalk connection on the com side.
    UAVTalkConnection telemUAVTalkCon;
    UAVTalkConnection radioUAVTalkCon;

    // Queue handles.
    xQueueHandle uavtalkEventQueue;
    xQueueHandle radioEventQueue;

    // The raw serial Rx buffer
    uint8_t  serialRxBuf[SERIAL_RX_BUF_LEN];

    // Error statistics.
    uint32_t telemetryTxRetries;
    uint32_t radioTxRetries;

    // Is this modem the coordinator
    bool     isCoordinator;

    // Should we parse UAVTalk?
    bool     parseUAVTalk;

    // The current configured uart speed
    OPLinkSettingsComSpeedOptions comSpeed;
} RadioComBridgeData;

// ****************
// Private functions

static void telemetryTxTask(void *parameters);
static void telemetryRxTask(void *parameters);
static void serialRxTask(void *parameters);
static void radioTxTask(void *parameters);
static void radioRxTask(void *parameters);
static void PPMInputTask(void *parameters);
static int32_t UAVTalkSendHandler(uint8_t *buf, int32_t length);
static int32_t RadioSendHandler(uint8_t *buf, int32_t length);
static void ProcessTelemetryStream(UAVTalkConnection inConnectionHandle, UAVTalkConnection outConnectionHandle, uint8_t rxbyte);
static void ProcessRadioStream(UAVTalkConnection inConnectionHandle, UAVTalkConnection outConnectionHandle, uint8_t rxbyte);
static void objectPersistenceUpdatedCb(UAVObjEvent *objEv);
static void registerObject(UAVObjHandle obj);

// ****************
// Private variables

static RadioComBridgeData *data;

/**
 * @brief Start the module
 *
 * @return -1 if initialisation failed, 0 on success
 */
static int32_t RadioComBridgeStart(void)
{
    if (data) {
        // Get the settings.
        OPLinkSettingsData oplinkSettings;
        OPLinkSettingsGet(&oplinkSettings);

        // Check if this is the coordinator modem
        data->isCoordinator = (oplinkSettings.Coordinator == OPLINKSETTINGS_COORDINATOR_TRUE);

        // We will not parse/send UAVTalk if any ports are configured as Serial (except for over the USB HID port).
        data->parseUAVTalk  = ((oplinkSettings.MainPort != OPLINKSETTINGS_MAINPORT_SERIAL) &&
                               (oplinkSettings.FlexiPort != OPLINKSETTINGS_FLEXIPORT_SERIAL) &&
                               (oplinkSettings.VCPPort != OPLINKSETTINGS_VCPPORT_SERIAL));

        // Set the maximum radio RF power.
        switch (oplinkSettings.MaxRFPower) {
        case OPLINKSETTINGS_MAXRFPOWER_125:
            PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_0);
            break;
        case OPLINKSETTINGS_MAXRFPOWER_16:
            PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_1);
            break;
        case OPLINKSETTINGS_MAXRFPOWER_316:
            PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_2);
            break;
        case OPLINKSETTINGS_MAXRFPOWER_63:
            PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_3);
            break;
        case OPLINKSETTINGS_MAXRFPOWER_126:
            PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_4);
            break;
        case OPLINKSETTINGS_MAXRFPOWER_25:
            PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_5);
            break;
        case OPLINKSETTINGS_MAXRFPOWER_50:
            PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_6);
            break;
        case OPLINKSETTINGS_MAXRFPOWER_100:
            PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_7);
            break;
        default:
            // do nothing
            break;
        }

        // Configure our UAVObjects for updates.
        UAVObjConnectQueue(UAVObjGetByID(OPLINKSTATUS_OBJID), data->uavtalkEventQueue, EV_UPDATED | EV_UPDATED_MANUAL | EV_UPDATE_REQ);
        UAVObjConnectQueue(UAVObjGetByID(OBJECTPERSISTENCE_OBJID), data->uavtalkEventQueue, EV_UPDATED | EV_UPDATED_MANUAL);
        if (data->isCoordinator) {
            UAVObjConnectQueue(UAVObjGetByID(OPLINKRECEIVER_OBJID), data->radioEventQueue, EV_UPDATED | EV_UPDATED_MANUAL | EV_UPDATE_REQ);
        } else {
            UAVObjConnectQueue(UAVObjGetByID(OPLINKRECEIVER_OBJID), data->uavtalkEventQueue, EV_UPDATED | EV_UPDATED_MANUAL | EV_UPDATE_REQ);
        }

        if (data->isCoordinator) {
            registerObject(RadioComBridgeStatsHandle());
        }

        // Configure the UAVObject callbacks
        ObjectPersistenceConnectCallback(&objectPersistenceUpdatedCb);

        // Start the primary tasks for receiving/sending UAVTalk packets from the GCS.
        xTaskCreate(telemetryTxTask, "telemetryTxTask", STACK_SIZE_BYTES, NULL, TASK_PRIORITY, &(data->telemetryTxTaskHandle));
        xTaskCreate(telemetryRxTask, "telemetryRxTask", STACK_SIZE_BYTES, NULL, TASK_PRIORITY, &(data->telemetryRxTaskHandle));
        if (PIOS_PPM_RECEIVER != 0) {
            xTaskCreate(PPMInputTask, "PPMInputTask", STACK_SIZE_BYTES, NULL, TASK_PRIORITY, &(data->PPMInputTaskHandle));
#ifdef PIOS_INCLUDE_WDG
            PIOS_WDG_RegisterFlag(PIOS_WDG_PPMINPUT);
#endif
        }
        if (!data->parseUAVTalk) {
            // If the user wants raw serial communication, we need to spawn another thread to handle it.
            xTaskCreate(serialRxTask, "serialRxTask", STACK_SIZE_BYTES, NULL, TASK_PRIORITY, &(data->serialRxTaskHandle));
#ifdef PIOS_INCLUDE_WDG
            PIOS_WDG_RegisterFlag(PIOS_WDG_SERIALRX);
#endif
        }
        xTaskCreate(radioTxTask, "radioTxTask", STACK_SIZE_BYTES, NULL, TASK_PRIORITY, &(data->radioTxTaskHandle));
        xTaskCreate(radioRxTask, "radioRxTask", STACK_SIZE_BYTES, NULL, TASK_PRIORITY, &(data->radioRxTaskHandle));

        // Register the watchdog timers.
#ifdef PIOS_INCLUDE_WDG
        PIOS_WDG_RegisterFlag(PIOS_WDG_TELEMETRYTX);
        PIOS_WDG_RegisterFlag(PIOS_WDG_TELEMETRYRX);
        PIOS_WDG_RegisterFlag(PIOS_WDG_RADIOTX);
        PIOS_WDG_RegisterFlag(PIOS_WDG_RADIORX);
#endif
        return 0;
    }

    return -1;
}

/**
 * @brief Initialise the module
 *
 * @return -1 if initialisation failed on success
 */
static int32_t RadioComBridgeInitialize(void)
{
    // allocate and initialize the static data storage only if module is enabled
    data = (RadioComBridgeData *)pios_malloc(sizeof(RadioComBridgeData));
    if (!data) {
        return -1;
    }

    // Initialize the UAVObjects that we use
    OPLinkStatusInitialize();
    ObjectPersistenceInitialize();
    OPLinkReceiverInitialize();
    RadioComBridgeStatsInitialize();

    // Initialise UAVTalk
    data->telemUAVTalkCon    = UAVTalkInitialize(&UAVTalkSendHandler);
    data->radioUAVTalkCon    = UAVTalkInitialize(&RadioSendHandler);

    // Initialize the queues.
    data->uavtalkEventQueue  = xQueueCreate(EVENT_QUEUE_SIZE, sizeof(UAVObjEvent));
    data->radioEventQueue    = xQueueCreate(EVENT_QUEUE_SIZE, sizeof(UAVObjEvent));

    // Initialize the statistics.
    data->telemetryTxRetries = 0;
    data->radioTxRetries     = 0;

    data->parseUAVTalk = true;
    data->comSpeed     = OPLINKSETTINGS_COMSPEED_9600;
    PIOS_COM_RADIO     = PIOS_COM_RFM22B;

    return 0;
}
MODULE_INITCALL(RadioComBridgeInitialize, RadioComBridgeStart);


// TODO this code (badly) duplicates code from telemetry.c
// This method should be used only for periodically updated objects.
// The register method defined in telemetry.c should be factored out in a shared location so it can be
// used from here...
static void registerObject(UAVObjHandle obj)
{
    // Setup object for periodic updates
    UAVObjEvent ev = {
        .obj    = obj,
        .instId = UAVOBJ_ALL_INSTANCES,
        .event  = EV_UPDATED_PERIODIC,
        .lowPriority = true,
    };

    // Get metadata
    UAVObjMetadata metadata;

    UAVObjGetMetadata(obj, &metadata);

    EventPeriodicQueueCreate(&ev, data->uavtalkEventQueue, metadata.telemetryUpdatePeriod);
    UAVObjConnectQueue(obj, data->uavtalkEventQueue, EV_UPDATED_PERIODIC | EV_UPDATED_MANUAL | EV_UPDATE_REQ);
}

/**
 * Update telemetry statistics
 */
static void updateRadioComBridgeStats()
{
    UAVTalkStats telemetryUAVTalkStats;
    UAVTalkStats radioUAVTalkStats;
    RadioComBridgeStatsData radioComBridgeStats;

    // Get telemetry stats
    UAVTalkGetStats(data->telemUAVTalkCon, &telemetryUAVTalkStats, true);

    // Get radio stats
    UAVTalkGetStats(data->radioUAVTalkCon, &radioUAVTalkStats, true);

    // Get stats object data
    RadioComBridgeStatsGet(&radioComBridgeStats);

    radioComBridgeStats.TelemetryTxRetries     = data->telemetryTxRetries;
    radioComBridgeStats.RadioTxRetries         = data->radioTxRetries;

    // Update stats object
    radioComBridgeStats.TelemetryTxBytes      += telemetryUAVTalkStats.txBytes;
    radioComBridgeStats.TelemetryTxFailures   += telemetryUAVTalkStats.txErrors;

    radioComBridgeStats.TelemetryRxBytes      += telemetryUAVTalkStats.rxBytes;
    radioComBridgeStats.TelemetryRxFailures   += telemetryUAVTalkStats.rxErrors;
    radioComBridgeStats.TelemetryRxSyncErrors += telemetryUAVTalkStats.rxSyncErrors;
    radioComBridgeStats.TelemetryRxCrcErrors  += telemetryUAVTalkStats.rxCrcErrors;

    radioComBridgeStats.RadioTxBytes += radioUAVTalkStats.txBytes;
    radioComBridgeStats.RadioTxFailures       += radioUAVTalkStats.txErrors;

    radioComBridgeStats.RadioRxBytes += radioUAVTalkStats.rxBytes;
    radioComBridgeStats.RadioRxFailures       += radioUAVTalkStats.rxErrors;
    radioComBridgeStats.RadioRxSyncErrors     += radioUAVTalkStats.rxSyncErrors;
    radioComBridgeStats.RadioRxCrcErrors      += radioUAVTalkStats.rxCrcErrors;

    // Update stats object data
    RadioComBridgeStatsSet(&radioComBridgeStats);
}

/**
 * @brief Telemetry transmit task, regular priority
 *
 * @param[in] parameters  The task parameters
 */
static void telemetryTxTask(__attribute__((unused)) void *parameters)
{
    UAVObjEvent ev;

    // Loop forever
    while (1) {
#ifdef PIOS_INCLUDE_WDG
        PIOS_WDG_UpdateFlag(PIOS_WDG_TELEMETRYTX);
#endif
        // Wait for queue message
        if (xQueueReceive(data->uavtalkEventQueue, &ev, MAX_PORT_DELAY) == pdTRUE) {
            if (ev.obj == RadioComBridgeStatsHandle()) {
                updateRadioComBridgeStats();
            }
            // Send update (with retries)
            int32_t ret = -1;
            uint32_t retries = 0;
            while (retries <= MAX_RETRIES && ret == -1) {
                ret = UAVTalkSendObject(data->telemUAVTalkCon, ev.obj, ev.instId, 0, RETRY_TIMEOUT_MS);
                if (ret == -1) {
                    ++retries;
                }
            }
            // Update stats
            data->telemetryTxRetries += retries;
        }
    }
}

/**
 * @brief Radio tx task.  Receive data packets from the com port and send to the radio.
 *
 * @param[in] parameters  The task parameters
 */
static void radioTxTask(__attribute__((unused)) void *parameters)
{
    // Task loop
    while (1) {
#ifdef PIOS_INCLUDE_WDG
        PIOS_WDG_UpdateFlag(PIOS_WDG_RADIOTX);
#endif

        // Process the radio event queue, sending UAVObjects over the radio link as necessary.
        UAVObjEvent ev;

        // Wait for queue message
        if (xQueueReceive(data->radioEventQueue, &ev, MAX_PORT_DELAY) == pdTRUE) {
            if ((ev.event == EV_UPDATED) || (ev.event == EV_UPDATE_REQ)) {
                // Send update (with retries)
                int32_t ret = -1;
                uint32_t retries = 0;
                while (retries <= MAX_RETRIES && ret == -1) {
                    ret = UAVTalkSendObject(data->radioUAVTalkCon, ev.obj, ev.instId, 0, RETRY_TIMEOUT_MS);
                    if (ret == -1) {
                        ++retries;
                    }
                }
                data->radioTxRetries += retries;
            }
        }
    }
}

/**
 * @brief Radio rx task.  Receive data packets from the radio and pass them on.
 *
 * @param[in] parameters  The task parameters
 */
static void radioRxTask(__attribute__((unused)) void *parameters)
{
    // Task loop
    while (1) {
#ifdef PIOS_INCLUDE_WDG
        PIOS_WDG_UpdateFlag(PIOS_WDG_RADIORX);
#endif
        if (PIOS_COM_RADIO) {
            uint8_t serial_data[1];
            uint16_t bytes_to_process = PIOS_COM_ReceiveBuffer(PIOS_COM_RADIO, serial_data, sizeof(serial_data), MAX_PORT_DELAY);
            if (bytes_to_process > 0) {
                if (data->parseUAVTalk) {
                    // Pass the data through the UAVTalk parser.
                    for (uint8_t i = 0; i < bytes_to_process; i++) {
                        ProcessRadioStream(data->radioUAVTalkCon, data->telemUAVTalkCon, serial_data[i]);
                    }
                } else if (PIOS_COM_TELEMETRY) {
                    // Send the data straight to the telemetry port.
                    // Following call can fail with -2 error code (buffer full) or -3 error code (could not acquire send mutex)
                    // It is the caller responsibility to retry in such cases...
                    int32_t ret   = -2;
                    uint8_t count = 5;
                    while (count-- > 0 && ret < -1) {
                        ret = PIOS_COM_SendBufferNonBlocking(PIOS_COM_TELEMETRY, serial_data, bytes_to_process);
                    }
                }
            }
        } else {
            vTaskDelay(5);
        }
    }
}

/**
 * @brief Receive telemetry from the USB/COM port.
 *
 * @param[in] parameters  The task parameters
 */
static void telemetryRxTask(__attribute__((unused)) void *parameters)
{
    // Task loop
    while (1) {
        uint32_t inputPort = data->parseUAVTalk ? PIOS_COM_TELEMETRY : 0;
#ifdef PIOS_INCLUDE_WDG
        PIOS_WDG_UpdateFlag(PIOS_WDG_TELEMETRYRX);
#endif
#if defined(PIOS_INCLUDE_USB)
        // Determine output port (USB takes priority over telemetry port)
        if (PIOS_USB_CheckAvailable(0) && PIOS_COM_TELEM_USB_HID) {
            inputPort = PIOS_COM_TELEM_USB_HID;
        }
#endif /* PIOS_INCLUDE_USB */
        if (inputPort) {
            uint8_t serial_data[1];
            uint16_t bytes_to_process = PIOS_COM_ReceiveBuffer(inputPort, serial_data, sizeof(serial_data), MAX_PORT_DELAY);
            if (bytes_to_process > 0) {
                for (uint8_t i = 0; i < bytes_to_process; i++) {
                    ProcessTelemetryStream(data->telemUAVTalkCon, data->radioUAVTalkCon, serial_data[i]);
                }
            }
        } else {
            vTaskDelay(5);
        }
    }
}

/**
 * @brief Reads the PPM input device and sends out OPLinkReceiver objects.
 *
 * @param[in] parameters  The task parameters (unused)
 */
static void PPMInputTask(__attribute__((unused)) void *parameters)
{
    xSemaphoreHandle sem = PIOS_RCVR_GetSemaphore(PIOS_PPM_RECEIVER, 1);
    int16_t channels[RFM22B_PPM_NUM_CHANNELS];

    while (1) {
 #ifdef PIOS_INCLUDE_WDG
        PIOS_WDG_UpdateFlag(PIOS_WDG_PPMINPUT);
 #endif

        // Wait for the receiver semaphore.
        if (xSemaphoreTake(sem, PPM_INPUT_TIMEOUT) == pdTRUE) {
            // Read the receiver inputs.
            for (uint8_t i = 0; i < OPLINKRECEIVER_CHANNEL_NUMELEM; ++i) {
                channels[i] = PIOS_RCVR_Read(PIOS_PPM_RECEIVER, i + 1);
            }
        } else {
            // Failsafe
            for (uint8_t i = 0; i < OPLINKRECEIVER_CHANNEL_NUMELEM; ++i) {
                channels[i] = PIOS_RCVR_INVALID;
            }
        }

        // Pass the channel values to the radio device.
        PIOS_RFM22B_PPMSet(pios_rfm22b_id, channels);
    }
}

/**
 * @brief Receive raw serial data from the USB/COM port.
 *
 * @param[in] parameters  The task parameters
 */
static void serialRxTask(__attribute__((unused)) void *parameters)
{
    // Task loop
    while (1) {
        uint32_t inputPort = PIOS_COM_TELEMETRY;
#ifdef PIOS_INCLUDE_WDG
        PIOS_WDG_UpdateFlag(PIOS_WDG_SERIALRX);
#endif
        if (inputPort && PIOS_COM_RADIO) {
            // Receive some data.
            uint16_t bytes_to_process = PIOS_COM_ReceiveBuffer(inputPort, data->serialRxBuf, sizeof(data->serialRxBuf), MAX_PORT_DELAY);

            if (bytes_to_process > 0) {
                // Send the data over the radio link.
                // Following call can fail with -2 error code (buffer full) or -3 error code (could not acquire send mutex)
                // It is the caller responsibility to retry in such cases...
                int32_t ret   = -2;
                uint8_t count = 5;
                while (count-- > 0 && ret < -1) {
                    ret = PIOS_COM_SendBufferNonBlocking(PIOS_COM_RADIO, data->serialRxBuf, bytes_to_process);
                }
            }
        } else {
            vTaskDelay(5);
        }
    }
}

/**
 * @brief Transmit data buffer to the com port.
 *
 * @param[in] buf Data buffer to send
 * @param[in] length Length of buffer
 * @return -1 on failure
 * @return number of bytes transmitted on success
 */
static int32_t UAVTalkSendHandler(uint8_t *buf, int32_t length)
{
    int32_t ret;
    uint32_t outputPort = data->parseUAVTalk ? PIOS_COM_TELEMETRY : 0;

#if defined(PIOS_INCLUDE_USB)
    // Determine output port (USB takes priority over telemetry port)
    if (PIOS_COM_TELEM_USB_HID && PIOS_COM_Available(PIOS_COM_TELEM_USB_HID)) {
        outputPort = PIOS_COM_TELEM_USB_HID;
    }
#endif /* PIOS_INCLUDE_USB */
    if (outputPort) {
        // Following call can fail with -2 error code (buffer full) or -3 error code (could not acquire send mutex)
        // It is the caller responsibility to retry in such cases...
        ret = -2;
        uint8_t count = 5;
        while (count-- > 0 && ret < -1) {
            ret = PIOS_COM_SendBufferNonBlocking(outputPort, buf, length);
        }
    } else {
        ret = -1;
    }
    return ret;
}

/**
 * Transmit data buffer to the com port.
 *
 * @param[in] buf Data buffer to send
 * @param[in] length Length of buffer
 * @return -1 on failure
 * @return number of bytes transmitted on success
 */
static int32_t RadioSendHandler(uint8_t *buf, int32_t length)
{
    if (!data->parseUAVTalk) {
        return length;
    }
    uint32_t outputPort = PIOS_COM_RADIO;

    // Don't send any data unless the radio port is available.
    if (outputPort && PIOS_COM_Available(outputPort)) {
        // Following call can fail with -2 error code (buffer full) or -3 error code (could not acquire send mutex)
        // It is the caller responsibility to retry in such cases...
        int32_t ret   = -2;
        uint8_t count = 5;
        while (count-- > 0 && ret < -1) {
            ret = PIOS_COM_SendBufferNonBlocking(outputPort, buf, length);
        }
        return ret;
    } else {
        return -1;
    }
}

/**
 * @brief Process a byte of data received on the telemetry stream
 *
 * @param[in] inConnectionHandle  The UAVTalk connection handle on the telemetry port
 * @param[in] outConnectionHandle  The UAVTalk connection handle on the radio port.
 * @param[in] rxbyte  The received byte.
 */
static void ProcessTelemetryStream(UAVTalkConnection inConnectionHandle, UAVTalkConnection outConnectionHandle, uint8_t rxbyte)
{
    // Keep reading until we receive a completed packet.
    UAVTalkRxState state = UAVTalkProcessInputStreamQuiet(inConnectionHandle, rxbyte);

    if (state == UAVTALK_STATE_COMPLETE) {
        // We only want to unpack certain telemetry objects
        uint32_t objId = UAVTalkGetPacketObjId(inConnectionHandle);
        switch (objId) {
        case OPLINKSTATUS_OBJID:
        case OPLINKSETTINGS_OBJID:
        case OPLINKRECEIVER_OBJID:
        case MetaObjectId(OPLINKSTATUS_OBJID):
        case MetaObjectId(OPLINKSETTINGS_OBJID):
        case MetaObjectId(OPLINKRECEIVER_OBJID):
            UAVTalkReceiveObject(inConnectionHandle);
            break;
        case OBJECTPERSISTENCE_OBJID:
        case MetaObjectId(OBJECTPERSISTENCE_OBJID):
            // receive object locally
            // some objects will send back a response to telemetry
            // FIXME:
            // OPLM will ack or nack all objects requests and acked object sends
            // Receiver will probably also ack / nack the same messages
            // This has some consequences like :
            // Second ack/nack will not match an open transaction or will apply to wrong transaction
            // Question : how does GCS handle receiving the same object twice
            // The OBJECTPERSISTENCE logic can be broken too if for example OPLM nacks and then REVO acks...
            UAVTalkReceiveObject(inConnectionHandle);
            // relay packet to remote modem
            UAVTalkRelayPacket(inConnectionHandle, outConnectionHandle);
            break;
        default:
            // all other packets are relayed to the remote modem
            UAVTalkRelayPacket(inConnectionHandle, outConnectionHandle);
            break;
        }
    }
}

/**
 * @brief Process a byte of data received on the radio data stream.
 *
 * @param[in] inConnectionHandle  The UAVTalk connection handle on the radio port.
 * @param[in] outConnectionHandle  The UAVTalk connection handle on the telemetry port.
 * @param[in] rxbyte  The received byte.
 */
static void ProcessRadioStream(UAVTalkConnection inConnectionHandle, UAVTalkConnection outConnectionHandle, uint8_t rxbyte)
{
    // Keep reading until we receive a completed packet.
    UAVTalkRxState state = UAVTalkProcessInputStreamQuiet(inConnectionHandle, rxbyte);

    if (state == UAVTALK_STATE_COMPLETE) {
        // We only want to unpack certain objects from the remote modem
        // Similarly we only want to relay certain objects to the telemetry port
        uint32_t objId = UAVTalkGetPacketObjId(inConnectionHandle);
        switch (objId) {
        case OPLINKSTATUS_OBJID:
        case OPLINKSETTINGS_OBJID:
        case MetaObjectId(OPLINKSTATUS_OBJID):
        case MetaObjectId(OPLINKSETTINGS_OBJID):
            // Ignore object...
            // These objects are shadowed by the modem and are not transmitted to the telemetry port
            // - OPLINKSTATUS_OBJID : ground station will receive the OPLM link status instead
            // - OPLINKSETTINGS_OBJID : ground station will read and write the OPLM settings instead
            break;
        case OPLINKRECEIVER_OBJID:
        case MetaObjectId(OPLINKRECEIVER_OBJID):
            // Receive object locally
            // These objects are received by the modem and are not transmitted to the telemetry port
            // - OPLINKRECEIVER_OBJID : not sure why
            // some objects will send back a response to the remote modem
            UAVTalkReceiveObject(inConnectionHandle);
            break;
        default:
            // all other packets are relayed to the telemetry port
            UAVTalkRelayPacket(inConnectionHandle, outConnectionHandle);
            break;
        }
    }
}

/**
 * @brief Callback that is called when the ObjectPersistence UAVObject is changed.
 * @param[in] objEv  The event that precipitated the callback.
 */
static void objectPersistenceUpdatedCb(__attribute__((unused)) UAVObjEvent *objEv)
{
    // Get the ObjectPersistence object.
    ObjectPersistenceData obj_per;

    ObjectPersistenceGet(&obj_per);

    // Is this concerning our setting object?
    if (obj_per.ObjectID == OPLINKSETTINGS_OBJID) {
        // Is this a save, load, or delete?
        bool success = false;
        switch (obj_per.Operation) {
        case OBJECTPERSISTENCE_OPERATION_LOAD:
        {
#if defined(PIOS_INCLUDE_FLASH_LOGFS_SETTINGS)
            // Load the settings.
            void *obj = UAVObjGetByID(obj_per.ObjectID);
            if (obj == 0) {
                success = false;
            } else {
                // Load selected instance
                success = (UAVObjLoad(obj, obj_per.InstanceID) == 0);
            }
#endif
            break;
        }
        case OBJECTPERSISTENCE_OPERATION_SAVE:
        {
#if defined(PIOS_INCLUDE_FLASH_LOGFS_SETTINGS)
            void *obj = UAVObjGetByID(obj_per.ObjectID);
            if (obj == 0) {
                success = false;
            } else {
                // Save selected instance
                success = UAVObjSave(obj, obj_per.InstanceID) == 0;
            }
#endif
            break;
        }
        case OBJECTPERSISTENCE_OPERATION_DELETE:
        {
#if defined(PIOS_INCLUDE_FLASH_LOGFS_SETTINGS)
            void *obj = UAVObjGetByID(obj_per.ObjectID);
            if (obj == 0) {
                success = false;
            } else {
                // Save selected instance
                success = UAVObjDelete(obj, obj_per.InstanceID) == 0;
            }
#endif
            break;
        }
        default:
            break;
        }
        if (success == true) {
            obj_per.Operation = OBJECTPERSISTENCE_OPERATION_COMPLETED;
            ObjectPersistenceSet(&obj_per);
        }
    }
}
