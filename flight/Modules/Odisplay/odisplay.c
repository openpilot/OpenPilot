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
#include "baroaltitude.h"
#include "gpsposition.h"
#include "baroairspeed.h"
#include "pipxstatus.h"

#include <pios_board_info.h>

// Private constants
#define STACK_SIZE_BYTES 500
#define TASK_PRIORITY (tskIDLE_PRIORITY+1)
#define UPDATE_RATE 250.0f

int32_t OdisplayInitialize();
static bool OsdInit=false;
int16_t intPart, fractPart;
uint8_t sensPart;

static uint32_t gpsPort;

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
	BaroAltitudeInitialize();
	GPSVelocityInitialize();
	BaroAirspeedInitialize();
	PipXStatusInitialize();
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
	BaroAltitudeData bdata;
	GPSPositionData gpsposition;
	BaroAirspeedData adata;
	PipXStatusData pdata;
	uint8_t Boardtype=0;
	
	const struct pios_board_info * bdinfo = &pios_board_info_blob;
	Boardtype=bdinfo->board_type;

	while (OsdInit==0)
		{
			if((xTaskGetTickCount() > 5000))
			{
				OsdInit=true;
			}
			vTaskDelay(UPDATE_RATE/portTICK_RATE_MS);
		}
	OsdInit=0;

	uint8_t Text[16]={0};
	PIOS_SSD1308_Init(pios_i2c_flexi_adapter_id);//init display on aux i2c
	/*
	switch(bdinfo->board_type) {
		case 0x04:
			{
#if defined(PIOS_I2C_AUX)
			PIOS_SSD1308_Init(pios_i2c_aux_adapter_id);//init display on aux i2c
#else
			PIOS_SSD1308_Init(pios_i2c_flexi_adapter_id);//init display on aux i2c
#endif
			break;
			}
		case 0x03:
			{
			PIOS_SSD1308_Init(pios_i2c_flexi_adapter_id);//init display on aux i2c
			break;
			}
		}*/
	//pipxtreme
	//PIOS_SSD1308_Init(pios_i2c_flexi_adapter_id);//init display on aux i2c
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
		PipXStatusGet(&pdata);
					PIOS_SSD1308_ClearFrameBuffer();
					sprintf((char*)Text,"Tx:%d           ",pdata.TXRate);
					PIOS_SSD1308_drawText(1, 3, 3, (uint8_t *)Text, 16, 0);
					Text[16]="                 ";
					sprintf((char*)Text,"Rx:%d            ",pdata.RXRate);
					PIOS_SSD1308_drawText(1, 3, 14, (uint8_t *)Text, 16, 0);
					Text[16]="                 ";
					sprintf((char*)Text,"RSSI:%d          ",pdata.RSSI);
					PIOS_SSD1308_drawText(1, 3, 25, (uint8_t *)Text, 16, 0);
					PIOS_SSD1308_drawRectangle(1, 0, 23, 127+(int)(pdata.RSSI), 33);
					/*
	switch(Boardtype) {
		case 0x03:
			{
			PipXStatusGet(&pdata);
			PIOS_SSD1308_ClearFrameBuffer();
			sprintf((char*)Text,"Tx:%d           ",pdata.TXRate);
			PIOS_SSD1308_drawText(1, 3, 3, (uint8_t *)Text, 16, 0);
			Text[16]="                 ";
			sprintf((char*)Text,"Rx:%d            ",pdata.RXRate);
			PIOS_SSD1308_drawText(1, 3, 14, (uint8_t *)Text, 16, 0);
			Text[16]="                 ";
			sprintf((char*)Text,"RSSI:%d          ",pdata.RSSI);
			PIOS_SSD1308_drawText(1, 3, 25, (uint8_t *)Text, 16, 0);
			PIOS_SSD1308_drawRectangle(1, 0, 23, 127+(int)(pdata.RSSI), 33);
			break;
			}
		case 0x04:
			{
			GyrosGet(&gyros);
			AccelsGet(&accels);
			AttitudeActualGet(&attitude);
			//AdcsGet(&data);
			PIOS_SSD1308_ClearFrameBuffer();
			gpsPort = 1;//PIOS_COM_GPS;
			if (gpsPort)
			{
				GPSPositionGet(&gpsposition);
				Text[16]="                 ";
				printFloat(gpsposition.Altitude, 2);
				sprintf((char*)Text,"A:%c%d.%d m           ",sensPart,(int)(intPart),(int)(fractPart));
				PIOS_SSD1308_drawText(1, 3, 3, (uint8_t *)Text, 16, 0);
				//PIOS_SSD1308_drawRectangle(1, 64, 1, 64+(int)(attitude.Roll/2), 11);
				Text[16]="                 ";
				printFloat(gpsposition.Groundspeed*3.6f, 2);
				sprintf((char*)Text,"GS:%c%d.%d km/h           ",sensPart,(int)(intPart),(int)(fractPart));
				PIOS_SSD1308_drawText(1, 3, 47, (uint8_t *)Text, 16, 0);
				//PIOS_SSD1308_drawRectangle(1, 64, 45, 64+(int)(attitude.Pitch/2), 55);
			}
			else
			{
				Text[16]="                 ";
				printFloat(attitude.Roll, 2);
				sprintf((char*)Text,"R:%c%d.%d Deg           ",sensPart,(int)(intPart),(int)(fractPart));
				PIOS_SSD1308_drawText(1, 3, 3, (uint8_t *)Text, 16, 0);
				PIOS_SSD1308_drawRectangle(1, 64, 1, 64+(int)(attitude.Roll/2), 11);
				Text[16]="                 ";
				printFloat(attitude.Pitch, 2);
				sprintf((char*)Text,"P:%c%d.%d Deg           ",sensPart,(int)(intPart),(int)(fractPart));
				PIOS_SSD1308_drawText(1, 3, 47, (uint8_t *)Text, 16, 0);
				PIOS_SSD1308_drawRectangle(1, 64, 45, 64+(int)(attitude.Pitch/2), 55);
			}
#if defined(PIOS_INCLUDE_MS5611)//bdata.Variometer
			BaroAltitudeGet(&bdata);
			Text[16]="                 ";
			printFloat(bdata.Variometer, 2);
			sprintf((char*)Text,"V:%c%d.%d m/s            ",sensPart,(int)(intPart),(int)(fractPart));
			PIOS_SSD1308_drawText(1, 3, 14, (uint8_t *)Text, 16, 0);
			if(bdata.Variometer<0.0f)
			{
				PIOS_SSD1308_drawFullRectangle(1, 100, 32,127, 32-(int)(bdata.Variometer*5));
			}
			else
			{
				PIOS_SSD1308_drawFullRectangle(1, 100, 32-(int)(bdata.Variometer*5),127,32);
			}
			BaroAltitudeGet(&bdata);
			Text[16]="                 ";
			printFloat(bdata.Temperature, 1);
			sprintf((char*)Text,"T:%c%d.%d C           ",sensPart,(int)(intPart),(int)(fractPart));
			PIOS_SSD1308_drawText(1, 3, 36, (uint8_t *)Text, 16, 0);
			//-------------airspeed
			BaroAirspeedGet(&adata);
			Text[16]="                 ";
			printFloat(adata.Airspeed, 1);
			sprintf((char*)Text,"AS:%c%d.%d m/s            ",sensPart,(int)(intPart),(int)(fractPart));
			PIOS_SSD1308_drawText(1, 3, 25, (uint8_t *)Text, 16, 0);
			PIOS_SSD1308_drawLine(1, 90, 32, 100, 32);
			PIOS_SSD1308_drawRectangle(1, 100, 0, 127, 63);
			//PIOS_SSD1308_drawRectangle(1, 64, 23, 64+(int)(accels.y*4), 33);
			//PIOS_SSD1308_drawRectangle(1, 64, 34, 64+(int)(accels.z*4), 44);
#else
			Text[16]="                 ";
			printFloat(accels.x, 2);
			sprintf((char*)Text,"x:%c%d.%d g            ",sensPart,(int)(intPart),(int)(fractPart));
			PIOS_SSD1308_drawText(1, 3, 14, (uint8_t *)Text, 16, 0);
			PIOS_SSD1308_drawRectangle(1, 64, 12, 64+(int)(accels.x*4), 22);
			Text[16]="                 ";
			printFloat(accels.y, 2);
			sprintf((char*)Text,"y:%c%d.%d g            ",sensPart,(int)(intPart),(int)(fractPart));
			PIOS_SSD1308_drawText(1, 3, 25, (uint8_t *)Text, 16, 0);
			PIOS_SSD1308_drawRectangle(1, 64, 23, 64+(int)(accels.y*4), 33);
			Text[16]="                 ";
			printFloat(accels.z, 2);
			sprintf((char*)Text,"z:%c%d.%d g            ",sensPart,(int)(intPart),(int)(fractPart));
			PIOS_SSD1308_drawText(1, 3, 36, (uint8_t *)Text, 16, 0);
			PIOS_SSD1308_drawRectangle(1, 64, 34, 64+(int)(accels.z*4), 44);
#endif
			break;
			}
		}*/
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

/**
 * @}
 * @}
 */
