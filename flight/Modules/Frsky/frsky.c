/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup AltitudeModule Altitude Module
 * @brief Communicate with BMP085 and update @ref BaroAltitude "BaroAltitude UAV Object"
 * @{ 
 *
 * @file       altitude.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Altitude module, handles temperature and pressure readings from BMP085
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


#include "openpilot.h"
#include "hwsettings.h"
#include "frsky.h"
#include "gpsposition.h"
#include "baroaltitude.h"
#include "accels.h"
#include "gyros.h"


// Private constants
#define STACK_SIZE_BYTES 540
#define TASK_PRIORITY (tskIDLE_PRIORITY+1)
#define UPDATE_PERIOD 200

// Private types

// Private variables
static xTaskHandle taskHandle;

static uint32_t telemetryPort;

struct FrFloat
{
	int16_t before;
	uint16_t after;
	uint16_t nbdecimal;
};

int16_t FrAccX;
int16_t FrAccY;
int16_t FrAccZ;
int16_t FrT1;
int16_t FrT2;
int16_t FrV;
uint16_t FrRPM;

uint16_t FrFuel;

uint8_t F1Buff[45];
uint8_t SendPos;
        // Private functions
static void frskyTask(void *parameters);
static void updateSettings();
void SeparateFloat(float Value,struct FrFloat * Fr);
void SendF1(int16_t AccX,int16_t AccY,int16_t AccZ,int16_t Alti1,uint16_t Alti2,uint16_t T1,uint16_t T2,uint16_t V,uint16_t RPM);
void SendF2(int16_t Course1,uint16_t Course2,int16_t Lat1,uint16_t Lat2,int16_t Long1,uint16_t Long2,uint16_t Speed1,uint16_t Speed2,int16_t Alt1,uint16_t Alt2,uint16_t Fuel);

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t FrskyStart()
{

	xTaskCreate(frskyTask, (signed char *)"Frsky", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &taskHandle);
	return 0;
}

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t FrskyInitialize()
{

	// Update telemetry settings
	telemetryPort = PIOS_COM_TELEM_FR;
	//HwSettingsInitialize();
	//updateSettings();
	SendPos=0;
	PIOS_COM_ChangeBaud(telemetryPort, 9600);

	return 0;
}
MODULE_INITCALL(FrskyInitialize, FrskyStart)
/**
 * Module thread, should not return.
 */
static void frskyTask(void *parameters)
{
	portTickType lastSysTime;
	AccelsData adata;
	GyrosData gdata;
	GPSPositionData pdata;
	BaroAltitudeData bdata;

	struct FrFloat course;
	course.nbdecimal=100;
	struct FrFloat lat;
	lat.nbdecimal=1000000;
	struct FrFloat lon;
	lon.nbdecimal=1000000;
	struct FrFloat speed;
	speed.nbdecimal=100;
	struct FrFloat galt;
	galt.nbdecimal=100;
	struct FrFloat balt;
	balt.nbdecimal=100;

	float LatTemp=0.0f;
	float LongTemp=0.0f;


	AccelsInitialize();
	GyrosInitialize();
	GPSPositionInitialize();
	BaroAltitudeInitialize();
	// Main task loop
	lastSysTime = xTaskGetTickCount();
	while (1)
	{
		if(SendPos>=6)
		{
			GPSPositionGet(&pdata);
			SeparateFloat(pdata.Heading,&course);
			LatTemp=((float)pdata.Latitude*0.0000001f);
			SeparateFloat(LatTemp,&lat);
			LongTemp=((float)pdata.Longitude*0.0000001f);
			SeparateFloat(LatTemp,&lon);
			SeparateFloat(pdata.Groundspeed,&speed);
			SeparateFloat(pdata.Altitude,&galt);
			FrFuel=0;
			SendF2(course.before,course.after,lat.before,lat.after,lon.before,lon.after,speed.before,speed.after,galt.before,galt.after,FrFuel);
			SendPos=0;
		}
		AccelsGet(&adata);
		GyrosGet(&gdata);
		BaroAltitudeGet(&bdata);
		FrAccX=(int16_t)(adata.x/0.016f);//according to frsky protocol
		FrAccY=(int16_t)(adata.y/0.016f);//according to frsky protocol
		FrAccZ=(int16_t)(adata.z/0.016f);//according to frsky protocol
		//FrGyroX=(uint16_t)gdata.x;
		//FrGyroY=(uint16_t)gdata.y;
		SeparateFloat(bdata.Altitude,&balt);
		//FrAlti1=(int16_t)bdata.Altitude;
		//FrAlti2=(uint16_t)((bdata.Altitude-(float)FrAlti1)*100);
		FrT1=0;
		FrT2=0;
		FrV=0;
		FrRPM=0;
		SendF1(FrAccX,FrAccY,FrAccZ,balt.before,balt.after,FrT1,FrT2,FrV,FrRPM);
		SendPos++;
		// Delay until it is time to read the next sample
		vTaskDelayUntil(&lastSysTime, UPDATE_PERIOD / portTICK_RATE_MS);
	}
}

/**
 * Update the telemetry settings, called on startup.
 * FIXME: This should be in the TelemetrySettings object. But objects
 * have too much overhead yet. Also the telemetry has no any specific
 * settings, etc. Thus the HwSettings object which contains the
 * telemetry port speed is used for now.
 */
static void updateSettings()
{

	if (telemetryPort) {
		PIOS_COM_ChangeBaud(telemetryPort, 9600);
	}
}


void SendF1(int16_t AccX,int16_t AccY,int16_t AccZ,int16_t Alti1,uint16_t Alti2,uint16_t T1,uint16_t T2,uint16_t V,uint16_t RPM)
{
	uint32_t outputPort;
	F1Buff[0]=0x5e;
	F1Buff[1]=0x24;
	F1Buff[2]=AccX & 0xFF;
	F1Buff[3]=AccX << 8;
	F1Buff[4]=0x5e;
	F1Buff[5]=0x25;
	F1Buff[6]=AccY & 0xFF;
	F1Buff[7]=AccY << 8;
	F1Buff[8]=0x5e;
	F1Buff[9]=0x26;
	F1Buff[10]=AccZ & 0xFF;
	F1Buff[11]=AccZ << 8;
	F1Buff[12]=0x5e;
	F1Buff[13]=0x10;
	F1Buff[14]=Alti1 & 0xFF;
	F1Buff[15]=Alti1 << 8;
	F1Buff[16]=0x5e;
	F1Buff[17]=0x10;
	F1Buff[18]=Alti2 & 0xFF;
	F1Buff[19]=Alti2 << 8;
	F1Buff[20]=0x5e;
	F1Buff[21]=0x02;
	F1Buff[22]=T1 & 0xFF;
	F1Buff[23]=T1 << 8;
	F1Buff[24]=0x5e;
	F1Buff[25]=0x05;
	F1Buff[26]=T2 & 0xFF;
	F1Buff[27]=T2 << 8;
	F1Buff[28]=0x5e;
	F1Buff[29]=0x06;
	F1Buff[30]=V & 0xFF;
	F1Buff[31]=V << 8;
	F1Buff[32]=0x5e;
	F1Buff[33]=0x03;
	F1Buff[34]=RPM & 0xFF;
	F1Buff[35]=RPM << 8;
	F1Buff[36]=0x5e;
	outputPort = telemetryPort;
	if (outputPort) {
		PIOS_COM_SendBuffer(outputPort, F1Buff, 37);
	}
}

void SendF2(int16_t Course1,uint16_t Course2,int16_t Lat1,uint16_t Lat2,int16_t Long1,uint16_t Long2,uint16_t Speed1,uint16_t Speed2,int16_t Alt1,uint16_t Alt2,uint16_t Fuel)
{
	uint32_t outputPort;
	F1Buff[0]=0x5e;
	F1Buff[1]=0x14;
	F1Buff[2]=Course1 & 0xFF;
	F1Buff[3]=Course1 << 8;
	F1Buff[4]=0x5e;
	F1Buff[5]=0x1C;
	F1Buff[6]=Course2 & 0xFF;
	F1Buff[7]=Course2 << 8;
	F1Buff[8]=0x5e;
	F1Buff[9]=0x13;
	F1Buff[10]=Lat1 & 0xFF;
	F1Buff[11]=Lat1 << 8;
	F1Buff[12]=0x5e;
	F1Buff[13]=0x1B;
	F1Buff[14]=Lat2 & 0xFF;
	F1Buff[15]=Lat2 << 8;
	F1Buff[16]=0x5e;
	F1Buff[17]=0x12;
	F1Buff[18]=Long1 & 0xFF;
	F1Buff[19]=Long1 << 8;
	F1Buff[20]=0x5e;
	F1Buff[21]=0x1A;
	F1Buff[22]=Long2 & 0xFF;
	F1Buff[23]=Long2 << 8;
	F1Buff[24]=0x5e;
	F1Buff[25]=0x11;
	F1Buff[26]=Speed1 & 0xFF;
	F1Buff[27]=Speed1 << 8;
	F1Buff[28]=0x5e;
	F1Buff[29]=0x19;
	F1Buff[30]=Speed2 & 0xFF;
	F1Buff[31]=Speed2 << 8;
	F1Buff[32]=0x5e;
	F1Buff[33]=0x01;
	F1Buff[34]=Alt1 & 0xFF;
	F1Buff[35]=Alt1 << 8;
	F1Buff[36]=0x5e;
	F1Buff[37]=0x09;
	F1Buff[38]=Alt2 & 0xFF;
	F1Buff[39]=Alt2 << 8;
	F1Buff[40]=0x5e;
	F1Buff[41]=0x04;
	F1Buff[42]=Fuel & 0xFF;
	F1Buff[43]=Fuel << 8;
	F1Buff[44]=0x5e;
	outputPort = telemetryPort;
		if (outputPort) {
			PIOS_COM_SendBuffer(outputPort, F1Buff, 45);
		}
}

void SeparateFloat(float Value,struct FrFloat * Fr)
{
	Fr->before=(int16_t)Value;
	Fr->after=(uint16_t)(float)((Value-(float)Fr->before)*Fr->nbdecimal);
}

/**
 * @}
 * @}
 */
