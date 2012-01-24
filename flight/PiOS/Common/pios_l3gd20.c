/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_L3GD20 L3GD20 Functions
 * @brief Deals with the hardware interface to the 3-axis gyro
 * @{
 *
 * @file       pios_l3gd20.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      L3GD20 3-axis gyro chip
 * @see        The GNU Public License (GPL) Version 3
 *
 ******************************************************************************
 */
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

#if defined(PIOS_INCLUDE_L3GD20)

#include "fifo_buffer.h"

/* Global Variables */
uint32_t pios_spi_gyro;

/* Local Variables */
#define DEG_TO_RAD (M_PI / 180.0)
static void PIOS_L3GD20_Config(struct pios_l3gd20_cfg const * cfg);
static int32_t PIOS_L3GD20_SetReg(uint8_t address, uint8_t buffer);
static int32_t PIOS_L3GD20_GetReg(uint8_t address);

#define PIOS_L3GD20_MAX_DOWNSAMPLE 100
static int16_t pios_l3gd20_buffer[PIOS_L3GD20_MAX_DOWNSAMPLE * sizeof(struct pios_l3gd20_data)];
static t_fifo_buffer pios_l3gd20_fifo;

volatile bool l3gd20_configured = false;

static struct pios_l3gd20_cfg const * cfg;

#define GRAV 9.81f

/**
 * @brief Initialize the MPU6050 3-axis gyro sensor.
 * @return none
 */
void PIOS_L3GD20_Init(const struct pios_l3gd20_cfg * new_cfg)
{
	cfg = new_cfg;
	
	fifoBuf_init(&pios_l3gd20_fifo, (uint8_t *) pios_l3gd20_buffer, sizeof(pios_l3gd20_buffer));

	/* Configure the MPU6050 Sensor */
	PIOS_SPI_SetClockSpeed(pios_spi_gyro, SPI_BaudRatePrescaler_256);
	PIOS_L3GD20_Config(cfg);
	PIOS_SPI_SetClockSpeed(pios_spi_gyro, SPI_BaudRatePrescaler_16);

	/* Set up EXTI */
	PIOS_EXTI_Init(new_cfg->exti_cfg);
	
	// An initial read is needed to get it running
	struct pios_l3gd20_data data;
	PIOS_L3GD20_ReadGyros(&data);

}

/**
 * @brief Initialize the L3GD20 3-axis gyro sensor
 * \return none
 * \param[in] PIOS_L3GD20_ConfigTypeDef struct to be used to configure sensor.
*
*/
static void PIOS_L3GD20_Config(struct pios_l3gd20_cfg const * cfg)
{
	// This register enables the channels and sets the bandwidth
	PIOS_L3GD20_SetReg(PIOS_L3GD20_CTRL_REG1, PIOS_L3GD20_CTRL1_FASTEST |
					   PIOS_L3GD20_CTRL1_PD | PIOS_L3GD20_CTRL1_ZEN |
					   PIOS_L3GD20_CTRL1_YEN | PIOS_L3GD20_CTRL1_XEN);
					   
	// Disable the high pass filters
	PIOS_L3GD20_SetReg(PIOS_L3GD20_CTRL_REG2, 0);
	
	// Set int2 to go high on data ready
	PIOS_L3GD20_SetReg(PIOS_L3GD20_CTRL_REG3, 0x08);
	
	// Select SPI interface, 500 deg/s, endianness?
	PIOS_L3GD20_SetReg(PIOS_L3GD20_CTRL_REG4, 0x10);

	// Enable FIFO, disable HPF
	PIOS_L3GD20_SetReg(PIOS_L3GD20_CTRL_REG5, 0x40);
	
	// Fifo stream mode
	PIOS_L3GD20_SetReg(PIOS_L3GD20_FIFO_CTRL_REG, 0x40);
}

/**
 * @brief Claim the SPI bus for the accel communications and select this chip
 * @return 0 if successful, -1 if unable to claim bus
 */
int32_t PIOS_L3GD20_ClaimBus()
{
	if(PIOS_SPI_ClaimBus(pios_spi_gyro) != 0)
		return -1;
	PIOS_SPI_RC_PinSet(pios_spi_gyro,0,0);
	return 0;
}

/**
 * @brief Release the SPI bus for the accel communications and end the transaction
 * @return 0 if successful
 */
int32_t PIOS_L3GD20_ReleaseBus()
{
	PIOS_SPI_RC_PinSet(pios_spi_gyro,0,1);
	return PIOS_SPI_ReleaseBus(pios_spi_gyro);
}

/**
 * @brief Connect to the correct SPI bus
 */
void PIOS_L3GD20_Attach(uint32_t spi_id)
{
	pios_spi_gyro = spi_id;
}

/**
 * @brief Read a register from L3GD20
 * @returns The register value or -1 if failure to get bus
 * @param reg[in] Register address to be read
 */
static int32_t PIOS_L3GD20_GetReg(uint8_t reg)
{
	uint8_t data;
	
	if(PIOS_L3GD20_ClaimBus() != 0)
		return -1;	
	
	PIOS_SPI_TransferByte(pios_spi_gyro,(0x80 | reg) ); // request byte
	data = PIOS_SPI_TransferByte(pios_spi_gyro,0 );     // receive response
	
	PIOS_L3GD20_ReleaseBus();
	return data;
}

/**
 * @brief Writes one byte to the L3GD20
 * \param[in] reg Register address
 * \param[in] data Byte to write
 * \return 0 if operation was successful
 * \return -1 if unable to claim SPI bus
 * \return -2 if unable to claim i2c device
 */
static int32_t PIOS_L3GD20_SetReg(uint8_t reg, uint8_t data)
{
	if(PIOS_L3GD20_ClaimBus() != 0)
		return -1;
	
	if(PIOS_SPI_TransferByte(pios_spi_gyro, 0x7f & reg) != 0) {
		PIOS_L3GD20_ReleaseBus();
		return -2;
	}
	
	if(PIOS_SPI_TransferByte(pios_spi_gyro, data) != 0) {
		PIOS_L3GD20_ReleaseBus();
		return -3;
	}
	
	PIOS_L3GD20_ReleaseBus();
	
	return 0;
}

/**
 * @brief Read current X, Z, Y values (in that order)
 * \param[out] int16_t array of size 3 to store X, Z, and Y magnetometer readings
 * \returns The number of samples remaining in the fifo
 */
uint32_t l3gd20_irq = 0;
int32_t PIOS_L3GD20_ReadGyros(struct pios_l3gd20_data * data)
{

	uint8_t buf[7] = {PIOS_L3GD20_GYRO_X_OUT_LSB | 0x80 | 0x40, 0, 0, 0, 0, 0, 0};
	uint8_t rec[7];
	
	if(PIOS_L3GD20_ClaimBus() != 0)
		return -1;

	if(PIOS_SPI_TransferBlock(pios_spi_gyro, &buf[0], &rec[0], sizeof(buf), NULL) < 0) {
		PIOS_L3GD20_ReleaseBus();
		data->gyro_x = 0;
		data->gyro_y = 0;
		data->gyro_z = 0;
		data->temperature = 0;
		return -2;
	}
		
	PIOS_L3GD20_ReleaseBus();
	
	memcpy((uint8_t *) &(data->gyro_x), &rec[1], 6);
	data->temperature = PIOS_L3GD20_GetReg(PIOS_L3GD20_OUT_TEMP);
	
	return 0;
}

/**
 * @brief Read the identification bytes from the MPU6050 sensor
 * \return ID read from MPU6050 or -1 if failure
*/
int32_t PIOS_L3GD20_ReadID()
{
	int32_t l3gd20_id = PIOS_L3GD20_GetReg(PIOS_L3GD20_WHOAMI);
	if(l3gd20_id < 0)
		return -1;
	return l3gd20_id;
}

/**
 * \brief Reads the data from the MPU6050 FIFO
 * \param[out] buffer destination buffer
 * \param[in] len maximum number of bytes which should be read
 * \note This returns the data as X, Y, Z the temperature
 * \return number of bytes transferred if operation was successful
 * \return -1 if error during I2C transfer
 */
int32_t PIOS_L3GD20_ReadFifo(struct pios_l3gd20_data * buffer)
{
	if(fifoBuf_getUsed(&pios_l3gd20_fifo) < sizeof(*buffer))
		return -1;
		
	fifoBuf_getData(&pios_l3gd20_fifo, (uint8_t *) buffer, sizeof(*buffer));
	
	return 0;
}



float PIOS_L3GD20_GetScale() 
{
	return 0.01750f;
	switch (cfg->gyro_range) {
		case PIOS_L3GD20_SCALE_250_DEG:
			return 0.00875f;
		case PIOS_L3GD20_SCALE_500_DEG:
			return 0.01750f;
		case PIOS_L3GD20_SCALE_2000_DEG:
			return 0.070f;
	}
	return 0;
}

/**
 * @brief Run self-test operation.
 * \return 0 if test succeeded
 * \return non-zero value if test succeeded
 */
uint8_t PIOS_L3GD20_Test(void)
{
	int32_t l3gd20_id = PIOS_L3GD20_ReadID();
	if(l3gd20_id < 0)
		return -1;

	uint8_t id = l3gd20_id;
	if(id == 0xD4)
		return 0;

	return -2;
}

/**
* @brief IRQ Handler.  Read all the data from onboard buffer
*/
int32_t l3gd20_count;
uint32_t l3gd20_fifo_full = 0;

void PIOS_L3GD20_IRQHandler(void)
{
	struct pios_l3gd20_data data;
	PIOS_L3GD20_ReadGyros(&data);
	
	data.temperature = l3gd20_irq;
	
	if(fifoBuf_getFree(&pios_l3gd20_fifo) < sizeof(data)) {
		l3gd20_fifo_full++;
		return;	
	}
	
	fifoBuf_putData(&pios_l3gd20_fifo, (uint8_t *) &data, sizeof(data));

	l3gd20_irq++;
	
}

#endif /* L3GD20 */

/**
 * @}
 * @}
 */
