/**
 ******************************************************************************
 *
 * @file       pios_bmp085.c  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2009.   
 * @brief      IRQ Enable/Disable routines
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   PIOS_BMP085 BMP085 Functions
 * @{
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
/*
Example of how to use this module:
	PIOS_BMP085_Init();
	
	PIOS_BMP085_StartADC(Temperature);
		(Interrupt reads the ADC)
	
	Somwhere inbetween these 2 calls we need to wait for the first one to finish
	
	PIOS_BMP085_StartADC(Pressure);
		(Interrupt reads the ADC)
	
	PIOS_BMP085_GetValues(&Pressure, &Altitude, &Temperature);
*/

/* Project Includes */
#include "pios.h"


/* Glocal Variables */
ConversionTypeTypeDef CurrentRead;


/* Local Variables */
static BMP085CalibDataTypeDef CalibData;
static uint16_t RawPressure;
static uint32_t RawTemperature;


/**
* Initialise the BMP085 sensor
*/
void PIOS_BMP085_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* Enable EOC GPIO clock */
	RCC_APB2PeriphClockCmd(BMP085_EOC_CLK | RCC_APB2Periph_AFIO, ENABLE);

	/* Configure EOC pin as input floating */
	GPIO_InitStructure.GPIO_Pin = BMP085_EOC_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(BMP085_EOC_GPIO_PORT, &GPIO_InitStructure);
	
	/* Configure the End Of Conversion (EOC) interrup */
	GPIO_EXTILineConfig(BMP085_EOC_PORT_SOURCE, BMP085_EOC_PIN_SOURCE);
	EXTI_InitStructure.EXTI_Line = BMP085_EOC_EXTI_LINE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	/* Enable and set EOC EXTI Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = BMP085_EOC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 15;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	/* Read all 22 bytes of calibration data in one transfer, this is a very optimised way of doing things */
	uint8_t Data[22];
	PIOS_BMP085_Read(BMP085_CALIB_ADDR, Data, BMP085_CALIB_LEN);
	
	/* Parameters AC1-AC6 */
	CalibData.AC1 = (Data[0] << 8) | Data[1];
	CalibData.AC2 = (Data[2] << 8) | Data[3];
	CalibData.AC3 = (Data[4] << 8) | Data[5];
	CalibData.AC4 = (Data[6] << 8) | Data[7];
	CalibData.AC5 = (Data[8] << 8) | Data[9];
	CalibData.AC6 = (Data[10] << 8) | Data[11];

	/* Parameters B1, B2 */
	CalibData.B1 =  (Data[12] << 8) | Data[13];
	CalibData.B2 =  (Data[14] << 8) | Data[15];

	/* Parameters MB, MC, MD */
	CalibData.MB =  (Data[16] << 8) | Data[17];
	CalibData.MC =  (Data[18] << 8) | Data[19];
	CalibData.MD =  (Data[20] << 8) | Data[21];
}


/**
* Start the ADC conversion
* \param[in] PresOrTemp BMP085_PRES_ADDR or BMP085_TEMP_ADDR
* \return Raw ADC value
*/
void PIOS_BMP085_StartADC(ConversionTypeTypeDef Type)
{
	/* Start the conversion */
	if(Type == Temperature) {
		PIOS_BMP085_Write(BMP085_CTRL_ADDR, (uint8_t *)BMP085_TEMP_ADDR, 1);
	} else if(Type == Pressure) {
		PIOS_BMP085_Write(BMP085_CTRL_ADDR, (uint8_t *)BMP085_PRES_ADDR, 1);
	}
	
	CurrentRead = Type;
}


/**
* Read the ADC conversion value (once ADC converion has completed)
* \param[in] PresOrTemp BMP085_PRES_ADDR or BMP085_TEMP_ADDR
* \return Raw ADC value
*/
void PIOS_BMP085_ReadADC(void)
{
	uint8_t Data[3];
	
	/* Read and store the 16bit result */
	if(CurrentRead == Temperature) {
		/* Read the temperature conversion */
		PIOS_BMP085_Read(BMP085_ADC_MSB, Data, 2);
		RawTemperature = ((Data[0] << 8) | Data[1]);
	} else {
		/* Read the pressure conversion */
		PIOS_BMP085_Read(BMP085_ADC_MSB, Data, 3);
		RawPressure = ((Data[0] << 16) | (Data[1] << 8) | Data[2]) >> (8 - BMP085_OVERSAMPLING);
	}
}


/**
* Get the converted raw values
* \param[out] Pressure Pointer to the pressure variable
* \param[out] Altitude Pointer to the altitude variable
* \param[out] Temperature Pointer to the temperature variable
*/
void PIOS_BMP085_GetValues(uint16_t *Pressure, uint16_t *Altitude, uint16_t *Temperature)
{
	/* Straight from the datasheet */
	int32_t X1, X2, X3, B3, B5, B6, P;
	uint32_t B4, B7;

	/* Convert Temperatre */
	X1 = (RawTemperature - CalibData.AC6) * CalibData.AC5 >> 15;
	X2 = ((int32_t) CalibData.MC << 11) / (X1 + CalibData.MD);
	B5 = X1 + X2;
	*Temperature = (B5 + 8) >> 4;

	/* Calculate Pressure */
	B6 = B5 - 4000;
	X1 = (CalibData.B2 * (B6 * B6 >> 12)) >> 11;
	X2 = CalibData.AC2 * B6 >> 11;
	X3 = X1 + X2;
	B3 = ((int32_t) CalibData.AC1 * 4 + X3 + 2) >> 2;
	X1 = CalibData.AC3 * B6 >> 13;
	X2 = (CalibData.B1 * (B6 * B6 >> 12)) >> 16;
	X3 = ((X1 + X2) + 2) >> 2;
	B4 = (CalibData.AC4 * (uint32_t) (X3 + 32768)) >> 15;
	B7 = ((uint32_t) RawPressure - B3) * 50000;
	P = B7 < 0x80000000 ? (B7 * 2) / B4 : (B7 / B4) * 2;
	X1 = (P >> 8) * (P >> 8);
	X1 = (X1 * 3038) >> 16;
	X2 = (-7357 * P) >> 16;
	*Pressure = P + ((X1 + X2 + 3791) >> 4);
	
	/* Calculate Altitude */
	*Altitude = (uint16_t) 44330 * (1 - (pow((*Pressure/BMP085_P0), (1/5.255))));
}


/**
* Reads one or more bytes into a buffer
* \param[in] address BMP085 register address (depends on size)
* \param[out] buffer destination buffer
* \param[in] len number of bytes which should be read (1..64)
* \return 0 if operation was successful
* \return -1 if error during I2C transfer
* \return -2 if BMP085 blocked by another task (retry it!)
* \return -4 if invalid length
*/
int32_t PIOS_BMP085_Read(uint16_t address, uint8_t *buffer, uint8_t len)
{
	/* Try to get the IIC peripheral */
	if(PIOS_I2C_TransferBegin(I2C_Non_Blocking) < 0) {
		/* Request a retry */
		return -2;
	}

	/* Send I2C address and EEPROM address */
	/* To avoid issues with litte/big endian: copy address into temporary buffer */
	uint8_t addr_buffer[2] = {(uint8_t)(address>>8), (uint8_t)address};
	int32_t error = PIOS_I2C_Transfer(I2C_Write_WithoutStop, BMP085_I2C_ADDR, addr_buffer, 2);

	if(!error) {
		error = PIOS_I2C_TransferWait();
	}

	/* Now receive byte(s) */
	if(!error) {
		error = PIOS_I2C_Transfer(I2C_Read, BMP085_I2C_ADDR, buffer, len);
	}
	if(!error) {
		error = PIOS_I2C_TransferWait();
	}

	/* Release I2C peripheral */
	PIOS_I2C_TransferFinished();
	
	/* Return error status */
	return error < 0 ? -1 : 0;
}


/**
* Writes one or more bytes to the BMP085
* \param[in] address BankStick address (depends on size)
* \param[in] buffer source buffer
* \param[in] len number of bytes which should be written (1..64)
* \return 0 if operation was successful
* \return -1 if error during IIC transfer
* \return -2 if BankStick blocked by another task (retry it!)
* \note Use \ref PIOS_I2C_BS_CheckWriteFinished to check when the write operation
* has been finished - this can take up to 5 mS!
*/
int32_t PIOS_BMP085_Write(uint16_t address, uint8_t *buffer, uint8_t len)
{
	/* Try to get the IIC peripheral */
	if(PIOS_I2C_TransferBegin(I2C_Non_Blocking) < 0) {
		/* Request a retry */
		return -2;
	}

	/* Send I2C address and data */
	uint8_t WriteBuffer[64+2];
	WriteBuffer[0] = (uint8_t) (address >> 8);
	WriteBuffer[1] = (uint8_t) address;

	uint8_t i;
	for(i = 0; i < len; i++) {
		WriteBuffer[i+2] = buffer[i];
	}
	
	int32_t error = PIOS_I2C_Transfer(I2C_Write, BMP085_I2C_ADDR, WriteBuffer, len+2);
	
	if(!error) {
		error = PIOS_I2C_TransferWait();
	}

	/* Release I2C peripheral */
	PIOS_I2C_TransferFinished();
	
	/* Return error status */
	return error < 0 ? -1 : 0;
}


/**
* This function handles External lines 15 to 10 interrupt request
*/
void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(BMP085_EOC_EXTI_LINE) != RESET) {
		/* Disable interrupts */
		PIOS_IRQ_Disable();
		
		/* Read the ADC Value */
		PIOS_BMP085_ReadADC();
		
		/* Clear the EOC EXTI line pending bit */
		EXTI_ClearITPendingBit(BMP085_EOC_EXTI_LINE);
		
		/* Enable interrupts */
		PIOS_IRQ_Enable();
	}
}
