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
#include "oplinkstatus.h"
#include "odisplaysettings.h"

// Private constants
#define STACK_SIZE_BYTES 500
#define TASK_PRIORITY (tskIDLE_PRIORITY+1)
#define UPDATE_RATE 250.0f

int32_t OdisplayInitialize();
static bool OsdInit=false;
int16_t intPart, fractPart;
uint8_t sensPart;

uint8_t OdisplayLine;
uint8_t RefreshODEvery=40;//every 10 sec look for new ODisplaySettings
uint8_t RefreshOD;

//AdcsData data;
AccelsData accels;
GyrosData gyros;
AttitudeActualData attitude;
BaroAltitudeData bdata;
GPSPositionData gpsposition;
BaroAirspeedData adata;
OPLinkStatusData pdata;
ODisplaySettingsData odata;

uint8_t DisplayLines[6]={0x01,0x02,0x03,0x00,0x00,0x00};


// Private types

// Private variables
static xTaskHandle taskHandle;

// Private functions
static void odisplayTask(void *parameters);
void printFloat(float v, uint8_t decimalDigits);
void DrawUAVO(uint8_t *Uavo);

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
	OPLinkStatusInitialize();
	ODisplaySettingsInitialize();
	
	odata.L0=0x01;
	odata.L1=0x02;
	odata.L2=0x03;
	odata.L3=0x00;
	odata.L4=0x00;
	odata.L5=0x00;
	//ODisplaySettingsSet(&odata);
	
	return 0;
}
MODULE_INITCALL(OdisplayInitialize, OdisplayStart)
/**
 * Module thread, should not return.
 */
static void odisplayTask(void *parameters)
{

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
	
#if defined(PIOS_I2C_AUX)
	while(pios_i2c_aux_adapter_id==0)//wait until i2c hasn't been set
	{
		vTaskDelay(1000.0f/portTICK_RATE_MS);
	}
	PIOS_SSD1308_Init(pios_i2c_aux_adapter_id);//init display on aux i2c
#else
	while(pios_i2c_flexi_adapter_id==0)//wait until i2c hasn't been set
	{
		vTaskDelay(1000.0f/portTICK_RATE_MS);
	}
	PIOS_SSD1308_Init(pios_i2c_flexi_adapter_id);//init display on aux i2c
#endif

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
	
	RefreshOD=0;
	OdisplayLine=0;

	// Main task loop
	while (1)
	{
		if(RefreshOD>=RefreshODEvery)
		{
			RefreshOD=0;
			ODisplaySettingsGet(&odata);
			DisplayLines[0]=(uint8_t)odata.L0;
			DisplayLines[1]=(uint8_t)odata.L1;
			DisplayLines[2]=(uint8_t)odata.L2;
			DisplayLines[3]=(uint8_t)odata.L3;
			DisplayLines[4]=(uint8_t)odata.L4;
			DisplayLines[5]=(uint8_t)odata.L5;
		}
		else
		{
			RefreshOD++;
		}
	
		//PIOS_SSD1308_ClearFrameBuffer();

		//DrawUAVO(DisplayLines[0],0);
		DrawUAVO(DisplayLines);

		/*for(OdisplayLine=0;OdisplayLine<6;OdisplayLine++)
		{
			DrawUAVO(DisplayLines[OdisplayLine],OdisplayLine);
		}*/
		
		/*OPLinkStatusGet(&pdata);
		sprintf((char*)Text,"Tx:%d           ",pdata.TXRate);
		PIOS_SSD1308_drawText(1, 3, 3, (uint8_t *)Text, 16, 0);
		Text[16]="                 ";
		sprintf((char*)Text,"Rx:%d            ",pdata.RXRate);
		PIOS_SSD1308_drawText(1, 3, 14, (uint8_t *)Text, 16, 0);
		Text[16]="                 ";
		sprintf((char*)Text,"RSSI:%d          ",pdata.RSSI);
		PIOS_SSD1308_drawText(1, 3, 25, (uint8_t *)Text, 16, 0);
		PIOS_SSD1308_drawRectangle(1, 0, 23, 127+(int)(pdata.RSSI), 33);
		*/
		/*
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
#endif*/
		//PIOS_SSD1308_ShowFrameBuffer();
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

void DrawUAVO(uint8_t *Uavo)
{
	uint8_t Text[16]={0};
	PIOS_SSD1308_ClearFrameBuffer();
	for(OdisplayLine=0;OdisplayLine<6;OdisplayLine++)
	{
		switch (Uavo[OdisplayLine])
		{
			case UAVO_NONE:
			{
				break;
			}
			case UAVO_TX:
			{
				OPLinkStatusGet(&pdata);
				sprintf((char*)Text,"Tx:%d            ",pdata.TXRate);
				PIOS_SSD1308_drawText(1, 3, (10*OdisplayLine)+2, (uint8_t *)Text, 16, 0);
				break;
			}
			case UAVO_RX:
			{
				OPLinkStatusGet(&pdata);
				sprintf((char*)Text,"Rx:%d            ",pdata.RXRate);
				PIOS_SSD1308_drawText(1, 3, (10*OdisplayLine)+2, (uint8_t *)Text, 16, 0);
				break;
			}
			case UAVO_RSSI:
			{
				OPLinkStatusGet(&pdata);
				sprintf((char*)Text,"Rssi:%d            ",pdata.RSSI);
				PIOS_SSD1308_drawText(1, 3, (10*OdisplayLine)+2, (uint8_t *)Text, 16, 0);
				PIOS_SSD1308_drawRectangle(1, 0, 10*OdisplayLine, 127+(int)(pdata.RSSI), 10*(OdisplayLine+1));
				break;
			}
			case UAVO_ALTITUDE:
			{
				BaroAltitudeGet(&bdata);
				printFloat(bdata.Altitude, 2);
				sprintf((char*)Text,"A:%c%d.%d m            ",sensPart,(int)(intPart),(int)(fractPart));
				PIOS_SSD1308_drawText(1, 3, (10*OdisplayLine)+2, (uint8_t *)Text, 16, 0);
				break;
			}
			case UAVO_VARIOMETER:
			{
				BaroAltitudeGet(&bdata);
				printFloat(bdata.Variometer, 2);
				sprintf((char*)Text,"V:%c%d.%d m/s          ",sensPart,(int)(intPart),(int)(fractPart));
				PIOS_SSD1308_drawText(1, 3, (10*OdisplayLine)+2, (uint8_t *)Text, 16, 0);
				if(bdata.Variometer<0.0f)
				{
					PIOS_SSD1308_drawFullRectangle(1, 100, 32,127, 32-(int)(bdata.Variometer*5));
				}
				else
				{
					PIOS_SSD1308_drawFullRectangle(1, 100, 32-(int)(bdata.Variometer*5),127,32);
				}
				break;
				PIOS_SSD1308_drawLine(1, 90, 32, 100, 32);
				PIOS_SSD1308_drawRectangle(1, 100, 0, 127, 63);
			}
			case UAVO_GALTITUDE:
			{
				GPSPositionGet(&gpsposition);
				printFloat(gpsposition.Altitude, 2);
				sprintf((char*)Text,"GA:%c%d.%d m          ",sensPart,(int)(intPart),(int)(fractPart));
				PIOS_SSD1308_drawText(1, 3, (10*OdisplayLine)+2, (uint8_t *)Text, 16, 0);
				break;
			}
			case UAVO_GSPEED:
			{
				GPSPositionGet(&gpsposition);
				printFloat(gpsposition.Groundspeed*3.6f, 2);
				sprintf((char*)Text,"GS:%c%d.%d km/h        ",sensPart,(int)(intPart),(int)(fractPart));
				PIOS_SSD1308_drawText(1, 3, (10*OdisplayLine)+2, (uint8_t *)Text, 16, 0);
				break;
			}
			case UAVO_TEMPERATURE:
			{
				BaroAltitudeGet(&bdata);
				printFloat(bdata.Temperature, 2);
				sprintf((char*)Text,"T:%c%d.%d C        ",sensPart,(int)(intPart),(int)(fractPart));
				PIOS_SSD1308_drawText(1, 3, (10*OdisplayLine)+2, (uint8_t *)Text, 16, 0);
				break;
			}
			case UAVO_BATTERY:
			{
				//BaroAltitudeGet(&bdata);
				//printFloat(bdata.Temperature, 2);
				//sprintf((char*)Text,"T:%c%d.%d C        ",sensPart,(int)(intPart),(int)(fractPart));
				//PIOS_SSD1308_drawText(1, 3, (10*Line)+2, (uint8_t *)Text, 16, 0);
				break;
			}
		}
	}
	PIOS_SSD1308_ShowFrameBuffer();
}

/**
 * @}
 * @}
 */
