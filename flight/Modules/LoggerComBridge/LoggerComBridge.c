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
#define TASK_PRIORITY (tskIDLE_PRIORITY)
#define MAX_RETRIES 2
#define RETRY_TIMEOUT_MS 20
#define EVENT_QUEUE_SIZE 10
#define MAX_PORT_DELAY 200
#define EV_SEND_ACK 0x20
#define EV_SEND_NACK 0x30

#define MAX_BUFFER_LENGTH 512

#define SENSOR_PERIOD 4
#define LOG_NAME "mpu6050.log"
#define LOG_SERIAL_NAME "serial.log"
#define LOG_ADC_NAME "adc.log"

// ****************
// Private types

typedef struct {

	// The task handles.
	xTaskHandle telemetryTxTaskHandle;
	xTaskHandle loggerRx1TaskHandle;
	xTaskHandle loggerRx2TaskHandle;
	xTaskHandle loggerRx3TaskHandle;
	xTaskHandle loggerTxTaskHandle;
	xTaskHandle loggerMpuTaskHandle;
	xTaskHandle loggerWriteSDTaskHandle;
	xTaskHandle loggerAdcTaskHandle;

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
	
	char Log_Buffer_1[MAX_BUFFER_LENGTH];
	char Log_Buffer_2[MAX_BUFFER_LENGTH];
	
	uint16_t Pos_Buffer_1;
	uint16_t Pos_Buffer_2;
	uint8_t Current_buffer;
	
	uint8_t Ready2Record_1;
	uint8_t Ready2Record_2;
	
	uint8_t Buffers_InUse;
	xSemaphoreHandle buffer_lock;
	

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

bool NewMpu6050BufferReady;
bool NewSerialBufferReady;

// ****************
// Private functions

static void telemetryTxTask(void *parameters);
static void loggerRx1Task(void *parameters);
static void loggerRx2Task(void *parameters);
static void loggerRx3Task(void *parameters);
static void loggerTxTask(void *parameters);
static void loggerMpuTask(void *parameters);
static void loggerWriteSDTask(void *parameters);
static void loggerAdcTask(void *parameters);
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

FILEINFO Mpu6050_File;
char Mpu6050_Buffer[512];
uint32_t Cache;
bool LogAscii;

FILEINFO Serial_File;
char Serial_Buffer[512];
uint32_t Serial_Cache;
uint16_t PosInSerialBuf;

FILEINFO Adc_File;
char Adc_Buffer[512];
uint32_t Adc_Cache;
uint16_t PosInAdcBuf;

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
		xTaskCreate(loggerRx1Task, (signed char *)"loggerRx1Task", STACK_SIZE_BYTES, NULL, TASK_PRIORITY, &(data->loggerRx1TaskHandle));
		xTaskCreate(loggerRx2Task, (signed char *)"loggerRx2Task", STACK_SIZE_BYTES, NULL, TASK_PRIORITY, &(data->loggerRx2TaskHandle));
		//xTaskCreate(loggerRx3Task, (signed char *)"loggerRx3Task", STACK_SIZE_BYTES, NULL, TASK_PRIORITY, &(data->loggerRx3TaskHandle));
		//xTaskCreate(loggerTxTask, (signed char *)"loggerTxTask", STACK_SIZE_BYTES, NULL, TASK_PRIORITY, &(data->loggerTxTaskHandle));
#if defined(PIOS_INCLUDE_MPU6050)
		xTaskCreate(loggerMpuTask, (signed char *)"loggerMpuTask", STACK_SIZE_BYTES, NULL, TASK_PRIORITY, &(data->loggerMpuTaskHandle));
#endif

#if defined(PIOS_INCLUDE_ADC)
		xTaskCreate(loggerAdcTask, (signed char *)"loggerAdcTask", STACK_SIZE_BYTES, NULL, TASK_PRIORITY, &(data->loggerAdcTaskHandle));
#endif

		xTaskCreate(loggerWriteSDTask, (signed char *)"loggerWriteSDTask", STACK_SIZE_BYTES, NULL, TASK_PRIORITY, &(data->loggerWriteSDTaskHandle));

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
	
		// Delete the file if it exists - ignore errors 
	DFS_UnlinkFile(&PIOS_SDCARD_VolInfo, (uint8_t *) LOG_NAME, PIOS_SDCARD_Sector);
	if (DFS_OpenFile(&PIOS_SDCARD_VolInfo, (uint8_t *) LOG_NAME, DFS_WRITE, PIOS_SDCARD_Sector, &Mpu6050_File)) {
		// Error opening file 
		//return -1;
	}
	
	sprintf(Mpu6050_Buffer, "PiOS Log\r\n\r\nLog file creation completed.\r\n");
	if (DFS_WriteFile(&Mpu6050_File, PIOS_SDCARD_Sector, (uint8_t *) Mpu6050_Buffer, &Cache, strlen(Mpu6050_Buffer))) {
		// Error writing to file 
		//return -1;
	}
	
	sprintf(Mpu6050_Buffer, "------------------------------\r\nMpu6050 i2C Log\r\n------------------------------\r\n");
	if (DFS_WriteFile(&Mpu6050_File, PIOS_SDCARD_Sector, (uint8_t *) Mpu6050_Buffer, &Cache, strlen(Mpu6050_Buffer))) {
		/* Error writing to file */
		//return -2;
	}
	
	DFS_UnlinkFile(&PIOS_SDCARD_VolInfo, (uint8_t *) LOG_SERIAL_NAME, PIOS_SDCARD_Sector);
	if (DFS_OpenFile(&PIOS_SDCARD_VolInfo, (uint8_t *) LOG_SERIAL_NAME, DFS_WRITE, PIOS_SDCARD_Sector, &Serial_File)) {
		// Error opening file 
		//return -1;
	}
	
	sprintf(Serial_Buffer, "PiOS Log\r\n\r\nLog file creation completed.\r\n");
	if (DFS_WriteFile(&Serial_File, PIOS_SDCARD_Sector, (uint8_t *) Serial_Buffer, &Serial_Cache, strlen(Serial_Buffer))) {
		// Error writing to file 
		//return -1;
	}
	
	sprintf(Serial_Buffer, "------------------------------\r\nSerial Log\r\n------------------------------\r\n");
	if (DFS_WriteFile(&Serial_File, PIOS_SDCARD_Sector, (uint8_t *) Serial_Buffer, &Serial_Cache, strlen(Serial_Buffer))) {
		/* Error writing to file */
		//return -2;
	}
	
	DFS_UnlinkFile(&PIOS_SDCARD_VolInfo, (uint8_t *) LOG_ADC_NAME, PIOS_SDCARD_Sector);
	if (DFS_OpenFile(&PIOS_SDCARD_VolInfo, (uint8_t *) LOG_ADC_NAME, DFS_WRITE, PIOS_SDCARD_Sector, &Adc_File)) {
		// Error opening file 
		//return -1;
	}
	
	sprintf(Adc_Buffer, "PiOS Log\r\n\r\nLog file creation completed.\r\n");
	if (DFS_WriteFile(&Adc_File, PIOS_SDCARD_Sector, (uint8_t *) Adc_Buffer, &Adc_Cache, strlen(Adc_Buffer))) {
		// Error writing to file 
		//return -1;
	}
	
	sprintf(Adc_Buffer, "------------------------------\r\nAdc Log\r\n------------------------------\r\n");
	if (DFS_WriteFile(&Adc_File, PIOS_SDCARD_Sector, (uint8_t *) Adc_Buffer, &Adc_Cache, strlen(Adc_Buffer))) {
		/* Error writing to file */
		//return -2;
	}
	
	data->Pos_Buffer_1=0;
	data->Pos_Buffer_2=0;
	data->Current_buffer=1;
	data->Ready2Record_1=0;
	data->Ready2Record_2=0;
	data->Buffers_InUse=0;
	
	data->buffer_lock = xSemaphoreCreateMutex();
	if(data->buffer_lock == NULL)
		return -1;
	
	PosInSerialBuf=0;
	PosInAdcBuf=0;
	NewMpu6050BufferReady=FALSE;
	NewSerialBufferReady=FALSE;
	LogAscii=FALSE;

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
static void loggerRx1Task(void *parameters)
{
portTickType lastSysTime;
portTickType oldSysTime;
uint8_t Pos;
uint8_t FloatIEEE[4];
char Serial_FormBuffer[64];
oldSysTime = xTaskGetTickCount();
bool InSentence=FALSE;
uint8_t NbCLogged=0;
	// Task loop
	while (1) {
#ifdef PIOS_INCLUDE_WDG
		//PIOS_WDG_UpdateFlag(PIOS_WDG_RADIORX);
#endif
		//it takes 0.00015625Sec to stream the 9bits of a byte @ 57600bds, should have 6.4character/ms, 1ms seems to be the period the rx is polled
		uint8_t serial_data[1];
		uint16_t bytes_to_process = PIOS_COM_ReceiveBuffer(PIOS_COM_TELEM_UART_TELEM, serial_data, sizeof(serial_data), MAX_PORT_DELAY);
		if (bytes_to_process > 0)
		{
			PIOS_LED_Toggle(PIOS_LED_LINK);
			lastSysTime = xTaskGetTickCount();
			if(lastSysTime!=oldSysTime)
			{
				if(InSentence)//close the transaction and write it to the sd
				{
					Serial_FormBuffer[Pos]=NbCLogged;
					Pos++;
					Serial_FormBuffer[Pos]=0x0d;
					Pos++;
					Serial_FormBuffer[Pos]=0x0a;
					Pos++;
					InSentence=FALSE;
					// Get the lock for manipulating the buffer
					xSemaphoreTake(data->buffer_lock, portMAX_DELAY);
					if(data->Current_buffer==1)
					{
						if(data->Pos_Buffer_1+Pos>=MAX_BUFFER_LENGTH)
						{
							data->Current_buffer=2;
							data->Ready2Record_1=1;
							memcpy((char*)data->Log_Buffer_2,(char*)Serial_FormBuffer, Pos);
							//copybuf((char*)&data->Log_Buffer_2,(char*)&Serial_FormBuffer,0,(uint16_t)Pos);
							data->Pos_Buffer_2=Pos;
						}
						else
						{
							memcpy((char*)data->Log_Buffer_1+data->Pos_Buffer_1,(char*)Serial_FormBuffer, Pos);
							//copybuf((char*)&data->Log_Buffer_1,(char*)&Serial_FormBuffer,(uint16_t)data->Pos_Buffer_1,(uint16_t)Pos);
							data->Pos_Buffer_1+=Pos;
						}
					}
					else
					{
						if(data->Pos_Buffer_2+Pos>=MAX_BUFFER_LENGTH)
						{
							data->Current_buffer=1;
							data->Ready2Record_2=1;
							memcpy((char*)data->Log_Buffer_1,(char*)Serial_FormBuffer, Pos);
							//copybuf((char*)&data->Log_Buffer_1,(char*)&Serial_FormBuffer,0,(uint16_t)Pos);
							data->Pos_Buffer_1=Pos;
						}
						else
						{
							memcpy((char*)data->Log_Buffer_2+data->Pos_Buffer_2,(char*)Serial_FormBuffer, Pos);
							//copybuf((char*)&data->Log_Buffer_2,(char*)&Serial_FormBuffer,(uint16_t)data->Pos_Buffer_2,(uint16_t)Pos);
							data->Pos_Buffer_2+=Pos;
						}
					}
					xSemaphoreGive(data->buffer_lock);
					//data->Buffers_InUse=0;
				}
				
				{
					NbCLogged=0;
					Pos=0;
					Serial_FormBuffer[Pos]='R';
					Pos++;
					Serial_FormBuffer[Pos]='1';
					Pos++;
					memcpy(&FloatIEEE,&lastSysTime, sizeof(lastSysTime));//turn the float value to an array of 4bytes
					memcpy((char*)Serial_FormBuffer,(char*)FloatIEEE+Pos, sizeof(FloatIEEE));
					//copybuf((char*)&Serial_FormBuffer,(char*)&FloatIEEE,(uint16_t)Pos,(uint16_t)sizeof(FloatIEEE));//copy the 4 bytes at the end of the buffer
					Pos+=sizeof(FloatIEEE);
					oldSysTime=lastSysTime;
					InSentence=TRUE;
					for (uint8_t i = 0; i < bytes_to_process; i++)
					{
						Serial_FormBuffer[Pos]=serial_data[i];
						Pos++;
						NbCLogged++;
					}
				}
			}
			else
			{
				for (uint8_t i = 0; i < bytes_to_process; i++)
				{
					Serial_FormBuffer[Pos]=serial_data[i];
					Pos++;
					NbCLogged++;
				}
			}
		}
	}
}

static void loggerRx2Task(void *parameters)
{
portTickType lastSysTime;
portTickType oldSysTime;
uint8_t Pos;
uint8_t FloatIEEE[4];
char Serial_FormBuffer[64];
oldSysTime = xTaskGetTickCount();
bool InSentence=FALSE;
uint8_t NbCLogged=0;
	// Task loop
	while (1) {
		//it takes 0.00015625Sec to stream the 9bits of a byte @ 57600bds, should have 6.4character/ms, 1ms seems to be the period the rx is polled
		uint8_t serial_data[1];
		uint16_t bytes_to_process = PIOS_COM_ReceiveBuffer(PIOS_COM_TELEM_UART_AUX, serial_data, sizeof(serial_data), MAX_PORT_DELAY);
		if (bytes_to_process > 0)
		{
			PIOS_LED_Toggle(PIOS_LED_LINK);
			lastSysTime = xTaskGetTickCount();
			if(lastSysTime!=oldSysTime)
			{
				if(InSentence)//close the transaction and write it to the sd
				{
					Serial_FormBuffer[Pos]=NbCLogged;
					Pos++;
					Serial_FormBuffer[Pos]=0x0d;
					Pos++;
					Serial_FormBuffer[Pos]=0x0a;
					Pos++;
					InSentence=FALSE;
					// Get the lock for manipulating the buffer
					xSemaphoreTake(data->buffer_lock, portMAX_DELAY);
					if(data->Current_buffer==1)
					{
						if(data->Pos_Buffer_1+Pos>=MAX_BUFFER_LENGTH)
						{
							data->Current_buffer=2;
							data->Ready2Record_1=1;
							memcpy((char*)data->Log_Buffer_2,(char*)Serial_FormBuffer, Pos);
							//copybuf((char*)&data->Log_Buffer_2,(char*)&Serial_FormBuffer,0,(uint16_t)Pos);
							data->Pos_Buffer_2=Pos;
						}
						else
						{
							memcpy((char*)data->Log_Buffer_1+data->Pos_Buffer_1,(char*)Serial_FormBuffer, Pos);
							//copybuf((char*)&data->Log_Buffer_1,(char*)&Serial_FormBuffer,(uint16_t)data->Pos_Buffer_1,(uint16_t)Pos);
							data->Pos_Buffer_1+=Pos;
						}
					}
					else
					{
						if(data->Pos_Buffer_2+Pos>=MAX_BUFFER_LENGTH)
						{
							data->Current_buffer=1;
							data->Ready2Record_2=1;
							memcpy((char*)data->Log_Buffer_1,(char*)Serial_FormBuffer, Pos);
							//copybuf((char*)&data->Log_Buffer_1,(char*)&Serial_FormBuffer,0,(uint16_t)Pos);
							data->Pos_Buffer_1=Pos;
						}
						else
						{
							memcpy((char*)data->Log_Buffer_2+data->Pos_Buffer_2,(char*)Serial_FormBuffer, Pos);
							//copybuf((char*)&data->Log_Buffer_2,(char*)&Serial_FormBuffer,(uint16_t)data->Pos_Buffer_2,(uint16_t)Pos);
							data->Pos_Buffer_2+=Pos;
						}
					}
					xSemaphoreGive(data->buffer_lock);
					//data->Buffers_InUse=0;
				}
				
				{
					NbCLogged=0;
					Pos=0;
					Serial_FormBuffer[Pos]='R';
					Pos++;
					Serial_FormBuffer[Pos]='2';
					Pos++;
					memcpy(&FloatIEEE,&lastSysTime, sizeof(lastSysTime));//turn the float value to an array of 4bytes
					memcpy((char*)Serial_FormBuffer+Pos,(char*)FloatIEEE, sizeof(FloatIEEE));
					//copybuf((char*)&Serial_FormBuffer,(char*)&FloatIEEE,(uint16_t)Pos,(uint16_t)sizeof(FloatIEEE));//copy the 4 bytes at the end of the buffer
					Pos+=sizeof(FloatIEEE);
					oldSysTime=lastSysTime;
					InSentence=TRUE;
					for (uint8_t i = 0; i < bytes_to_process; i++)
					{
						Serial_FormBuffer[Pos]=serial_data[i];
						Pos++;
						NbCLogged++;
					}
				}
			}
			else
			{
				for (uint8_t i = 0; i < bytes_to_process; i++)
				{
					Serial_FormBuffer[Pos]=serial_data[i];
					Pos++;
					NbCLogged++;
				}
			}
		}
	}
}

static void loggerRx3Task(void *parameters)
{
portTickType lastSysTime;
portTickType oldSysTime;
uint8_t Pos;
uint8_t FloatIEEE[4];
char Serial_FormBuffer[64];
oldSysTime = xTaskGetTickCount();
bool InSentence=FALSE;
uint8_t NbCLogged=0;
	// Task loop
	while (1) {
		//it takes 0.00015625Sec to stream the 9bits of a byte @ 57600bds, should have 6.4character/ms, 1ms seems to be the period the rx is polled
		uint8_t serial_data[1];
		uint16_t bytes_to_process = PIOS_COM_ReceiveBuffer(PIOS_COM_TELEM_UART_AUX, serial_data, sizeof(serial_data), MAX_PORT_DELAY);
		if (bytes_to_process > 0)
		{
			PIOS_LED_Toggle(PIOS_LED_LINK);
			lastSysTime = xTaskGetTickCount();
			if(lastSysTime!=oldSysTime)
			{
				if(InSentence)//close the transaction and write it to the sd
				{
					Serial_FormBuffer[Pos]=NbCLogged;
					Pos++;
					Serial_FormBuffer[Pos]=0x0d;
					Pos++;
					Serial_FormBuffer[Pos]=0x0a;
					Pos++;
					InSentence=FALSE;
					// Get the lock for manipulating the buffer
					xSemaphoreTake(data->buffer_lock, portMAX_DELAY);
					if(data->Current_buffer==1)
					{
						if(data->Pos_Buffer_1+Pos>=MAX_BUFFER_LENGTH)
						{
							data->Current_buffer=2;
							data->Ready2Record_1=1;
							memcpy((char*)data->Log_Buffer_2,(char*)Serial_FormBuffer, Pos);
							//copybuf((char*)&data->Log_Buffer_2,(char*)&Serial_FormBuffer,0,(uint16_t)Pos);
							data->Pos_Buffer_2=Pos;
						}
						else
						{
							memcpy((char*)data->Log_Buffer_1+data->Pos_Buffer_1,(char*)Serial_FormBuffer, Pos);
							//copybuf((char*)&data->Log_Buffer_1,(char*)&Serial_FormBuffer,(uint16_t)data->Pos_Buffer_1,(uint16_t)Pos);
							data->Pos_Buffer_1+=Pos;
						}
					}
					else
					{
						if(data->Pos_Buffer_2+Pos>=MAX_BUFFER_LENGTH)
						{
							data->Current_buffer=1;
							data->Ready2Record_2=1;
							memcpy((char*)data->Log_Buffer_1,(char*)Serial_FormBuffer, Pos);
							//copybuf((char*)&data->Log_Buffer_1,(char*)&Serial_FormBuffer,0,(uint16_t)Pos);
							data->Pos_Buffer_1=Pos;
						}
						else
						{
							memcpy((char*)data->Log_Buffer_2+data->Pos_Buffer_2,(char*)Serial_FormBuffer, Pos);
							//copybuf((char*)&data->Log_Buffer_2,(char*)&Serial_FormBuffer,(uint16_t)data->Pos_Buffer_2,(uint16_t)Pos);
							data->Pos_Buffer_2+=Pos;
						}
					}
					xSemaphoreGive(data->buffer_lock);
					//data->Buffers_InUse=0;
				}
				
				{
					NbCLogged=0;
					Pos=0;
					Serial_FormBuffer[Pos]='R';
					Pos++;
					Serial_FormBuffer[Pos]='3';
					Pos++;
					memcpy(&FloatIEEE,&lastSysTime, sizeof(lastSysTime));//turn the float value to an array of 4bytes
					memcpy((char*)Serial_FormBuffer+Pos,(char*)FloatIEEE, sizeof(FloatIEEE));
					//copybuf((char*)&Serial_FormBuffer,(char*)&FloatIEEE,(uint16_t)Pos,(uint16_t)sizeof(FloatIEEE));//copy the 4 bytes at the end of the buffer
					Pos+=sizeof(FloatIEEE);
					oldSysTime=lastSysTime;
					InSentence=TRUE;
					for (uint8_t i = 0; i < bytes_to_process; i++)
					{
						Serial_FormBuffer[Pos]=serial_data[i];
						Pos++;
						NbCLogged++;
					}
				}
			}
			else
			{
				for (uint8_t i = 0; i < bytes_to_process; i++)
				{
					Serial_FormBuffer[Pos]=serial_data[i];
					Pos++;
					NbCLogged++;
				}
			}
		}
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

static void loggerWriteSDTask(void *parameters)
{
	//portTickType lastSysTime;
	while (1) {
		if(data->Ready2Record_1==1)
		{
			DFS_WriteFile(&Serial_File, PIOS_SDCARD_Sector, (uint8_t *) data->Log_Buffer_1, &Serial_Cache, data->Pos_Buffer_1);
			data->Pos_Buffer_1=0;
			data->Ready2Record_1=0;
		}
		if(data->Ready2Record_2==1)
		{
			DFS_WriteFile(&Serial_File, PIOS_SDCARD_Sector, (uint8_t *) data->Log_Buffer_2, &Serial_Cache, data->Pos_Buffer_2);
			data->Pos_Buffer_2=0;
			data->Ready2Record_2=0;
		}
	}
}

void copybuf(char * bufDest,char * bufSource,uint16_t pos, uint16_t lenght)
{
	uint16_t index;
	for(index=0;index<lenght;index++)
	{
		bufDest[pos+index]=bufSource[index];
	}
}

static void loggerAdcTask(void *parameters)
{
	portTickType lastSysTime;
	uint8_t FloatIEEE[4];
	uint8_t Pos=0;
	int32_t AdcValue;
	char Adc_FormBuffer[24];
	// Task loop
	while (1) 
	{
		lastSysTime = xTaskGetTickCount();
		Pos=0;
		Adc_FormBuffer[Pos]='A';
		Pos++;
		Adc_FormBuffer[Pos]='d';
		Pos++;
		memcpy(&FloatIEEE,&lastSysTime, sizeof(lastSysTime));//turn the float value to an array of 4bytes
		memcpy((char*)Adc_FormBuffer+Pos,(char*)FloatIEEE, sizeof(FloatIEEE));
		//copybuf((char*)&Adc_FormBuffer,(char*)&FloatIEEE,(uint16_t)Pos,(uint16_t)sizeof(FloatIEEE));//copy the 4 bytes at the end of the buffer
		Pos+=sizeof(FloatIEEE);
		AdcValue=PIOS_ADC_PinGet(0);
		memcpy(&FloatIEEE,&AdcValue, sizeof(AdcValue));//turn the float value to an array of 4bytes
		memcpy((char*)Adc_FormBuffer+Pos,(char*)FloatIEEE, sizeof(FloatIEEE));
		//copybuf((char*)&Adc_FormBuffer,(char*)&FloatIEEE,(uint16_t)Pos,(uint16_t)sizeof(FloatIEEE));//copy the 4 bytes at the end of the buffer
		Pos+=sizeof(FloatIEEE);
		AdcValue=PIOS_ADC_PinGet(1);
		memcpy(&FloatIEEE,&AdcValue, sizeof(AdcValue));//turn the float value to an array of 4bytes
		memcpy((char*)Adc_FormBuffer+Pos,(char*)FloatIEEE, sizeof(FloatIEEE));
		//copybuf((char*)&Adc_FormBuffer,(char*)&FloatIEEE,(uint16_t)Pos,(uint16_t)sizeof(FloatIEEE));//copy the 4 bytes at the end of the buffer
		Pos+=sizeof(FloatIEEE);
		AdcValue=PIOS_ADC_PinGet(2);
		memcpy(&FloatIEEE,&AdcValue, sizeof(AdcValue));//turn the float value to an array of 4bytes
		memcpy((char*)Adc_FormBuffer+Pos,(char*)FloatIEEE, sizeof(FloatIEEE));
		//copybuf((char*)&Adc_FormBuffer,(char*)&FloatIEEE,(uint16_t)Pos,(uint16_t)sizeof(FloatIEEE));//copy the 4 bytes at the end of the buffer
		Pos+=sizeof(FloatIEEE);
		Adc_FormBuffer[Pos]=0x0d;
		Pos++;
		Adc_FormBuffer[Pos]=0x0a;
		Pos++;
		xSemaphoreTake(data->buffer_lock, portMAX_DELAY);
		/*while(data->Buffers_InUse)//wait for other task to finish writing on buffer
		{
			vTaskDelay(1 / portTICK_RATE_MS);
		}
		data->Buffers_InUse=1;*/
		if(data->Current_buffer==1)
		{
			if(data->Pos_Buffer_1+Pos>=MAX_BUFFER_LENGTH)
			{
				data->Current_buffer=2;
				data->Ready2Record_1=1;
				memcpy((char*)data->Log_Buffer_2,(char*)Adc_FormBuffer, Pos);
				//copybuf((char*)&data->Log_Buffer_2,(char*)&Adc_FormBuffer,0,(uint16_t)Pos);
				data->Pos_Buffer_2=Pos;
			}
			else
			{
				memcpy((char*)data->Log_Buffer_1+data->Pos_Buffer_1,(char*)Adc_FormBuffer, Pos);
				//copybuf((char*)&data->Log_Buffer_1,(char*)&Adc_FormBuffer,(uint16_t)data->Pos_Buffer_1,(uint16_t)Pos);
				data->Pos_Buffer_1+=Pos;
			}
		}
		else
		{
			if(data->Pos_Buffer_2+Pos>=MAX_BUFFER_LENGTH)
			{
				data->Current_buffer=1;
				data->Ready2Record_2=1;
				memcpy((char*)data->Log_Buffer_1,(char*)Adc_FormBuffer, Pos);
				//copybuf((char*)&data->Log_Buffer_1,(char*)&Adc_FormBuffer,0,(uint16_t)Pos);
				data->Pos_Buffer_1=Pos;
			}
			else
			{
				memcpy((char*)data->Log_Buffer_2+data->Pos_Buffer_2,(char*)Adc_FormBuffer, Pos);
				//copybuf((char*)&data->Log_Buffer_2,(char*)&Adc_FormBuffer,(uint16_t)data->Pos_Buffer_2,(uint16_t)Pos);
				data->Pos_Buffer_2+=Pos;
			}
		}
		xSemaphoreGive(data->buffer_lock);
		//data->Buffers_InUse=0;
		vTaskDelayUntil(&lastSysTime, 10 / portTICK_RATE_MS);//sleep for 10ms (scan at 100hz)
	}
}
/**
 * Logger rx task.  Receive data packets from the logger and pass them on.
 */
static void loggerMpuTask(void *parameters)
{
portTickType lastSysTime;
int16_t inpart[7];
int16_t frpart[7];
uint8_t spart[7];
char Mpu6050Form_Buff[64];
char Mpu6050Hex_Buff[16];
uint8_t Pos=0;
uint8_t FloatIEEE[4];
float AdcValue;
	
#if defined(PIOS_INCLUDE_MPU6050)
	PIOS_MPU6050_Init(pios_i2c_flexi_adapter_id, &pios_mpu6050_cfg);
#endif
	
	// Task loop
	while (1) {
		if(updateSensors()==0)
		{
			lastSysTime = xTaskGetTickCount();
			Pos=0;
			Mpu6050Form_Buff[Pos]='M';
			Pos++;
			Mpu6050Form_Buff[Pos]='p';
			Pos++;
			memcpy(&FloatIEEE,&lastSysTime, sizeof(lastSysTime));//turn the float value to an array of 4bytes
			memcpy((char*)Mpu6050Form_Buff+Pos,(char*)FloatIEEE, sizeof(FloatIEEE));
			//copybuf((char*)&Mpu6050Form_Buff,(char*)&FloatIEEE,(uint16_t)Pos,(uint16_t)sizeof(FloatIEEE));//copy the 4 bytes at the end of the buffer
			Pos+=sizeof(FloatIEEE);
			memcpy(&FloatIEEE,&accels[0], sizeof(accels[0]));
			memcpy((char*)Mpu6050Form_Buff+Pos,(char*)FloatIEEE, sizeof(FloatIEEE));
			//copybuf((char*)&Mpu6050Form_Buff,(char*)&FloatIEEE,(uint16_t)Pos,(uint16_t)sizeof(FloatIEEE));
			Pos+=sizeof(FloatIEEE);
			memcpy(&FloatIEEE,&accels[1], sizeof(accels[1]));
			memcpy((char*)Mpu6050Form_Buff+Pos,(char*)FloatIEEE, sizeof(FloatIEEE));
			//copybuf((char*)&Mpu6050Form_Buff,(char*)&FloatIEEE,(uint16_t)Pos,(uint16_t)sizeof(FloatIEEE));
			Pos+=sizeof(FloatIEEE);
			memcpy(&FloatIEEE,&accels[2], sizeof(accels[2]));
			memcpy((char*)Mpu6050Form_Buff+Pos,(char*)FloatIEEE, sizeof(FloatIEEE));
			//copybuf((char*)&Mpu6050Form_Buff,(char*)&FloatIEEE,(uint16_t)Pos,(uint16_t)sizeof(FloatIEEE));
			Pos+=sizeof(FloatIEEE);
			memcpy(&FloatIEEE,&gyros[0], sizeof(gyros[0]));
			memcpy((char*)Mpu6050Form_Buff+Pos,(char*)FloatIEEE, sizeof(FloatIEEE));
			//copybuf((char*)&Mpu6050Form_Buff,(char*)&FloatIEEE,(uint16_t)Pos,(uint16_t)sizeof(FloatIEEE));
			Pos+=sizeof(FloatIEEE);
			memcpy(&FloatIEEE,&gyros[1], sizeof(gyros[1]));
			memcpy((char*)Mpu6050Form_Buff+Pos,(char*)FloatIEEE, sizeof(FloatIEEE));
			//copybuf((char*)&Mpu6050Form_Buff,(char*)&FloatIEEE,(uint16_t)Pos,(uint16_t)sizeof(FloatIEEE));
			Pos+=sizeof(FloatIEEE);
			memcpy(&FloatIEEE,&gyros[2], sizeof(gyros[2]));
			memcpy((char*)Mpu6050Form_Buff+Pos,(char*)FloatIEEE, sizeof(FloatIEEE));
			//copybuf((char*)&Mpu6050Form_Buff,(char*)&FloatIEEE,(uint16_t)Pos,(uint16_t)sizeof(FloatIEEE));
			Pos+=sizeof(FloatIEEE);
			memcpy(&FloatIEEE,&temperature, sizeof(temperature));
			memcpy((char*)Mpu6050Form_Buff+Pos,(char*)FloatIEEE, sizeof(FloatIEEE));
			//copybuf((char*)&Mpu6050Form_Buff,(char*)&FloatIEEE,(uint16_t)Pos,(uint16_t)sizeof(FloatIEEE));
			Pos+=sizeof(FloatIEEE);
			Mpu6050Form_Buff[Pos]=0x0d;
			Pos++;
			Mpu6050Form_Buff[Pos]=0x0a;
			Pos++;
			xSemaphoreTake(data->buffer_lock, portMAX_DELAY);
			/*while(data->Buffers_InUse)//wait for other task to finish writing on buffer
			{
				vTaskDelay(1 / portTICK_RATE_MS);
			}
			data->Buffers_InUse=1;*/
			if(data->Current_buffer==1)
			{
				if(data->Pos_Buffer_1+Pos>=MAX_BUFFER_LENGTH)
				{
					data->Current_buffer=2;
					data->Ready2Record_1=1;
					memcpy((char*)data->Log_Buffer_2,(char*)Mpu6050Form_Buff, Pos);
					//copybuf((char*)&data->Log_Buffer_2,(char*)&Mpu6050Form_Buff,0,(uint16_t)Pos);
					data->Pos_Buffer_2=Pos;
				}
				else
				{
					memcpy((char*)data->Log_Buffer_1+data->Pos_Buffer_1,(char*)Mpu6050Form_Buff, Pos);
					//copybuf((char*)&data->Log_Buffer_1,(char*)&Mpu6050Form_Buff,(uint16_t)data->Pos_Buffer_1,(uint16_t)Pos);
					data->Pos_Buffer_1+=Pos;
				}
			}
			else
			{
				if(data->Pos_Buffer_2+Pos>=MAX_BUFFER_LENGTH)
				{
					data->Current_buffer=1;
					data->Ready2Record_2=1;
					memcpy((char*)data->Log_Buffer_1,(char*)Mpu6050Form_Buff, Pos);
					//copybuf((char*)&data->Log_Buffer_1,(char*)&Mpu6050Form_Buff,0,(uint16_t)Pos);
					data->Pos_Buffer_1=Pos;
				}
				else
				{
					memcpy((char*)data->Log_Buffer_2+data->Pos_Buffer_2,(char*)Mpu6050Form_Buff, Pos);
					//copybuf((char*)&data->Log_Buffer_2,(char*)&Mpu6050Form_Buff,(uint16_t)data->Pos_Buffer_2,(uint16_t)Pos);
					data->Pos_Buffer_2+=Pos;
				}
			}
			xSemaphoreGive(data->buffer_lock);
			//data->Buffers_InUse=0;
			vTaskDelayUntil(&lastSysTime, 10 / portTICK_RATE_MS);//sleep a while
		}
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
