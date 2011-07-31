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

#include <pios.h>

#if defined(PIOS_INCLUDE_SPI)

#include <pios_spi_priv.h>

/* OP Interface
 * 
 * NOTE: Leave this declared as const data so that it ends up in the 
 * .rodata section (ie. Flash) rather than in the .bss section (RAM).
 */
void PIOS_SPI_op_irq_handler(void);
void DMA1_Channel5_IRQHandler() __attribute__ ((alias("PIOS_SPI_op_irq_handler")));
void DMA1_Channel4_IRQHandler() __attribute__ ((alias("PIOS_SPI_op_irq_handler")));
static const struct pios_spi_cfg pios_spi_op_cfg = {
	.regs = SPI2,
	.init = {
		 .SPI_Mode = SPI_Mode_Slave,
		 .SPI_Direction = SPI_Direction_2Lines_FullDuplex,
		 .SPI_DataSize = SPI_DataSize_8b,
		 .SPI_NSS = SPI_NSS_Hard,
		 .SPI_FirstBit = SPI_FirstBit_MSB,
		 .SPI_CRCPolynomial = 7,
		 .SPI_CPOL = SPI_CPOL_High,
		 .SPI_CPHA = SPI_CPHA_2Edge,
		 },
	.use_crc = TRUE,
	.dma = {
		.ahb_clk = RCC_AHBPeriph_DMA1,

		.irq = {
			.flags =
			(DMA1_FLAG_TC4 | DMA1_FLAG_TE4 | DMA1_FLAG_HT4 |
			 DMA1_FLAG_GL4),
			.init = {
				 .NVIC_IRQChannel = DMA1_Channel4_IRQn,
				 .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
				 .NVIC_IRQChannelSubPriority = 0,
				 .NVIC_IRQChannelCmd = ENABLE,
				 },
			},

		.rx = {
		       .channel = DMA1_Channel4,
		       .init = {
				.DMA_PeripheralBaseAddr =
				(uint32_t) & (SPI2->DR),
				.DMA_DIR = DMA_DIR_PeripheralSRC,
				.DMA_PeripheralInc =
				DMA_PeripheralInc_Disable,
				.DMA_MemoryInc = DMA_MemoryInc_Enable,
				.DMA_PeripheralDataSize =
				DMA_PeripheralDataSize_Byte,
				.DMA_MemoryDataSize =
				DMA_MemoryDataSize_Byte,
				.DMA_Mode = DMA_Mode_Normal,
				.DMA_Priority = DMA_Priority_Medium,
				.DMA_M2M = DMA_M2M_Disable,
				},
		       },
		.tx = {
		       .channel = DMA1_Channel5,
		       .init = {
				.DMA_PeripheralBaseAddr =
				(uint32_t) & (SPI2->DR),
				.DMA_DIR = DMA_DIR_PeripheralDST,
				.DMA_PeripheralInc =
				DMA_PeripheralInc_Disable,
				.DMA_MemoryInc = DMA_MemoryInc_Enable,
				.DMA_PeripheralDataSize =
				DMA_PeripheralDataSize_Byte,
				.DMA_MemoryDataSize =
				DMA_MemoryDataSize_Byte,
				.DMA_Mode = DMA_Mode_Normal,
				.DMA_Priority = DMA_Priority_Medium,
				.DMA_M2M = DMA_M2M_Disable,
				},
		       },
		},
	.ssel = {
		 .gpio = GPIOB,
		 .init = {
			  .GPIO_Pin = GPIO_Pin_12,
			  .GPIO_Speed = GPIO_Speed_10MHz,
			  .GPIO_Mode = GPIO_Mode_IN_FLOATING,
			  },
		 },
	.sclk = {
		 .gpio = GPIOB,
		 .init = {
			  .GPIO_Pin = GPIO_Pin_13,
			  .GPIO_Speed = GPIO_Speed_10MHz,
			  .GPIO_Mode = GPIO_Mode_IN_FLOATING,
			  },
		 },
	.miso = {
		 .gpio = GPIOB,
		 .init = {
			  .GPIO_Pin = GPIO_Pin_14,
			  .GPIO_Speed = GPIO_Speed_10MHz,
			  .GPIO_Mode = GPIO_Mode_AF_PP,
			  },
		 },
	.mosi = {
		 .gpio = GPIOB,
		 .init = {
			  .GPIO_Pin = GPIO_Pin_15,
			  .GPIO_Speed = GPIO_Speed_10MHz,
			  .GPIO_Mode = GPIO_Mode_IN_FLOATING,
			  },
		 },
};

uint32_t pios_spi_op_id;
void PIOS_SPI_op_irq_handler(void)
{
	/* Call into the generic code to handle the IRQ for this specific device */
	PIOS_SPI_IRQ_Handler(pios_spi_op_id);
}

#endif /* PIOS_INCLUDE_SPI */

/*
 * ADC system
 */
#include "pios_adc_priv.h"
extern void PIOS_ADC_handler(void);
void DMA1_Channel1_IRQHandler() __attribute__ ((alias("PIOS_ADC_handler")));
// Remap the ADC DMA handler to this one
static const struct pios_adc_cfg pios_adc_cfg = {
	.dma = {
		.ahb_clk  = RCC_AHBPeriph_DMA1,
		.irq = {
			.flags   = (DMA1_FLAG_TC1 | DMA1_FLAG_TE1 | DMA1_FLAG_HT1 | DMA1_FLAG_GL1),
			.init    = {
				.NVIC_IRQChannel                   = DMA1_Channel1_IRQn,
				.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
				.NVIC_IRQChannelSubPriority        = 0,
				.NVIC_IRQChannelCmd                = ENABLE,
			},
		},
		.rx = {
			.channel = DMA1_Channel1,
			.init    = {
				.DMA_PeripheralBaseAddr = (uint32_t) & ADC1->DR,
				.DMA_DIR                = DMA_DIR_PeripheralSRC,
				.DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
				.DMA_MemoryInc          = DMA_MemoryInc_Enable,
				.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word,
				.DMA_MemoryDataSize     = DMA_MemoryDataSize_Word,
				.DMA_Mode               = DMA_Mode_Circular,
				.DMA_Priority           = DMA_Priority_High,
				.DMA_M2M                = DMA_M2M_Disable,
			},
		}
	}, 
	.half_flag = DMA1_IT_HT1,
	.full_flag = DMA1_IT_TC1,
};

struct pios_adc_dev pios_adc_devs[] = {
	{
		.cfg = &pios_adc_cfg,
		.callback_function = NULL,
	},
};

uint8_t pios_adc_num_devices = NELEMENTS(pios_adc_devs);

void PIOS_ADC_handler() {
	PIOS_ADC_DMA_Handler();
}


#if defined(PIOS_INCLUDE_USART)
#include <pios_usart_priv.h>

/*
 * AUX USART
 */
static const struct pios_usart_cfg pios_usart_aux_cfg = {
	.regs = USART3,
	.init = {
		 .USART_BaudRate = 230400,
		 .USART_WordLength = USART_WordLength_8b,
		 .USART_Parity = USART_Parity_No,
		 .USART_StopBits = USART_StopBits_1,
		 .USART_HardwareFlowControl =
		 USART_HardwareFlowControl_None,
		 .USART_Mode = USART_Mode_Rx | USART_Mode_Tx,
		 },
	.irq = {
		.init = {
			 .NVIC_IRQChannel = USART3_IRQn,
			 .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
			 .NVIC_IRQChannelSubPriority = 0,
			 .NVIC_IRQChannelCmd = ENABLE,
			 },
		},
	.rx = {
	       .gpio = GPIOB,
	       .init = {
			.GPIO_Pin = GPIO_Pin_11,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Mode = GPIO_Mode_IPU,
			},
	       },
	.tx = {
	       .gpio = GPIOB,
	       .init = {
			.GPIO_Pin = GPIO_Pin_10,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Mode = GPIO_Mode_AF_PP,
			},
	       },
};

#endif /* PIOS_INCLUDE_USART */

#if defined(PIOS_INCLUDE_COM)

#include <pios_com_priv.h>

#define PIOS_COM_AUX_TX_BUF_LEN 192
static uint8_t pios_com_aux_tx_buffer[PIOS_COM_AUX_TX_BUF_LEN];

#endif /* PIOS_INCLUDE_COM */

#if defined(PIOS_INCLUDE_I2C)

#include <pios_i2c_priv.h>

/*
 * I2C Adapters
 */

void PIOS_I2C_main_adapter_ev_irq_handler(void);
void PIOS_I2C_main_adapter_er_irq_handler(void);
void I2C1_EV_IRQHandler()
    __attribute__ ((alias("PIOS_I2C_main_adapter_ev_irq_handler")));
void I2C1_ER_IRQHandler()
    __attribute__ ((alias("PIOS_I2C_main_adapter_er_irq_handler")));

static const struct pios_i2c_adapter_cfg pios_i2c_main_adapter_cfg = {
	.regs = I2C1,
	.init = {
		 .I2C_Mode = I2C_Mode_I2C,
		 .I2C_OwnAddress1 = 0,
		 .I2C_Ack = I2C_Ack_Enable,
		 .I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit,
		 .I2C_DutyCycle = I2C_DutyCycle_2,
		 .I2C_ClockSpeed = 200000,	/* bits/s */
		 },
	.transfer_timeout_ms = 50,
	.scl = {
		.gpio = GPIOB,
		.init = {
			 .GPIO_Pin = GPIO_Pin_6,
			 .GPIO_Speed = GPIO_Speed_10MHz,
			 .GPIO_Mode = GPIO_Mode_AF_OD,
			 },
		},
	.sda = {
		.gpio = GPIOB,
		.init = {
			 .GPIO_Pin = GPIO_Pin_7,
			 .GPIO_Speed = GPIO_Speed_10MHz,
			 .GPIO_Mode = GPIO_Mode_AF_OD,
			 },
		},
	.event = {
		  .flags = 0,	/* FIXME: check this */
		  .init = {
			   .NVIC_IRQChannel = I2C1_EV_IRQn,
			   .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGHEST,
			   .NVIC_IRQChannelSubPriority = 0,
			   .NVIC_IRQChannelCmd = ENABLE,
			   },
		  },
	.error = {
		  .flags = 0,	/* FIXME: check this */
		  .init = {
			   .NVIC_IRQChannel = I2C1_ER_IRQn,
			   .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGHEST,
			   .NVIC_IRQChannelSubPriority = 0,
			   .NVIC_IRQChannelCmd = ENABLE,
			   },
		  },
};

uint32_t pios_i2c_main_adapter_id;
void PIOS_I2C_main_adapter_ev_irq_handler(void)
{
	/* Call into the generic code to handle the IRQ for this specific device */
	PIOS_I2C_EV_IRQ_Handler(pios_i2c_main_adapter_id);
}

void PIOS_I2C_main_adapter_er_irq_handler(void)
{
	/* Call into the generic code to handle the IRQ for this specific device */
	PIOS_I2C_ER_IRQ_Handler(pios_i2c_main_adapter_id);
}

#endif /* PIOS_INCLUDE_I2C */

#if defined(PIOS_ENABLE_DEBUG_PINS)

static const struct stm32_gpio pios_debug_pins[] = {
  {
    .gpio = GPIOB,
    .init = {
      .GPIO_Pin   = GPIO_Pin_11,
      .GPIO_Speed = GPIO_Speed_50MHz,
      .GPIO_Mode  = GPIO_Mode_IN_FLOATING,
    },
  },
  {
    .gpio = GPIOB,
    .init = {
      .GPIO_Pin   = GPIO_Pin_10,
      .GPIO_Speed = GPIO_Speed_50MHz,
      .GPIO_Mode  = GPIO_Mode_Out_PP,
    },
  },
};

#endif /* PIOS_ENABLE_DEBUG_PINS */

extern const struct pios_com_driver pios_usart_com_driver;

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

	PIOS_LED_On(LED1);

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

