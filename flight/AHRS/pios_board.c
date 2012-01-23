/**
 ******************************************************************************
 *
 * @file       pios_board.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Defines board specific static initializers for hardware for the AHRS board.
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

#define PIOS_COM_AUX_TX_BUF_LEN 192
static uint8_t pios_com_aux_tx_buffer[PIOS_COM_AUX_TX_BUF_LEN];

uint32_t pios_com_aux_id;
uint8_t adc_fifo_buf[sizeof(float) * 6 * 4] __attribute__ ((aligned(4)));    // align to 32-bit to try and provide speed improvement

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

#if defined(PIOS_LED_HEARTBEAT)
	PIOS_LED_On(PIOS_LED_HEARTBEAT);
#endif	/* PIOS_LED_HEARTBEAT */

	/* Delay system */
	PIOS_DELAY_Init();

	/* Communication system */
#if !defined(PIOS_ENABLE_DEBUG_PINS)
#if defined(PIOS_INCLUDE_COM)
	{
		uint32_t pios_usart_aux_id;
		if (PIOS_USART_Init(&pios_usart_aux_id, &pios_usart_aux_cfg)) {
			PIOS_DEBUG_Assert(0);
		}
		if (PIOS_COM_Init(&pios_com_aux_id, &pios_usart_com_driver, pios_usart_aux_id,
				  NULL, 0,
				  pios_com_aux_tx_buffer, sizeof(pios_com_aux_tx_buffer))) {
			PIOS_DEBUG_Assert(0);
		}
	}
#endif	/* PIOS_INCLUDE_COM */
#endif

	/* IAP System Setup */
	PIOS_IAP_Init();

	/* ADC system */
	PIOS_ADC_Init();
	extern uint8_t adc_oversampling;
	PIOS_ADC_Config(adc_oversampling);
	extern void adc_callback(float *);
	PIOS_ADC_SetCallback(adc_callback);

	/* ADC buffer */
	extern t_fifo_buffer adc_fifo_buffer;
	fifoBuf_init(&adc_fifo_buffer, adc_fifo_buf, sizeof(adc_fifo_buf));

	/* Setup the Accelerometer FS (Full-Scale) GPIO */
	PIOS_GPIO_Enable(0);
	SET_ACCEL_6G;

#if defined(PIOS_INCLUDE_HMC5843) && defined(PIOS_INCLUDE_I2C)
	/* Magnetic sensor system */
	if (PIOS_I2C_Init(&pios_i2c_main_adapter_id, &pios_i2c_main_adapter_cfg)) {
		PIOS_DEBUG_Assert(0);
	}
	PIOS_HMC5843_Init();
#endif

#if defined(PIOS_INCLUDE_SPI)
#include "ahrs_spi_comm.h"
	AhrsInitComms();

	/* Set up the SPI interface to the OP board */
	if (PIOS_SPI_Init(&pios_spi_op_id, &pios_spi_op_cfg)) {
		PIOS_DEBUG_Assert(0);
	}

	AhrsConnect(pios_spi_op_id);
#endif
}

