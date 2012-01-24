/*
 *  pios_adxl345.c
 *  OpenPilotOSX
 *
 *  Created by James Cotton on 1/16/11.
 *  Copyright 2011 OpenPilot. All rights reserved.
 *
 */

#include "pios.h"

static uint32_t PIOS_SPI_ACCEL;

/**
 * @brief Claim the SPI bus for the accel communications and select this chip
 */
void PIOS_ADXL345_ClaimBus() 
{
	PIOS_SPI_ClaimBus(PIOS_SPI_ACCEL);
	PIOS_ADXL_ENABLE;		
}

/**
 * @brief Release the SPI bus for the accel communications and end the transaction
 */
void PIOS_ADXL345_ReleaseBus() 
{
	PIOS_ADXL_DISABLE;
	PIOS_SPI_ReleaseBus(PIOS_SPI_ACCEL);
}

/**
 * @brief Select the sampling rate of the chip
 * 
 * This also puts it into high power mode
 */
void PIOS_ADXL345_SelectRate(uint8_t rate) 
{
	uint8_t out[2] = {ADXL_RATE_ADDR, rate & 0x0F};
	PIOS_ADXL345_ClaimBus();
	PIOS_SPI_TransferBlock(PIOS_SPI_ACCEL,out,NULL,sizeof(out),NULL);
	PIOS_ADXL345_ReleaseBus();	
}

/**
 * @brief Set the range of the accelerometer and set the data to be right justified
 * with sign extension.  Also keep device in 4 wire mode.
 */
void PIOS_ADXL345_SetRange(uint8_t range) 
{
	uint8_t out[2] = {ADXL_FORMAT_ADDR, (range & 0x03) | ADXL_FULL_RES | ADXL_4WIRE};
	PIOS_ADXL345_ClaimBus();
	PIOS_SPI_TransferBlock(PIOS_SPI_ACCEL,out,NULL,sizeof(out),NULL);
	PIOS_ADXL345_ReleaseBus();	
}

/**
 * @brief Set the fifo depth that triggers an interrupt.  This will be matched to the oversampling
 */
void PIOS_ADXL345_FifoDepth(uint8_t depth)
{
	uint8_t out[2] = {ADXL_FIFO_ADDR, (depth & 0x1f) | ADXL_FIFO_STREAM};
	PIOS_ADXL345_ClaimBus();
	PIOS_SPI_TransferBlock(PIOS_SPI_ACCEL,out,NULL,sizeof(out),NULL);
	PIOS_ADXL345_ReleaseBus();		
}

/**
 * @brief Enable measuring.  This also disables the activity sensors (tap or free fall)
 */
void PIOS_ADXL345_SetMeasure(uint8_t enable)
{
	uint8_t out[2] = {ADXL_POWER_ADDR, ADXL_MEAURE};
	PIOS_ADXL345_ClaimBus();
	PIOS_SPI_TransferBlock(PIOS_SPI_ACCEL,out,NULL,sizeof(out),NULL);
	PIOS_ADXL345_ReleaseBus();		
}

/**
 * @brief Connect to the correct SPI bus
 */
void PIOS_ADXL345_Attach(uint32_t spi_id)
{
	PIOS_SPI_ACCEL = spi_id;
}

/**
 * @brief Initialize with good default settings
 */
void PIOS_ADXL345_Init()
{
	PIOS_ADXL345_ReleaseBus();
	PIOS_ADXL345_SelectRate(ADXL_RATE_3200);
	PIOS_ADXL345_SetRange(ADXL_RANGE_8G);
	PIOS_ADXL345_FifoDepth(16);
	PIOS_ADXL345_SetMeasure(1); 
}

/**
 * @brief Return number of entries in the fifo
 */
uint8_t  PIOS_ADXL345_FifoElements()
{
	uint8_t buf[2] = {0,0};
	uint8_t rec[2] = {0,0};
	buf[0] = ADXL_FIFOSTATUS_ADDR | ADXL_READ_BIT ; // Read fifo status
	
	PIOS_ADXL345_ClaimBus();
	PIOS_SPI_TransferBlock(PIOS_SPI_ACCEL,&buf[0],&rec[0],sizeof(buf),NULL);
	PIOS_ADXL345_ReleaseBus();
	
	return rec[1] & 0x3f;
}

/**
 * @brief Read a single set of values from the x y z channels
 * @returns The number of samples remaining in the fifo
 */
uint8_t PIOS_ADXL345_Read(struct pios_adxl345_data * data)
{
	// To save memory use same buffer for in and out but offset by
	// a byte
	uint8_t buf[9] = {0,0,0,0,0,0,0,0};
	uint8_t rec[9] = {0,0,0,0,0,0,0,0};
	buf[0] = ADXL_X0_ADDR | ADXL_MULTI_BIT | ADXL_READ_BIT ; // Multibyte read starting at X0
	
	PIOS_ADXL345_ClaimBus();
	PIOS_SPI_TransferBlock(PIOS_SPI_ACCEL,&buf[0],&rec[0],9,NULL);
	PIOS_ADXL345_ReleaseBus();	
	
	data->x = rec[1] + (rec[2] << 8);
	data->y = rec[3] + (rec[4] << 8);
	data->z = rec[5] + (rec[6] << 8);
	
	return rec[8] & 0x7F; // return number of remaining entries
}
