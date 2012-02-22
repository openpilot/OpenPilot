/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_HMC5883 HMC5883 Functions
 * @brief Deals with the hardware interface to the magnetometers
 * @{
 *
 * @file       pios_board.c
 * @author     David "Buzz" Carlson (buzz@chebuzz.com)
 * 				The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      Defines board specific static initializers for hardware for the INS board.
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

/* Pull in the board-specific static HW definitions.
 * Including .c files is a bit ugly but this allows all of
 * the HW definitions to be const and static to limit their
 * scope.  
 *
 * NOTE: THIS IS THE ONLY PLACE THAT SHOULD EVER INCLUDE THIS FILE
 */
#include "board_hw_defs.c"

#include <pios.h>

#if 0
#define PIOS_COM_AUX_TX_BUF_LEN 192
static uint8_t pios_com_aux_tx_buffer[PIOS_COM_AUX_TX_BUF_LEN];
#endif

#define PIOS_COM_GPS_RX_BUF_LEN 96
static uint8_t pios_com_gps_rx_buffer[PIOS_COM_GPS_RX_BUF_LEN];

uint32_t pios_com_aux_id;
uint32_t pios_com_gps_id;

/**
 * PIOS_Board_Init()
 * initializes all the core subsystems on this specific hardware
 * called from System/openpilot.c
 */
void PIOS_Board_Init(void) {
	/* Brings up System using CMSIS functions, enables the LEDs. */
	PIOS_SYS_Init();

#if defined(PIOS_INCLUDE_LED)
	PIOS_LED_Init(&pios_led_cfg);
#endif	/* PIOS_INCLUDE_LED */

	/* Delay system */
	PIOS_DELAY_Init();

	/* IAP System Setup */
	PIOS_IAP_Init();

#if defined(PIOS_INCLUDE_COM)
#if defined(PIOS_INCLUDE_GPS)
	uint32_t pios_usart_gps_id;
	if (PIOS_USART_Init(&pios_usart_gps_id, &pios_usart_gps_cfg)) {
		PIOS_DEBUG_Assert(0);
	}
	if (PIOS_COM_Init(&pios_com_gps_id, &pios_usart_com_driver, pios_usart_gps_id,
			  pios_com_gps_rx_buffer, sizeof(pios_com_gps_rx_buffer),
			  NULL, 0)) {
		PIOS_DEBUG_Assert(0);
	}
#endif	/* PIOS_INCLUDE_GPS */
#endif	/* PIOS_INCLUDE_COM */

#if defined (PIOS_INCLUDE_I2C)
	if (PIOS_I2C_Init(&pios_i2c_pres_mag_adapter_id, &pios_i2c_pres_mag_adapter_cfg)) {
		PIOS_DEBUG_Assert(0);
	}
#if defined (PIOS_INCLUDE_BMP085)
	PIOS_BMP085_Init();
#endif /* PIOS_INCLUDE_BMP085 */
#if defined (PIOS_INCLUDE_HMC5883)
	PIOS_HMC5883_Init();
#endif /* PIOS_INCLUDE_HMC5883 */

#if defined(PIOS_INCLUDE_IMU3000)
	if (PIOS_I2C_Init(&pios_i2c_gyro_adapter_id, &pios_i2c_gyro_adapter_cfg)) {
		PIOS_DEBUG_Assert(0);
	}
	PIOS_IMU3000_Init();
#endif /* PIOS_INCLUDE_IMU3000 */
#endif /* PIOS_INCLUDE_I2C */

#if defined(PIOS_INCLUDE_SPI)
	/* Set up the SPI interface to the accelerometer*/
	if (PIOS_SPI_Init(&pios_spi_accel_id, &pios_spi_accel_cfg)) {
		PIOS_DEBUG_Assert(0);
	}

	PIOS_BMA180_Attach(pios_spi_accel_id);

// #include "ahrs_spi_comm.h"
//	InsInitComms();
//
//	/* Set up the SPI interface to the OP board */
//	if (PIOS_SPI_Init(&pios_spi_op_id, &pios_spi_op_cfg)) {
//		PIOS_DEBUG_Assert(0);
//	}
//
//	InsConnect(pios_spi_op_id);
#endif /* PIOS_INCLUDE_SPI */
}

/**
 * @}
 * @}
 */

