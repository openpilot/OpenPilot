/**
  ******************************************************************************
  * @addtogroup PIOS PIOS Core hardware abstraction layer
  * @{
  * @addtogroup PIOS_MAX31855 MAX31855 Functions
  * @brief Hardware functions to deal with the altitude pressure sensor
  * @{
  *
  * @file       pios_max31855.c
  * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
  * @brief      MAX31855 thermocouple Sensor Routines
  * @see        The GNU Public License (GPL) Version 3
  *
  ******************************************************************************/
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

/* Project Includes */
#include "pios.h"

#if defined(PIOS_INCLUDE_MAX31855)
#endif

/* Local Variables */

static float CaptureValue;

static uint8_t Max31855Value[4];

static const uint8_t BinValue[8]={128,64,32,16,8,4,2,1};

#define PIOS_MAX31855_CLK_GPIO_PORT                  GPIOA
#define PIOS_MAX31855_CLK_PIN                        GPIO_Pin_2
#define PIOS_MAX31855_SDI_GPIO_PORT                  GPIOA
#define PIOS_MAX31855_SDI_PIN                        GPIO_Pin_0
#define PIOS_MAX31855_CS_GPIO_PORT                   GPIOA
#define PIOS_MAX31855_CS_PIN                         GPIO_Pin_1

/**
* Initialise the thermocouple sensor
*/
void PIOS_MAX31855_Init(void)
{
#if defined(PIOS_MAX31855_LOWTECH)//don't use hardware half duplex SPI
	/* Init clkpin */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Pin = PIOS_MAX31855_CLK_PIN;
	GPIO_Init(PIOS_MAX31855_CLK_GPIO_PORT, &GPIO_InitStructure);
	PIOS_MAX31855_CLK_GPIO_PORT->BSRR = PIOS_MAX31855_CLK_PIN;
	
	/* Init cskpin */
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Pin = PIOS_MAX31855_CS_PIN;
	GPIO_Init(PIOS_MAX31855_CS_GPIO_PORT, &GPIO_InitStructure);
	PIOS_MAX31855_CS_GPIO_PORT->BSRR = PIOS_MAX31855_CS_PIN;

	/* Configure input pins */
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Pin = PIOS_MAX31855_SDI_PIN;
	GPIO_Init(PIOS_MAX31855_SDI_GPIO_PORT, &GPIO_InitStructure);
#else
#endif

}

/**
* Get the value of the temperature
* \output >0 timer value
*/
float PIOS_MAX31855_Get(void)
{
	int32_t RefT=0;
#if defined(PIOS_MAX31855_LOWTECH)//don't use half duplex SPI
	uint8_t index;
	Max31855Value[0]=0;
	Max31855Value[1]=0;
	Max31855Value[2]=0;
	Max31855Value[3]=0;
	PIOS_MAX31855_CLK_GPIO_PORT->BRR = PIOS_MAX31855_CLK_PIN; //lower Clk pin
	PIOS_DELAY_WaituS(15);//wait a bit
	PIOS_MAX31855_CS_GPIO_PORT->BRR = PIOS_MAX31855_CS_PIN; //put CS low
	PIOS_DELAY_WaituS(10);//wait a bit
	for(index=0;index<8;index++)
	{
		PIOS_MAX31855_CLK_GPIO_PORT->BSRR = PIOS_MAX31855_CLK_PIN;//raise Clkpin
		PIOS_DELAY_WaituS(10);//wait a bit
		Max31855Value[0]+=GPIO_ReadInputDataBit(PIOS_MAX31855_SDI_GPIO_PORT,PIOS_MAX31855_SDI_PIN) * BinValue[index];
		PIOS_MAX31855_CLK_GPIO_PORT->BRR = PIOS_MAX31855_CLK_PIN; //lower Clk pin
		PIOS_DELAY_WaituS(10);//wait a bit
	}
	for(index=0;index<8;index++)
	{
		PIOS_MAX31855_CLK_GPIO_PORT->BSRR = PIOS_MAX31855_CLK_PIN;//raise Clkpin
		PIOS_DELAY_WaituS(10);//wait a bit
		Max31855Value[1]+=GPIO_ReadInputDataBit(PIOS_MAX31855_SDI_GPIO_PORT,PIOS_MAX31855_SDI_PIN) * BinValue[index];
		PIOS_MAX31855_CLK_GPIO_PORT->BRR = PIOS_MAX31855_CLK_PIN; //lower Clk pin
		PIOS_DELAY_WaituS(10);//wait a bit
	}
	for(index=0;index<8;index++)
	{
		PIOS_MAX31855_CLK_GPIO_PORT->BSRR = PIOS_MAX31855_CLK_PIN;//raise Clkpin
		PIOS_DELAY_WaituS(10);//wait a bit
		Max31855Value[2]+=GPIO_ReadInputDataBit(PIOS_MAX31855_SDI_GPIO_PORT,PIOS_MAX31855_SDI_PIN) * BinValue[index];
		PIOS_MAX31855_CLK_GPIO_PORT->BRR = PIOS_MAX31855_CLK_PIN; //lower Clk pin
		PIOS_DELAY_WaituS(10);//wait a bit
	}
	for(index=0;index<8;index++)
	{
		PIOS_MAX31855_CLK_GPIO_PORT->BSRR = PIOS_MAX31855_CLK_PIN;//raise Clkpin
		PIOS_DELAY_WaituS(10);//wait a bit
		Max31855Value[3]+=GPIO_ReadInputDataBit(PIOS_MAX31855_SDI_GPIO_PORT,PIOS_MAX31855_SDI_PIN) * BinValue[index];
		PIOS_MAX31855_CLK_GPIO_PORT->BRR = PIOS_MAX31855_CLK_PIN; //lower Clk pin
		PIOS_DELAY_WaituS(10);//wait a bit
	}
	PIOS_MAX31855_CS_GPIO_PORT->BSRR = PIOS_MAX31855_CS_PIN; //put CS High
	if((Max31855Value[3] & 0x01)==0x01)
	{
		//opencircuit
	}
	else if((Max31855Value[3] & 0x02)==0x02)
	{
		//grounded
	}
	else if((Max31855Value[3] & 0x04)==0x04)
	{
		//vcc
	}
	else
	{
		CaptureValue=(float)((((Max31855Value[0]*256)+Max31855Value[1])&0x00007ffc)>>2)/4.0f;
		RefT=(float)((((Max31855Value[2]*256)+Max31855Value[3])&0x00007ff0)>>4)/16.0f;
	}
	//CaptureValue+=RefT;
	//CaptureValue=CaptureValue&0xfffc;
	//CaptureValue=CaptureValue>>2;
	/*if((CaptureValue&0x2000)==1)
	{
		CaptureValue+=0xc000;
	}*/
#else
	CaptureValue=0;
#endif
	return CaptureValue;
}

