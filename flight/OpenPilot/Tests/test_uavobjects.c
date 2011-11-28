/**
 ******************************************************************************
 *
 * @file       test_uavobjects.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Tests for the UAVObject libraries
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
#include "exampleobject1.h"
#include "uavobjectsinit.h"

// Local functions
static void testTask(void *pvParameters);
static void eventCallback(UAVObjEvent* ev);
static void eventCallbackPeriodic(UAVObjEvent* ev);

// Variables
int testDelay = 0;

int main()
{
	/* Brings up System using CMSIS functions, enables the LEDs. */
	PIOS_SYS_Init();

	/* Delay system */
	PIOS_DELAY_Init();

	/* SPI Init */
	PIOS_SPI_Init();

	/* Enables the SDCard */
	PIOS_SDCARD_Init();
	PIOS_SDCARD_MountFS(0);

	// Turn on both LEDs
	PIOS_LED_On(LED1);
	PIOS_LED_On(LED2);

	// Create test task
	xTaskCreate(testTask, (signed portCHAR *)"ObjTest", 1000 , NULL, 1, NULL);

	// Start the FreeRTOS scheduler
	vTaskStartScheduler();
	return 0;
}

static void testTask(void *pvParameters)
{
	ExampleObject1Data data;

	// Initialize object and manager
	EventDispatcherInitialize();
	UAVObjInitialize();
	UAVObjectsInitializeAll();

	// Set data
	ExampleObject1Get(&data);
	data.Field1 = 1;
	data.Field2 = 2;
	ExampleObject1Set(&data);

	// Get data
	memset(&data, 0, sizeof(data));
	ExampleObject1Get(&data);

	// Benchmark time is takes to get and set data (both operations)
	int tickStart = xTaskGetTickCount();
	for (int n = 0; n < 10000; ++n)
	{
		ExampleObject1Set(&data);
		ExampleObject1Get(&data);
	}
	int delay = xTaskGetTickCount() - tickStart;

	// Create a new instance and get/set its data
	uint16_t instId = ExampleObject1CreateInstance();
	ExampleObject1InstSet(instId, &data);
	memset(&data, 0, sizeof(data));
	ExampleObject1InstGet(instId, &data);

	// Test pack/unpack
	uint8_t buffer[EXAMPLEOBJECT1_NUMBYTES];
	memset(buffer, 0, EXAMPLEOBJECT1_NUMBYTES);
	UAVObjPack(ExampleObject1Handle(), 0, buffer);
	memset(&data, 0, sizeof(data));
	ExampleObject1Set(&data);
	UAVObjUnpack(ExampleObject1Handle(), 0, buffer,EV_UNPACKED);
	ExampleObject1Get(&data);

	// Test object saving/loading to SD card
	UAVObjSave(ExampleObject1Handle(), 0);
	memset(&data, 0, sizeof(data));
	ExampleObject1Set(&data);
	UAVObjLoad(ExampleObject1Handle(), 0);
	ExampleObject1Get(&data);

	// Retrieve object handle by ID
	UAVObjHandle handle = UAVObjGetByID(EXAMPLEOBJECT1_OBJID);
	const char* name = UAVObjGetName(handle);

	// Get/Set the metadata
	UAVObjMetadata mdata;
	ExampleObject1GetMetadata(&mdata);
	mdata.gcsTelemetryAcked = 0;
	ExampleObject1SetMetadata(&mdata);
	memset(&mdata, 0, sizeof(mdata));
	ExampleObject1GetMetadata(&mdata);

	// Test callbacks
	ExampleObject1ConnectCallback(eventCallback);
	ExampleObject1Set(&data);

	// Test queue events
	xQueueHandle queue;
	queue = xQueueCreate(10, sizeof(UAVObjEvent));
	ExampleObject1ConnectQueue(queue);

	// Test periodic events
	UAVObjEvent ev;
	ev.event = 0;
	ev.instId = 0;
	ev.obj = ExampleObject1Handle();
	EventPeriodicCallbackCreate(&ev, eventCallbackPeriodic, 500);
	EventPeriodicQueueCreate(&ev, queue, 250);

	// Done testing
	while (1)
	{
		if(xQueueReceive(queue, &ev, portMAX_DELAY) == pdTRUE)
		{
			name = UAVObjGetName(ev.obj);
			PIOS_LED_Toggle(LED2);
		}
	}

}

static void eventCallback(UAVObjEvent* ev)
{
	const char* name = UAVObjGetName(ev->obj);
}

static void eventCallbackPeriodic(UAVObjEvent* ev)
{
	static int lastUpdate;
	testDelay = xTaskGetTickCount() - lastUpdate;
	lastUpdate = xTaskGetTickCount();
	const char* name = UAVObjGetName(ev->obj);
	PIOS_LED_Toggle(LED1);
	//ExampleObject1Updated();
}

void vApplicationIdleHook(void)
{
	/* Called when the scheduler has no tasks to run */
}
