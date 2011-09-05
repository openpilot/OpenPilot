/*
 *  pios_adxl345.h
 *  OpenPilotOSX
 *
 *  Created by James Cotton on 1/16/11.
 *  Copyright 2011 OpenPilot. All rights reserved.
 *
 */

#ifndef PIOS_ADXL345_H
#define PIOS_ADXL345_H

// Defined by data rate, not BW

#define ADXL_READ_BIT      0x80
#define ADXL_MULTI_BIT     0x40

#define ADXL_X0_ADDR       0x32
#define ADXL_FIFOSTATUS_ADDR 0x39

#define ADXL_RATE_ADDR     0x2C
#define ADXL_RATE_100      0x0A
#define ADXL_RATE_200      0x0B
#define ADXL_RATE_400      0x0C
#define ADXL_RATE_800      0x0D
#define ADXL_RATE_1600     0x0E
#define ADXL_RATE_3200     0x0F

#define ADXL_POWER_ADDR    0x2D
#define ADXL_MEAURE        0x08

#define ADXL_FORMAT_ADDR   0x31
#define ADXL_FULL_RES      0x08
#define ADXL_4WIRE         0x00
#define ADXL_RANGE_2G      0x00
#define ADXL_RANGE_4G      0x01
#define ADXL_RANGE_8G      0x02
#define ADXL_RANGE_16G     0x03

#define ADXL_FIFO_ADDR     0x38
#define ADXL_FIFO_STREAM   0x80


struct pios_adxl345_data {
	int16_t x;
	int16_t y;
	int16_t z;
};

void PIOS_ADXL345_SelectRate(uint8_t rate); 
void PIOS_ADXL345_SetRange(uint8_t range);
void PIOS_ADXL345_FifoDepth(uint8_t depth);
void PIOS_ADXL345_Attach(uint32_t spi_id);
void PIOS_ADXL345_Init();
uint8_t PIOS_ADXL345_Read(struct pios_adxl345_data * data);
uint8_t  PIOS_ADXL345_FifoElements();

#endif
