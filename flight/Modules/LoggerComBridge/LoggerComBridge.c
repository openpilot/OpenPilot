/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup LoggerComBridgeModule Com Port to Logger Bridge Module
 * @brief Bridge Com and Logger ports
 * @{ 
 *
 * @file       LoggerComBridge.c
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

// ****************

#include <openpilot.h>
#include <loggercombridge.h>
#include <packet_handler.h>
#include <gcsreceiver.h>
#include <oplogstatus.h>
#include <objectpersistence.h>
#include <oplogsettings.h>
#include <uavtalk_priv.h>
#include <ecc.h>
#if defined(PIOS_INCLUDE_FLASH_EEPROM)
#include <pios_eeprom.h>
#endif

#include <stdbool.h>

// ****************
// Private constants

#define STACK_SIZE_BYTES 150
#define TASK_PRIORITY (tskIDLE_PRIORITY + 1)
#define MAX_RETRIES 2
#define RETRY_TIMEOUT_MS 20
#define EVENT_QUEUE_SIZE 10
#define MAX_PORT_DELAY 200
#define EV_SEND_ACK 0x20
#define EV_SEND_NACK 0x30

#define SENSOR_PERIOD 4
#define LOG_NAME "mpu6050.log"

// ****************
// Private types

typedef struct {

	// The task handles.
	xTaskHandle telemetryTxTaskHandle;
	xTaskHandle loggerRxTaskHandle;
	xTaskHandle loggerTxTaskHandle;
	xTaskHandle loggerMpuTaskHandle;

	// The UAVTalk connection on the com side.
	UAVTalkConnection outUAVTalkCon;
	UAVTalkConnection inUAVTalkCon;

	// Queue handles.
	xQueueHandle gcsEventQueue;
	xQueueHandle uavtalkEventQueue;

	// Error statistics.
	uint32_t comTxErrors;
	uint32_t comTxRetries;
	uint32_t UAVTalkErrors;
	uint32_t droppedPackets;
	
	/*
	uint8_t Selected_Buffer;
	uint8_t buffer1[512];//first buffer to write to the SD
	bool buffer1_Full;
	uint16_t Pos_In_Buffer1;
	uint8_t buffer2[512];//second buffer to write to the SD
	bool buffer2_Full;
	uint16_t Pos_In_Buffer2;*///check the stcak size
	

} LoggerComBridgeData;

/**
 * Configuration for the MPU6050 chip
 */
#if defined(PIOS_INCLUDE_MPU6050)
#include "pios_mpu6050.h"
static const struct pios_exti_cfg pios_exti_mpu6050_cfg __exti_config = {
	.vector = PIOS_MPU6050_IRQHandler,
	.line = EXTI_Line4,
	.pin = {
		.gpio = GPIOA,
		.init = {
			.GPIO_Pin   = GPIO_Pin_4,
			.GPIO_Speed = GPIO_Speed_10MHz,
			.GPIO_Mode  = GPIO_Mode_IN_FLOATING,
		},
	},
	.irq = {
		.init = {
			.NVIC_IRQChannel = EXTI4_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
			.NVIC_IRQChannelSubPriority = 0,
			.NVIC_IRQChannelCmd = ENABLE,
		},
	},
	.exti = {
		.init = {
			.EXTI_Line = EXTI_Line4, // matches above GPIO pin
			.EXTI_Mode = EXTI_Mode_Interrupt,
			.EXTI_Trigger = EXTI_Trigger_Rising,
			.EXTI_LineCmd = ENABLE,
		},
	},
};

static const struct pios_mpu6050_cfg pios_mpu6050_cfg = {
	.exti_cfg = &pios_exti_mpu6050_cfg,
	.Fifo_store = PIOS_MPU6050_FIFO_TEMP_OUT | PIOS_MPU6050_FIFO_GYRO_X_OUT | PIOS_MPU6050_FIFO_GYRO_Y_OUT | PIOS_MPU6050_FIFO_GYRO_Z_OUT,
	// Clock at 8 khz, downsampled by 8 for 1khz
	.Smpl_rate_div = 11,
	.interrupt_cfg = PIOS_MPU6050_INT_CLR_ANYRD,
	.interrupt_en = PIOS_MPU6050_INTEN_DATA_RDY,
	.User_ctl = PIOS_MPU6050_USERCTL_FIFO_EN ,
	.Pwr_mgmt_clk = PIOS_MPU6050_PWRMGMT_PLL_X_CLK,
	.accel_range = PIOS_MPU6050_ACCEL_8G,
	.gyro_range = PIOS_MPU6050_SCALE_500_DEG,
	.filter = PIOS_MPU6050_LOWPASS_256_HZ,
	.orientation = PIOS_MPU6050_TOP_180DEG
};
#endif /* PIOS_INCLUDE_MPU6050 */

#if defined(PIOS_INCLUDE_MPU6050)
float accels[3], gyros[3];
float temperature;
#endif

// ****************
// Private functions

static void telemetryTxTask(void *parameters);
static void loggerRxTask(void *parameters);
static void loggerTxTask(void *parameters);
static void loggerMpuTask(void *parameters);
static int32_t UAVTalkSendHandler(uint8_t *buf, int32_t length);
static int32_t LoggerSendHandler(uint8_t *buf, int32_t length);
static void ProcessInputStream(UAVTalkConnection connectionHandle, uint8_t rxbyte);
static void queueEvent(xQueueHandle queue, void *obj, uint16_t instId, UAVObjEventType type);
//static void configureComCallback(OPLogSettingsOutputConnectionOptions com_port, OPLogSettingsComSpeedOptions com_speed);
static void updateSettings();
static int32_t updateSensors(void);
void printFloat(float v, uint8_t decimalDigits);

// ****************
// Private variables

static LoggerComBridgeData *data;

int16_t intPart, fractPart;
uint8_t sensPart;

FILEINFO File;
char Buffer[1024];
uint32_t Cache;

/**
 * Start the module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
static int32_t LoggerComBridgeStart(void)
{
	if(data) {

		// Configure the com port configuration callback
		//PIOS_RFM22B_SetComConfigCallback(pios_rfm22b_id, &configureComCallback);

		// Set the baudrates, etc.
		updateSettings();

		// Start the primary tasks for receiving/sending UAVTalk packets from the GCS.
		xTaskCreate(telemetryTxTask, (signed char *)"telemTxTask", STACK_SIZE_BYTES, NULL, TASK_PRIORITY, &(data->telemetryTxTaskHandle));
		xTaskCreate(loggerRxTask, (signed char *)"loggerRxTask", STACK_SIZE_BYTES, NULL, TASK_PRIORITY, &(data->loggerRxTaskHandle));
		xTaskCreate(loggerTxTask, (signed char *)"loggerTxTask", STACK_SIZE_BYTES, NULL, TASK_PRIORITY, &(data->loggerTxTaskHandle));
#if defined(PIOS_INCLUDE_MPU6050)
		xTaskCreate(loggerMpuTask, (signed char *)"loggerMpuTask", STACK_SIZE_BYTES, NULL, TASK_PRIORITY, &(data->loggerMpuTaskHandle));
#endif

		// Register the watchdog timers.
#ifdef PIOS_INCLUDE_WDG
		PIOS_WDG_RegisterFlag(PIOS_WDG_TELEMETRY);
		//PIOS_WDG_RegisterFlag(PIOS_WDG_RADIORX);
		//PIOS_WDG_RegisterFlag(PIOS_WDG_RADIOTX);
#endif
	return 0;
	}

	return -1;
}

/**
 * Initialise the module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
static int32_t LoggerComBridgeInitialize(void)
{

	// allocate and initialize the static data storage only if module is enabled
	data = (LoggerComBridgeData *)pvPortMalloc(sizeof(LoggerComBridgeData));
	if (!data)
		return -1;

	// Initialize the UAVObjects that we use
	GCSReceiverInitialize();
	OPLogStatusInitialize();
	ObjectPersistenceInitialize();

	// Initialise UAVTalk
	data->outUAVTalkCon = UAVTalkInitialize(&UAVTalkSendHandler);
	data->inUAVTalkCon = UAVTalkInitialize(&LoggerSendHandler);

	// Initialize the queues.
	data->uavtalkEventQueue = xQueueCreate(EVENT_QUEUE_SIZE, sizeof(UAVObjEvent));

	// Configure our UAVObjects for updates.
	UAVObjConnectQueue(UAVObjGetByID(OPLOGSTATUS_OBJID), data->uavtalkEventQueue, EV_UPDATED | EV_UPDATED_MANUAL | EV_UPDATE_REQ);
	UAVObjConnectQueue(UAVObjGetByID(OBJECTPERSISTENCE_OBJID), data->uavtalkEventQueue, EV_UPDATED | EV_UPDATED_MANUAL);

	// Initialize the statistics.
	data->comTxErrors = 0;
	data->comTxRetries = 0;
	data->UAVTalkErrors = 0;
	
		/* Delete the file if it exists - ignore errors */
	DFS_UnlinkFile(&PIOS_SDCARD_VolInfo, (uint8_t *) LOG_NAME, PIOS_SDCARD_Sector);
	if (DFS_OpenFile(&PIOS_SDCARD_VolInfo, (uint8_t *) LOG_NAME, DFS_WRITE, PIOS_SDCARD_Sector, &File)) {
		/* Error opening file */
		return -2;
	}
	
	sprintf(Buffer, "PiOS Log\r\n\r\nLog file creation completed.\r\n");
	if (DFS_WriteFile(&File, PIOS_SDCARD_Sector, (uint8_t *) Buffer, &Cache, strlen(Buffer))) {
		/* Error writing to file */
		return -3;
	}

	return 0;
}
MODULE_INITCALL(LoggerComBridgeInitialize, LoggerComBridgeStart)

/**
 * Telemetry transmit task, regular priority
 */
static void telemetryTxTask(void *parameters)
{
	UAVObjEvent ev;

	// Loop forever
	while (1) {
#ifdef PIOS_INCLUDE_WDG
		PIOS_WDG_UpdateFlag(PIOS_WDG_TELEMETRY);
#endif
		// Wait for queue message
		if (xQueueReceive(data->uavtalkEventQueue, &ev, MAX_PORT_DELAY) == pdTRUE) {
			if ((ev.event == EV_UPDATED) || (ev.event == EV_UPDATE_REQ))
			{
				// Send update (with retries)
				uint32_t retries = 0;
				int32_t success = -1;
				while (retries < MAX_RETRIES && success == -1) {
					success = UAVTalkSendObject(data->outUAVTalkCon, ev.obj, 0, 0, RETRY_TIMEOUT_MS) == 0;
					if (!success)
						++retries;
				}
				data->comTxRetries += retries;
			}
			else if(ev.event == EV_SEND_ACK)
			{
				// Send the ACK
				uint32_t retries = 0;
				int32_t success = -1;
				while (retries < MAX_RETRIES && success == -1) {
					success = UAVTalkSendAck(data->outUAVTalkCon, ev.obj, ev.instId) == 0;
					if (!success)
						++retries;
				}
				data->comTxRetries += retries;
			}
			else if(ev.event == EV_SEND_NACK)
			{
				// Send the NACK
				uint32_t retries = 0;
				int32_t success = -1;
				while (retries < MAX_RETRIES && success == -1) {
					success = UAVTalkSendNack(data->outUAVTalkCon, UAVObjGetID(ev.obj)) == 0;
					if (!success)
						++retries;
				}
				data->comTxRetries += retries;
			}
		}
	}
}

/**
 * Logger rx task.  Receive data packets from the logger and pass them on.
 */
static void loggerRxTask(void *parameters)
{
	// Task loop
	while (1) {
#ifdef PIOS_INCLUDE_WDG
		//PIOS_WDG_UpdateFlag(PIOS_WDG_RADIORX);
#endif
		uint8_t serial_data[1];
		uint16_t bytes_to_process = PIOS_COM_ReceiveBuffer(PIOS_COM_TELEM_UART_TELEM, serial_data, sizeof(serial_data), MAX_PORT_DELAY);
		if (bytes_to_process > 0)
			for (uint8_t i = 0; i < bytes_to_process; i++)
			sprintf((char *)Buffer,"R:%u\r\n",serial_data[i]);
			DFS_WriteFile(&File, PIOS_SDCARD_Sector, (uint8_t *) Buffer, &Cache, strlen(Buffer));
	}
}

/**
 * Logger rx task.  Receive data from a com port and pass it on to the logger.
 */
static void loggerTxTask(void *parameters)
{
	// Task loop
	while (1) {
		uint32_t inputPort = PIOS_COM_TELEMETRY;
#ifdef PIOS_INCLUDE_WDG
		//PIOS_WDG_UpdateFlag(PIOS_WDG_RADIOTX);
#endif
#if defined(PIOS_INCLUDE_USB)
		// Determine output port (USB takes priority over telemetry port)
		if (PIOS_USB_CheckAvailable(0) && PIOS_COM_TELEM_USB)
			inputPort = PIOS_COM_TELEM_USB;
#endif /* PIOS_INCLUDE_USB */
		if(inputPort)
		{
			uint8_t serial_data[1];
			uint16_t bytes_to_process = PIOS_COM_ReceiveBuffer(inputPort, serial_data, sizeof(serial_data), MAX_PORT_DELAY);
			if (bytes_to_process > 0)
				for (uint8_t i = 0; i < bytes_to_process; i++)
					ProcessInputStream(data->inUAVTalkCon, serial_data[i]);
		}
	}
}

/**
 * Logger rx task.  Receive data packets from the logger and pass them on.
 */
static void loggerMpuTask(void *parameters)
{
	
#if defined(PIOS_INCLUDE_MPU6050)
	PIOS_MPU6050_Init(pios_i2c_flexi_adapter_id, &pios_mpu6050_cfg);
#endif
	
	// Task loop
	while (1) {
		updateSensors();
		printFloat(accels[0], 2);
		sprintf((char *)Buffer,"Ax:%c%d.%d\r\n",sensPart,(int)(intPart),(int)(fractPart));
		DFS_WriteFile(&File, PIOS_SDCARD_Sector, (uint8_t *) Buffer, &Cache, strlen(Buffer));
		printFloat(accels[1], 2);
		sprintf((char *)Buffer,"Ay:%c%d.%d\r\n",sensPart,(int)(intPart),(int)(fractPart));
		DFS_WriteFile(&File, PIOS_SDCARD_Sector, (uint8_t *) Buffer, &Cache, strlen(Buffer));
		printFloat(accels[2], 2);
		sprintf((char *)Buffer,"Az:%c%d.%d\r\n",sensPart,(int)(intPart),(int)(fractPart));
		DFS_WriteFile(&File, PIOS_SDCARD_Sector, (uint8_t *) Buffer, &Cache, strlen(Buffer));

	}
}

/**
 * Transmit data buffer to the com port.
 * \param[in] buf Data buffer to send
 * \param[in] length Length of buffer
 * \return -1 on failure
 * \return number of bytes transmitted on success
 */
static int32_t UAVTalkSendHandler(uint8_t *buf, int32_t length)
{
	uint32_t outputPort = PIOS_COM_TELEMETRY;
#if defined(PIOS_INCLUDE_USB)
	// Determine output port (USB takes priority over telemetry port)
	if (PIOS_COM_TELEM_USB && PIOS_COM_Available(PIOS_COM_TELEM_USB))
		outputPort = PIOS_COM_TELEM_USB;
#endif /* PIOS_INCLUDE_USB */
	if(outputPort)
		return PIOS_COM_SendBufferNonBlocking(outputPort, buf, length);
	else
		return -1;
}

/**
 * Transmit data buffer to the com port.
 * \param[in] buf Data buffer to send
 * \param[in] length Length of buffer
 * \return -1 on failure
 * \return number of bytes transmitted on success
 */
static int32_t LoggerSendHandler(uint8_t *buf, int32_t length)
{
	uint32_t outputPort = PIOS_COM_RADIO;
	// Don't send any data unless the logger port is available.
	if(outputPort && PIOS_COM_Available(outputPort))
		return PIOS_COM_SendBuffer(outputPort, buf, length);
	else
		// For some reason, if this function returns failure, it prevents saving settings.
		return length;
}

static void ProcessInputStream(UAVTalkConnection connectionHandle, uint8_t rxbyte)
{
	// Keep reading until we receive a completed packet.
	UAVTalkRxState state = UAVTalkRelayInputStream(connectionHandle, rxbyte);
	UAVTalkConnectionData *connection = (UAVTalkConnectionData*)(connectionHandle);
	UAVTalkInputProcessor *iproc = &(connection->iproc);

	if (state == UAVTALK_STATE_COMPLETE)
 	{
		// Is this a local UAVObject?
		// We only generate GcsReceiver ojects, we don't consume them.
		if ((iproc->obj != NULL) && (iproc->objId != GCSRECEIVER_OBJID))
		{
			// We treat the ObjectPersistence object differently
			if(iproc->objId == OBJECTPERSISTENCE_OBJID)
			{
				// Unpack object, if the instance does not exist it will be created!
				UAVObjUnpack(iproc->obj, iproc->instId, connection->rxBuffer);

				// Get the ObjectPersistence object.
				ObjectPersistenceData obj_per;
				ObjectPersistenceGet(&obj_per);

				// Is this concerning or setting object?
				if (obj_per.ObjectID == OPLOGSETTINGS_OBJID)
				{
					// Queue up the ACK.
					queueEvent(data->uavtalkEventQueue, (void*)iproc->obj, iproc->instId, EV_SEND_ACK);

					// Is this a save, load, or delete?
					bool success = true;
					switch (obj_per.Operation)
					{
					case OBJECTPERSISTENCE_OPERATION_LOAD:
					{
#if defined(PIOS_INCLUDE_FLASH_EEPROM)
						// Load the settings.
						OPLogSettingsData oplogSettings;
						if (PIOS_EEPROM_Load((uint8_t*)&oplogSettings, sizeof(OPLogSettingsData)) == 0)
							OPLogSettingsSet(&oplogSettings);
						else
							success = false;
#endif
						break;
					}
					case OBJECTPERSISTENCE_OPERATION_SAVE:
					{
#if defined(PIOS_INCLUDE_FLASH_EEPROM)
						// Save the settings.
						OPLogSettingsData oplogSettings;
						OPLogSettingsGet(&oplogSettings);
						int32_t ret = PIOS_EEPROM_Save((uint8_t*)&oplogSettings, sizeof(OPLogSettingsData));
						if (ret != 0)
							success = false;
#endif
						break;
					}
					case OBJECTPERSISTENCE_OPERATION_DELETE:
					{
#if defined(PIOS_INCLUDE_FLASH_EEPROM)
						// Erase the settings.
						OPLogSettingsData oplogSettings;
						uint8_t *ptr = (uint8_t*)&oplogSettings;
						memset(ptr, 0, sizeof(OPLogSettingsData));
						int32_t ret = PIOS_EEPROM_Save(ptr, sizeof(OPLogSettingsData));
						if (ret != 0)
							success = false;
#endif
						break;
					}
					default:
						break;
					}
					if (success == true)
					{
						obj_per.Operation = OBJECTPERSISTENCE_OPERATION_COMPLETED;
						ObjectPersistenceSet(&obj_per);
					}
				}
			}
			else
			{
				switch (iproc->type)
				{
				case UAVTALK_TYPE_OBJ:
					// Unpack object, if the instance does not exist it will be created!
					UAVObjUnpack(iproc->obj, iproc->instId, connection->rxBuffer);
					break;
				case UAVTALK_TYPE_OBJ_REQ:
					// Queue up an object send request.
					queueEvent(data->uavtalkEventQueue, (void*)iproc->obj, iproc->instId, EV_UPDATE_REQ);
					break;
				case UAVTALK_TYPE_OBJ_ACK:
					if (UAVObjUnpack(iproc->obj, iproc->instId, connection->rxBuffer) == 0)
						// Queue up an ACK
						queueEvent(data->uavtalkEventQueue, (void*)iproc->obj, iproc->instId, EV_SEND_ACK);
					break;
				}
			}
		}

	} else if(state == UAVTALK_STATE_ERROR) {
		data->UAVTalkErrors++;

		// Send a NACK if required.
		if((iproc->obj) && (iproc->type == UAVTALK_TYPE_OBJ_ACK))
			// Queue up a NACK
			queueEvent(data->uavtalkEventQueue, iproc->obj, iproc->instId, EV_SEND_NACK);
 	}
}

/**
 * Queue and event into an event queue.
 * \param[in] queue  The event queue
 * \param[in] obj  The data pointer
 * \param[in] type The event type
 */
static void queueEvent(xQueueHandle queue, void *obj, uint16_t instId, UAVObjEventType type)
{
	UAVObjEvent ev;
	ev.obj = (UAVObjHandle)obj;
	ev.instId = instId;
	ev.event = type;
	xQueueSend(queue, &ev, portMAX_DELAY);
}

/**
 * Get an update from the sensors
 * @param[in] attitudeRaw Populate the UAVO instead of saving right here
 * @return 0 if successfull, -1 if not
 */
 #if defined(PIOS_INCLUDE_MPU6050)
struct pios_mpu6050_data mpu6050_data;
#endif

static int32_t updateSensors(void)
{
	
#if defined(PIOS_INCLUDE_MPU6050)
	
	xQueueHandle queue = PIOS_MPU6050_GetQueue();
	
	if(xQueueReceive(queue, (void *) &mpu6050_data, SENSOR_PERIOD) == errQUEUE_EMPTY)
		return -1;	// Error, no data


	gyros[0] = mpu6050_data.gyro_x * PIOS_MPU6050_GetScale();
	gyros[1] = mpu6050_data.gyro_y * PIOS_MPU6050_GetScale();
	gyros[2] = mpu6050_data.gyro_z * PIOS_MPU6050_GetScale();
	
	accels[0] = mpu6050_data.accel_x * PIOS_MPU6050_GetAccelScale();
	accels[1] = mpu6050_data.accel_y * PIOS_MPU6050_GetAccelScale();
	accels[2] = mpu6050_data.accel_z * PIOS_MPU6050_GetAccelScale();

	temperature = 35.0f + ((float) mpu6050_data.temperature + 512.0f) / 340.0f;
#endif

	return 0;
}

/**
 * Configure the output port based on a configuration event from the remote coordinator.
 * \param[in] com_port  The com port to configure
 * \param[in] com_speed  The com port speed
 */
/*static void configureComCallback(OPLogSettingsOutputConnectionOptions com_port, OPLogSettingsComSpeedOptions com_speed)
{

	// Get the settings.
	OPLogSettingsData oplogSettings;
	OPLogSettingsGet(&oplogSettings);

	// Set the output telemetry port and speed.
	switch (com_port)
	{
	case OPLOGSETTINGS_OUTPUTCONNECTION_REMOTEHID:
		oplogSettings.InputConnection = OPLOGSETTINGS_INPUTCONNECTION_HID;
		break;
	case OPLOGSETTINGS_OUTPUTCONNECTION_REMOTEVCP:
		oplogSettings.InputConnection = OPLOGSETTINGS_INPUTCONNECTION_VCP;
		break;
	case OPLOGSETTINGS_OUTPUTCONNECTION_REMOTETELEMETRY:
		oplogSettings.InputConnection = OPLOGSETTINGS_INPUTCONNECTION_TELEMETRY;
		break;
	case OPLOGSETTINGS_OUTPUTCONNECTION_REMOTEFLEXI:
		oplogSettings.InputConnection = OPLOGSETTINGS_INPUTCONNECTION_FLEXI;
		break;
	case OPLOGSETTINGS_OUTPUTCONNECTION_TELEMETRY:
		oplogSettings.InputConnection = OPLOGSETTINGS_INPUTCONNECTION_HID;
		break;
	case OPLOGSETTINGS_OUTPUTCONNECTION_FLEXI:
		oplogSettings.InputConnection = OPLOGSETTINGS_INPUTCONNECTION_HID;
		break;
	}
	oplogSettings.ComSpeed = com_speed;

	// A non-coordinator modem should not set a remote telemetry connection.
	oplogSettings.OutputConnection = OPLOGSETTINGS_OUTPUTCONNECTION_REMOTEHID;

	// Update the OPLogSettings object.
	OPLogSettingsSet(&oplogSettings);

	// Perform the update.
	updateSettings();
}
*/
/**
 * Update the oplog settings, called on startup.
 */
static void updateSettings()
{

	bool is_coordinator=false;
	// Get the settings.
	OPLogSettingsData oplogSettings;
	OPLogSettingsGet(&oplogSettings);

	// Determine what com ports we're using.
	switch (oplogSettings.InputConnection)
	{
	case OPLOGSETTINGS_INPUTCONNECTION_VCP:
		PIOS_COM_TELEMETRY = PIOS_COM_TELEM_VCP;
		break;
	case OPLOGSETTINGS_INPUTCONNECTION_TELEMETRY:
		PIOS_COM_TELEMETRY = PIOS_COM_TELEM_UART_TELEM;
		break;
	case OPLOGSETTINGS_INPUTCONNECTION_FLEXI:
		PIOS_COM_TELEMETRY = PIOS_COM_TELEM_UART_FLEXI;
		break;
	default:
		PIOS_COM_TELEMETRY = 0;
		break;
	}

	switch (oplogSettings.OutputConnection)
	{
	case OPLOGSETTINGS_OUTPUTCONNECTION_FLEXI:
		PIOS_COM_RADIO = PIOS_COM_TELEM_UART_FLEXI;
		break;
	case OPLOGSETTINGS_OUTPUTCONNECTION_TELEMETRY:
		PIOS_COM_RADIO = PIOS_COM_TELEM_UART_TELEM;
		break;
	default:
		PIOS_COM_RADIO = 0;
		break;
	}

	// Configure the com port speeds.
	switch (oplogSettings.ComSpeed) {
	case OPLOGSETTINGS_COMSPEED_2400:
		if (is_coordinator && PIOS_COM_RADIO)  PIOS_COM_ChangeBaud(PIOS_COM_RADIO, 2400);
		if (PIOS_COM_TELEMETRY)  PIOS_COM_ChangeBaud(PIOS_COM_TELEMETRY, 2400);
		break;
	case OPLOGSETTINGS_COMSPEED_4800:
		if (is_coordinator && PIOS_COM_RADIO)  PIOS_COM_ChangeBaud(PIOS_COM_RADIO, 4800);
		if (PIOS_COM_TELEMETRY)  PIOS_COM_ChangeBaud(PIOS_COM_TELEMETRY, 4800);
		break;
	case OPLOGSETTINGS_COMSPEED_9600:
		if (is_coordinator && PIOS_COM_RADIO)  PIOS_COM_ChangeBaud(PIOS_COM_RADIO, 9600);
		if (PIOS_COM_TELEMETRY)  PIOS_COM_ChangeBaud(PIOS_COM_TELEMETRY, 9600);
		break;
	case OPLOGSETTINGS_COMSPEED_19200:
		if (is_coordinator && PIOS_COM_RADIO)  PIOS_COM_ChangeBaud(PIOS_COM_RADIO, 19200);
		if (PIOS_COM_TELEMETRY)  PIOS_COM_ChangeBaud(PIOS_COM_TELEMETRY, 19200);
		break;
	case OPLOGSETTINGS_COMSPEED_38400:
		if (is_coordinator && PIOS_COM_RADIO)  PIOS_COM_ChangeBaud(PIOS_COM_RADIO, 38400);
		if (PIOS_COM_TELEMETRY)  PIOS_COM_ChangeBaud(PIOS_COM_TELEMETRY, 38400);
		break;
	case OPLOGSETTINGS_COMSPEED_57600:
		if (is_coordinator && PIOS_COM_RADIO)  PIOS_COM_ChangeBaud(PIOS_COM_RADIO, 57600);
		if (PIOS_COM_TELEMETRY)  PIOS_COM_ChangeBaud(PIOS_COM_TELEMETRY, 57600);
		break;
	case OPLOGSETTINGS_COMSPEED_115200:
		if (is_coordinator && PIOS_COM_RADIO)  PIOS_COM_ChangeBaud(PIOS_COM_RADIO, 115200);
		if (PIOS_COM_TELEMETRY)  PIOS_COM_ChangeBaud(PIOS_COM_TELEMETRY, 115200);
		break;
	}
}

void printFloat(float v, uint8_t decimalDigits)
{
	uint8_t i = 1;
	for (;decimalDigits!=0; i*=10, decimalDigits--);
	intPart = (int16_t)v;
	fractPart = (int16_t)((v-(float)(int16_t)v)*i);
	if(intPart<0 || fractPart<0)
	{
		intPart=-intPart;
		fractPart=-fractPart;
		sensPart='-';
	}
	else
	{
		sensPart='+';
	}
}
