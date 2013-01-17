/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup Odisplay OLED Display Module
 * @brief Communicate with the Seeedstudio oled display
 * @{ 
 *
 * @file       odisplay.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Adc module, handles adc readings from ETASV3
 *
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
\
/**
 * Output object: none
 *
 * This module will periodically update the Oled display.
 *
 */

#include "openpilot.h"
#include "hwsettings.h"
#include "odisplay.h"
//#include "adcs.h"	// object that will be updated by the module
#include "accels.h"
#include "gyros.h"
#include "attitudeactual.h"

// Private constants
#define STACK_SIZE_BYTES 500
#define TASK_PRIORITY (tskIDLE_PRIORITY+1)
#define UPDATE_RATE 250.0f

int32_t OdisplayInitialize();
static bool OsdInit=false;
int16_t intPart, fractPart;

// Private types

// Private variables
static xTaskHandle taskHandle;

// Private functions
static void odisplayTask(void *parameters);
void printFloat(float v, uint8_t decimalDigits);

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t OdisplayStart()
{	
	// Start main task
	xTaskCreate(odisplayTask, (signed char *)"Odisplay", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &taskHandle);
//	TaskMonitorAdd(TASKINFO_RUNNING_ODISPLAY, taskHandle);


	return 0;
}

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t OdisplayInitialize()
{
	//Initialize the UAVObjects
	//AdcsInitialize();
	AccelsInitialize();
	GyrosInitialize();
	AttitudeActualInitialize();
	return 0;
}
MODULE_INITCALL(OdisplayInitialize, OdisplayStart)
/**
 * Module thread, should not return.
 */
static void odisplayTask(void *parameters)
{
	//AdcsData data;
	AccelsData accels;
	GyrosData gyros;
	AttitudeActualData attitude;

	uint8_t Text[16]={0};
	PIOS_SSD1308_Init();//init display on aux i2c
	PIOS_SSD1308_setHorizontalMode();//set horizontal
	PIOS_SSD1308_clearDisplay();//clear display
	PIOS_SSD1308_setTextXY(0,0);
	PIOS_SSD1308_clearDisplay();//clear display
	PIOS_SSD1308_putChar('F');
	PIOS_SSD1308_putChar('e');
	PIOS_SSD1308_putChar('r');
	PIOS_SSD1308_putChar('t');
	PIOS_SSD1308_putChar('i');
	PIOS_SSD1308_putChar('t');
	PIOS_SSD1308_putChar('o');
	PIOS_SSD1308_setTextXY(0,0);
	
	while (OsdInit==0)
	{
		if((xTaskGetTickCount() > 5000)) 
		{
			OsdInit=true;
		}
		vTaskDelay(UPDATE_RATE/portTICK_RATE_MS);
	}
	
	// Main task loop
	while (1)
	{
		GyrosGet(&gyros);
		AccelsGet(&accels);
		AttitudeActualGet(&attitude);
		//AdcsGet(&data);
		PIOS_SSD1308_ClearFrameBuffer();
		/*Text[16]="                 ";
		printFloat(data.Adc2, 2);
		sprintf((char*)Text,"%d.%d Volt           ",(int)(intPart),(int)(fractPart));
		PIOS_SSD1308_drawText(1, 3, 3, (uint8_t *)Text, 16, 0);
		PIOS_SSD1308_drawRectangle(1, 0, 1, (int)(data.Adc2*30.0f), 11);*/
		Text[16]="                 ";
		printFloat(attitude.Roll, 2);
		if(attitude.Roll<=0)
		{
			sprintf((char*)Text,"R:-%d.%d Deg           ",(int)(-intPart),(int)(-fractPart));
		}
		else
		{
			sprintf((char*)Text,"R:%d.%d Deg           ",(int)(intPart),(int)(fractPart));
		}
		PIOS_SSD1308_drawText(1, 3, 3, (uint8_t *)Text, 16, 0);
		PIOS_SSD1308_drawRectangle(1, 64, 1, 64+(int)(attitude.Roll/2), 11);
		Text[16]="                 ";
		printFloat(accels.x, 2);
		if(accels.x<=0)
		{
			sprintf((char*)Text,"x:-%d.%d g            ",(int)(-intPart),(int)(-fractPart));
		}
		else
		{
			sprintf((char*)Text,"x: %d.%d g            ",(int)(intPart),(int)(fractPart));
		}
		PIOS_SSD1308_drawText(1, 3, 14, (uint8_t *)Text, 16, 0);
		PIOS_SSD1308_drawRectangle(1, 64, 12, 64+(int)(accels.x*4), 22);
		Text[16]="                 ";
		printFloat(accels.y, 2);
		if(accels.y<=0)
		{
			sprintf((char*)Text,"y:-%d.%d g            ",(int)(-intPart),(int)(-fractPart));
		}
		else
		{
			sprintf((char*)Text,"y: %d.%d g            ",(int)(intPart),(int)(fractPart));
		}
		PIOS_SSD1308_drawText(1, 3, 25, (uint8_t *)Text, 16, 0);
		PIOS_SSD1308_drawRectangle(1, 64, 23, 64+(int)(accels.y*4), 33);
		Text[16]="                 ";
		printFloat(accels.z, 2);
		if(accels.z<=0)
		{
			sprintf((char*)Text,"z:-%d.%d g            ",(int)(-intPart),(int)(-fractPart));
		}
		else
		{
			sprintf((char*)Text,"z: %d.%d g            ",(int)(intPart),(int)(fractPart));
		}
		PIOS_SSD1308_drawText(1, 3, 36, (uint8_t *)Text, 16, 0);
		PIOS_SSD1308_drawRectangle(1, 64, 34, 64+(int)(accels.z*4), 44);
		Text[16]="                 ";
		printFloat(attitude.Pitch, 2);
		if(attitude.Pitch<=0)
		{
			sprintf((char*)Text,"P:-%d.%d Deg           ",(int)(-intPart),(int)(-fractPart));
		}
		else
		{
			sprintf((char*)Text,"P:%d.%d Deg           ",(int)(intPart),(int)(fractPart));
		}
		PIOS_SSD1308_drawText(1, 3, 47, (uint8_t *)Text, 16, 0);
		PIOS_SSD1308_drawRectangle(1, 64, 45, 64+(int)(attitude.Pitch/2), 55);
		PIOS_SSD1308_ShowFrameBuffer();
		// Update the task
		vTaskDelay(UPDATE_RATE/portTICK_RATE_MS);
	}
}
  
void printFloat(float v, uint8_t decimalDigits)
{
  uint8_t i = 1;
  for (;decimalDigits!=0; i*=10, decimalDigits--);
  intPart = (int16_t)v;
  fractPart = (int16_t)((v-(float)(int16_t)v)*i);
}

/**
 * @}
 * @}
 */
