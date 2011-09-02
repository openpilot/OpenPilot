/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_BMA180 BMA180 Functions
 * @brief Deals with the hardware interface to the BMA180 3-axis accelerometer
 * @{
 *
 * @file       pios_bma180.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      PiOS BMA180 digital accelerometer driver.
 *                 - Driver for the BMA180 digital accelerometer on the SPI bus.
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

#include "pios.h"
#include "fifo_buffer.h"

static uint32_t PIOS_SPI_ACCEL;

static int32_t PIOS_BMA180_GetReg(uint8_t reg);
static int32_t PIOS_BMA180_SetReg(uint8_t reg, uint8_t data);
static int32_t PIOS_BMA180_SelectBW(enum bma180_bandwidth bw);
static int32_t PIOS_BMA180_SetRange(enum bma180_range range);
static int32_t PIOS_BMA180_Config();
static int32_t PIOS_BMA180_EnableIrq();
static void PIOS_BMA180_IRQHandler(void);

#define PIOS_BMA180_MAX_DOWNSAMPLE 10
static int16_t pios_bma180_buffer[PIOS_BMA180_MAX_DOWNSAMPLE * sizeof(struct pios_bma180_data)];
static t_fifo_buffer pios_bma180_fifo;

static const struct pios_bma180_cfg * dev_cfg;
static enum bma180_range range;

#define GRAV 9.81
/**
 * @brief Initialize with good default settings
 */
int32_t PIOS_BMA180_Init(const struct pios_bma180_cfg * cfg)
{	
	dev_cfg = cfg; // store config before enabling interrupt
	
	fifoBuf_init(&pios_bma180_fifo, (uint8_t *) pios_bma180_buffer, sizeof(pios_bma180_buffer));
	
	/* Configure EOC pin as input floating */
	GPIO_Init(cfg->drdy.gpio, &cfg->drdy.init);
	
	/* Configure the End Of Conversion (EOC) interrupt */
	SYSCFG_EXTILineConfig(cfg->eoc_exti.port_source, cfg->eoc_exti.pin_source);
	EXTI_Init(&cfg->eoc_exti.init);
	
	/* Enable and set EOC EXTI Interrupt to the lowest priority */
	NVIC_Init(&cfg->eoc_irq.init);
	
	if(PIOS_BMA180_Config() < 0)
		return -1;
	PIOS_DELAY_WaituS(50);
	PIOS_BMA180_EnableIrq();
	return 0;
}

/**
 * @brief Claim the SPI bus for the accel communications and select this chip
 * @return 0 if successful, -1 if unable to claim bus
 */
int32_t PIOS_BMA180_ClaimBus()
{
	if(PIOS_SPI_ClaimBus(PIOS_SPI_ACCEL) != 0)
		return -1;
	PIOS_SPI_RC_PinSet(PIOS_SPI_ACCEL,0,0);
	return 0;
}

/**
 * @brief Release the SPI bus for the accel communications and end the transaction
 * @return 0 if successful
 */
int32_t PIOS_BMA180_ReleaseBus()
{
	PIOS_SPI_RC_PinSet(PIOS_SPI_ACCEL,0,1);
	return PIOS_SPI_ReleaseBus(PIOS_SPI_ACCEL);
}

/**
 * @brief Read a register from BMA180
 * @returns The register value or -1 if failure to get bus
 * @param reg[in] Register address to be read
 */
int32_t PIOS_BMA180_GetReg(uint8_t reg)
{
	uint8_t data;
	
	if(PIOS_BMA180_ClaimBus() != 0)
		return -1;	

	PIOS_SPI_TransferByte(PIOS_SPI_ACCEL,(0x80 | reg) ); // request byte
	data = PIOS_SPI_TransferByte(PIOS_SPI_ACCEL,0 );     // receive response
	
	PIOS_BMA180_ReleaseBus();
	return data;
}

/**
 * @brief Write a BMA180 register.  EEPROM must be unlocked before calling this function.
 * @return none
 * @param reg[in] address of register to be written
 * @param data[in] data that is to be written to register
 */
int32_t PIOS_BMA180_SetReg(uint8_t reg, uint8_t data)
{
	if(PIOS_BMA180_ClaimBus() != 0)
		return -1;
	
	PIOS_SPI_TransferByte(PIOS_SPI_ACCEL, 0x7f & reg);
	PIOS_SPI_TransferByte(PIOS_SPI_ACCEL, data);

	PIOS_BMA180_ReleaseBus();
	
	return 0;
}


static int32_t PIOS_BMA180_EnableEeprom() {
	// Enable EEPROM writing
	int32_t byte = PIOS_BMA180_GetReg(BMA_CTRREG0);
	if(byte < 0)
		return -1;
	byte |= 0x10;                                      // Set bit 4
	if(PIOS_BMA180_SetReg(BMA_CTRREG0,(uint8_t) byte) < 0)    // Have to set ee_w to		
		return -1;
	return 0;
}

static int32_t PIOS_BMA180_DisableEeprom() {
	// Enable EEPROM writing
	int32_t byte = PIOS_BMA180_GetReg(BMA_CTRREG0);
	if(byte < 0)
		return -1;
	byte |= 0x10;                                      // Set bit 4
	if(PIOS_BMA180_SetReg(BMA_CTRREG0,(uint8_t) byte) < 0)    // Have to set ee_w to		
		return -1;
	return 0;
}

/**
 * @brief Set the default register settings
 * @return 0 if successful, -1 if not
 */
static int32_t PIOS_BMA180_Config() 
{
	/*
	0x35 = 0x81  //smp-skip = 1 for less interrupts
	0x33 = 0x81  //shadow-dis = 1, update MSB and LSB synchronously
	0x27 = 0x01  //dis-i2c
	0x21 = 0x02  //new_data_int = 1
	 */
		
	PIOS_DELAY_WaituS(20);
	
	if(PIOS_BMA180_EnableEeprom() < 0)
		return -1;
	if(PIOS_BMA180_SetReg(BMA_RESET, BMA_RESET_CODE) < 0)
		return -1;
	if(PIOS_BMA180_SetReg(BMA_OFFSET_LSB1, 0x81) < 0)
		return -1;
	if(PIOS_BMA180_SetReg(BMA_GAIN_Y, 0x81) < 0)
		return -1;
	if(PIOS_BMA180_SetReg(BMA_CTRREG3, 0xFF) < 0)
		return -1;
	PIOS_BMA180_SelectBW(BMA_BW_600HZ);
	PIOS_BMA180_SetRange(BMA_RANGE_8G);
	
	if(PIOS_BMA180_DisableEeprom() < 0)
		return -1;

	return 0;
}

/**
 * @brief Select the bandwidth the digital filter pass allows.
 * @return 0 if successful, -1 if not
 * @param rate[in] Bandwidth setting to be used
 * 
 * EEPROM must be write-enabled before calling this function.
 */
static int32_t PIOS_BMA180_SelectBW(enum bma180_bandwidth bw)
{
	uint8_t reg;
	reg = PIOS_BMA180_GetReg(BMA_BW_ADDR);
	reg = (reg & ~BMA_BW_MASK) | ((bw << BMA_BW_SHIFT) & BMA_BW_MASK);
	return PIOS_BMA180_SetReg(BMA_BW_ADDR, reg);
}

/**
 * @brief Select the full scale acceleration range.
 * @return 0 if successful, -1 if not
 * @param rate[in] Range setting to be used
 *
 */
static int32_t PIOS_BMA180_SetRange(enum bma180_range new_range) 
{
	uint8_t reg;
	range = new_range;
	reg = PIOS_BMA180_GetReg(BMA_RANGE_ADDR);
	reg = (reg & ~BMA_RANGE_MASK) | ((range << BMA_RANGE_SHIFT) & BMA_RANGE_MASK);
	return PIOS_BMA180_SetReg(BMA_RANGE_ADDR, reg);
}

static int32_t PIOS_BMA180_EnableIrq() 
{

	if(PIOS_BMA180_EnableEeprom() < 0)
		return -1;

	if(PIOS_BMA180_SetReg(BMA_CTRREG3, BMA_NEW_DAT_INT) < 0)
		return -1;

	if(PIOS_BMA180_DisableEeprom() < 0)
		return -1;

	return 0;
}

/**
 * @brief Connect to the correct SPI bus
 */
void PIOS_BMA180_Attach(uint32_t spi_id)
{
	PIOS_SPI_ACCEL = spi_id;
}

/**
 * @brief Read a single set of values from the x y z channels
 * @param[out] data Int16 array of (x,y,z) sensor values
 * @returns 0 if successful
 * @retval -1 unable to claim bus
 * @retval -2 unable to transfer data
 */
int32_t PIOS_BMA180_ReadAccels(struct pios_bma180_data * data)
{
	// To save memory use same buffer for in and out but offset by
	// a byte
	uint8_t buf[7] = {BMA_X_LSB_ADDR | 0x80,0,0,0,0,0};
	uint8_t rec[7] = {0,0,0,0,0,0};
	
	if(PIOS_BMA180_ClaimBus() != 0)
		return -1;
	if(PIOS_SPI_TransferBlock(PIOS_SPI_ACCEL,&buf[0],&rec[0],7,NULL) != 0)
		return -2;
	PIOS_BMA180_ReleaseBus();	
	
	//        |    MSB        |   LSB       | 0 | new_data |
	data->x = ((rec[2] << 8) | rec[1]);
	data->y = ((rec[4] << 8) | rec[3]);
	data->z = ((rec[6] << 8) | rec[5]);
	data->x /= 4;
	data->y /= 4;
	data->z /= 4;
	
	return 0; // return number of remaining entries
}

/**
 * @brief Returns the scale the BMA180 chip is using
 * @return Scale (m / s^2) / LSB
 */
float PIOS_BMA180_GetScale()
{
	switch (range) {
		case BMA_RANGE_1G:
			return GRAV / 8192.0;
		case BMA_RANGE_1_5G:
			return GRAV / 5460.0;
		case BMA_RANGE_2G:
			return GRAV / 4096.0;
		case BMA_RANGE_3G:
			return GRAV / 2730.0;
		case BMA_RANGE_4G:
			return GRAV / 2048.0;
		case BMA_RANGE_8G:
			return GRAV / 1024.0;
		case BMA_RANGE_16G:
			return GRAV / 512.0;
	}
	return 0;
}

/**
 * @brief Get data from fifo
 * @param [out] buffer pointer to a @ref pios_bma180_data structure to receive data
 * @return 0 for success, -1 for failure (no data available)
 */
int32_t PIOS_BMA180_ReadFifo(struct pios_bma180_data * buffer)
{
	if(fifoBuf_getUsed(&pios_bma180_fifo) < sizeof(*buffer))
		return -1;
	
	fifoBuf_getData(&pios_bma180_fifo, (uint8_t *) buffer, sizeof(*buffer));
	
	return 0;
}


/**
 * @brief Test SPI and chip functionality by reading chip ID register
 * @return 0 if success, -1 if failure.
 *
 */
int32_t PIOS_BMA180_Test()
{
	// Read chip ID then version ID
	uint8_t buf[3] = {0x80 | BMA_CHIPID_ADDR, 0, 0};
	uint8_t rec[3] = {0,0, 0};
	int32_t retval;

	if(PIOS_BMA180_ClaimBus() != 0)
		return -1;
	retval = PIOS_SPI_TransferBlock(PIOS_SPI_ACCEL,&buf[0],&rec[0],sizeof(buf),NULL);
	PIOS_BMA180_ReleaseBus();
	
	if(retval != 0)
		return -2;
	
	struct pios_bma180_data data;
	if(PIOS_BMA180_ReadAccels(&data) != 0)
		return -3;
	
	if(rec[1] != 0x3)
		return -4;
	
	if(rec[2] < 0x12)
		return -5;

	return 0;
}

static uint8_t pios_bma180_dmabuf[7];
static void PIOS_BMA180_SPI_Callback() 
{		
	// TODO: Make this conversion depend on configuration scale
	struct pios_bma180_data data;

	// Don't release bus till data has copied
	PIOS_BMA180_ReleaseBus();	
	
	// Must not return before releasing bus
	if(fifoBuf_getFree(&pios_bma180_fifo) < sizeof(data))
		return;
	
	// Bottom two bits indicate new data and are constant zeros.  Don't right 
	// shift because it drops sign bit
	data.x = ((pios_bma180_dmabuf[2] << 8) | pios_bma180_dmabuf[1]);
	data.y = ((pios_bma180_dmabuf[4] << 8) | pios_bma180_dmabuf[3]);
	data.z = ((pios_bma180_dmabuf[6] << 8) | pios_bma180_dmabuf[5]);
	data.x /= 4;
	data.y /= 4;
	data.z /= 4;
	
	fifoBuf_putData(&pios_bma180_fifo, (uint8_t *) &data, sizeof(data));
}

/**
 * @brief IRQ Handler
 */
const static uint8_t pios_bma180_req_buf[7] = {BMA_X_LSB_ADDR | 0x80,0,0,0,0,0};
static void PIOS_BMA180_IRQHandler(void)
{
	// If we can't get the bus then just move on for efficiency
	if(PIOS_BMA180_ClaimBus() == 0)
		PIOS_SPI_TransferBlock(PIOS_SPI_ACCEL,pios_bma180_req_buf,(uint8_t *) pios_bma180_dmabuf, 
							   7, PIOS_BMA180_SPI_Callback);	
}


/**
 * The physical IRQ handler
 * Soon this will be generic in pios_exti and the BMA180 will register
 * against it
 */
void EXTI4_IRQHandler(void)
{
	if (EXTI_GetITStatus(dev_cfg->eoc_exti.init.EXTI_Line) != RESET) {
		PIOS_BMA180_IRQHandler();
		EXTI_ClearITPendingBit(dev_cfg->eoc_exti.init.EXTI_Line);
	}
}


/**
 * @}
 * @}
 */
