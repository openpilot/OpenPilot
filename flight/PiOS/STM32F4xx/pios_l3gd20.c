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
	PIOS_SPI_SetPrescalar(pios_spi_gyro, SPI_BaudRatePrescaler_256);
	PIOS_L3GD20_Config(cfg);
	PIOS_SPI_SetPrescalar(pios_spi_gyro, SPI_BaudRatePrescaler_8);

	/* Configure EOC pin as input floating */
	GPIO_Init(cfg->drdy.gpio, &cfg->drdy.init);
	
	/* Configure the End Of Conversion (EOC) interrupt */
	SYSCFG_EXTILineConfig(cfg->eoc_exti.port_source, cfg->eoc_exti.pin_source);
	EXTI_Init(&cfg->eoc_exti.init);
	
	/* Enable and set EOC EXTI Interrupt to the lowest priority */
	NVIC_Init(&cfg->eoc_irq.init);

}

/**
 * @brief Initialize the L3GD20 3-axis gyro sensor
 * \return none
 * \param[in] PIOS_L3GD20_ConfigTypeDef struct to be used to configure sensor.
*
*/
static void PIOS_L3GD20_Config(struct pios_l3gd20_cfg const * cfg)
{

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
int32_t PIOS_L3GD20_ReadGyros(struct pios_l3gd20_data * data)
{
	uint8_t buf[7] = {PIOS_L3GD20_GYRO_X_OUT_MSB | 0x80, 0, 0, 0, 0, 0, 0};
	uint8_t rec[7];
	
	if(PIOS_L3GD20_ClaimBus() != 0)
		return -1;

	if(PIOS_SPI_TransferBlock(pios_spi_gyro, &buf[0], &rec[0], sizeof(buf), NULL) < 0)
		return -2;
		
	PIOS_L3GD20_ReleaseBus();
	
	data->gyro_x = rec[1] << 8 | rec[2];
	data->gyro_y = rec[3] << 8 | rec[4];
	data->gyro_z = rec[5] << 8 | rec[6];
	
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
	switch (cfg->gyro_range) {
		case PIOS_L3GD20_SCALE_250_DEG:
			return 1.0f / 131.0f;
		case PIOS_L3GD20_SCALE_500_DEG:
			return 1.0f / 65.5f;
		case PIOS_L3GD20_SCALE_1000_DEG:
			return 1.0f / 32.8f;
		case PIOS_L3GD20_SCALE_2000_DEG:
			return 1.0f / 16.4f;
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
 * @brief Run self-test operation.
 * \return 0 if test succeeded
 * \return non-zero value if test succeeded
 */
static int32_t PIOS_L3GD20_FifoDepth(void)
{
	uint8_t L3GD20_send_buf[3] = {PIOS_L3GD20_FIFO_CNT_MSB | 0x80, 0, 0};
	uint8_t L3GD20_rec_buf[3];

	if(PIOS_L3GD20_ClaimBus() != 0)
		return -1;

	if(PIOS_SPI_TransferBlock(pios_spi_gyro, &L3GD20_send_buf[0], &L3GD20_rec_buf[0], sizeof(L3GD20_send_buf), NULL) < 0) {
		PIOS_L3GD20_ReleaseBus();
		return -1;
	}

	PIOS_L3GD20_ReleaseBus();
	
	return (L3GD20_rec_buf[1] << 8) | L3GD20_rec_buf[2];
}

/**
* @brief IRQ Handler.  Read all the data from onboard buffer
*/
uint32_t l3gd20_irq = 0;
int32_t l3gd20_count;
uint32_t l3gd20_fifo_full = 0;

uint8_t l3gd20_last_read_count = 0;
uint32_t l3gd20_fails = 0;

uint32_t l3gd20_interval_us;
uint32_t l3gd20_time_us;
uint32_t l3gd20_transfer_size;

void PIOS_L3GD20_IRQHandler(void)
{
	static uint32_t timeval;
	l3gd20_interval_us = PIOS_DELAY_DiffuS(timeval);
	timeval = PIOS_DELAY_GetRaw();

	if(!l3gd20_configured)
		return;

	l3gd20_count = PIOS_L3GD20_FifoDepth();
	if(l3gd20_count < sizeof(struct pios_l3gd20_data))
		return;
		
	if(PIOS_L3GD20_ClaimBus() != 0)
		return;		
		
	uint8_t l3gd20_send_buf[1+sizeof(struct pios_l3gd20_data)] = {PIOS_L3GD20_FIFO_REG | 0x80, 0, 0, 0, 0, 0, 0, 0, 0};
	uint8_t l3gd20_rec_buf[1+sizeof(struct pios_l3gd20_data)];
	
	if(PIOS_SPI_TransferBlock(pios_spi_gyro, &l3gd20_send_buf[0], &l3gd20_rec_buf[0], sizeof(l3gd20_send_buf), NULL) < 0) {
		PIOS_L3GD20_ReleaseBus();
		l3gd20_fails++;
		return;
	}

	PIOS_L3GD20_ReleaseBus();
	
	struct pios_l3gd20_data data;
	
	if(fifoBuf_getFree(&pios_l3gd20_fifo) < sizeof(data)) {
		l3gd20_fifo_full++;
		return;			
	}
	
	// In the case where extras samples backed up grabbed an extra
	if (l3gd20_count >= (sizeof(data) * 2)) {
		if(PIOS_L3GD20_ClaimBus() != 0)
			return;		
		
		uint8_t l3gd20_send_buf[1+sizeof(struct pios_l3gd20_data)] = {PIOS_L3GD20_FIFO_REG | 0x80, 0, 0, 0, 0, 0, 0, 0, 0};
		uint8_t l3gd20_rec_buf[1+sizeof(struct pios_l3gd20_data)];
		
		if(PIOS_SPI_TransferBlock(pios_spi_gyro, &l3gd20_send_buf[0], &l3gd20_rec_buf[0], sizeof(l3gd20_send_buf), NULL) < 0) {
			PIOS_L3GD20_ReleaseBus();
			l3gd20_fails++;
			return;
		}
		
		PIOS_L3GD20_ReleaseBus();
		
		struct pios_l3gd20_data data;
		
		if(fifoBuf_getFree(&pios_l3gd20_fifo) < sizeof(data)) {
			l3gd20_fifo_full++;
			return;			
		}

	}
	
	data.temperature = l3gd20_rec_buf[1] << 8 | l3gd20_rec_buf[2];
	data.gyro_x = l3gd20_rec_buf[3] << 8 | l3gd20_rec_buf[4];
	data.gyro_y = l3gd20_rec_buf[5] << 8 | l3gd20_rec_buf[6];
	data.gyro_z = l3gd20_rec_buf[7] << 8 | l3gd20_rec_buf[8];

	fifoBuf_putData(&pios_l3gd20_fifo, (uint8_t *) &data, sizeof(data));
	l3gd20_irq++;
	
	l3gd20_time_us = PIOS_DELAY_DiffuS(timeval);
}

#endif /* L3GD20 */

/**
 * @}
 * @}
 */
