/**
 ******************************************************************************
 * @addtogroup OpenPilotSystem OpenPilot System
 * @{
 * @addtogroup OpenPilotCore OpenPilot Core
 * @{
 *
 * @file       pios_board.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Defines board specific static initializers for hardware for the OpenPilot board.
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
#include <openpilot.h>
#include <uavobjectsinit.h>
#include <hwsettings.h>
#include <manualcontrolsettings.h>
#include <gcsreceiver.h>

#if defined(PIOS_INCLUDE_SPI)

#include <pios_spi_priv.h>

/* Gyro interface */
void PIOS_SPI_gyro_irq_handler(void);
void DMA1_Channel2_IRQHandler() __attribute__ ((alias ("PIOS_SPI_gyro_irq_handler")));
void DMA1_Channel3_IRQHandler() __attribute__ ((alias ("PIOS_SPI_gyro_irq_handler")));
static const struct pios_spi_cfg pios_spi_gyro_cfg = {
	.regs   = SPI1,
	.init   = {
		.SPI_Mode              = SPI_Mode_Master,
		.SPI_Direction         = SPI_Direction_2Lines_FullDuplex,
		.SPI_DataSize          = SPI_DataSize_8b,
		.SPI_NSS               = SPI_NSS_Soft,
		.SPI_FirstBit          = SPI_FirstBit_MSB,
		.SPI_CRCPolynomial     = 7,
		.SPI_CPOL              = SPI_CPOL_High,
		.SPI_CPHA              = SPI_CPHA_2Edge,
		.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8, /* 10 Mhz */
	},
	.dma = {
		.ahb_clk  = RCC_AHBPeriph_DMA1,
		
		.irq = {
			.flags   = (DMA1_FLAG_TC2 | DMA1_FLAG_TE2 | DMA1_FLAG_HT2 | DMA1_FLAG_GL2),
			.init    = {
				.NVIC_IRQChannel                   = DMA1_Channel2_IRQn,
				.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
				.NVIC_IRQChannelSubPriority        = 0,
				.NVIC_IRQChannelCmd                = ENABLE,
			},
		},
		
		.rx = {
			.channel = DMA1_Channel2,
			.init    = {
				.DMA_PeripheralBaseAddr = (uint32_t)&(SPI1->DR),
				.DMA_DIR                = DMA_DIR_PeripheralSRC,
				.DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
				.DMA_MemoryInc          = DMA_MemoryInc_Enable,
				.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
				.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
				.DMA_Mode               = DMA_Mode_Normal,
				.DMA_Priority           = DMA_Priority_Medium,
				.DMA_M2M                = DMA_M2M_Disable,
			},
		},
		.tx = {
			.channel = DMA1_Channel3,
			.init    = {
				.DMA_PeripheralBaseAddr = (uint32_t)&(SPI1->DR),
				.DMA_DIR                = DMA_DIR_PeripheralDST,
				.DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
				.DMA_MemoryInc          = DMA_MemoryInc_Enable,
				.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
				.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
				.DMA_Mode               = DMA_Mode_Normal,
				.DMA_Priority           = DMA_Priority_Medium,
				.DMA_M2M                = DMA_M2M_Disable,
			},
		},
	},
	.sclk = {
		.gpio = GPIOA,
		.init = {
			.GPIO_Pin   = GPIO_Pin_5,
			.GPIO_Speed = GPIO_Speed_50MHz,
			.GPIO_Mode  = GPIO_Mode_AF_PP,
		},
	},
	.miso = {
		.gpio = GPIOA,
		.init = {
			.GPIO_Pin   = GPIO_Pin_6,
			.GPIO_Speed = GPIO_Speed_50MHz,
			.GPIO_Mode  = GPIO_Mode_IPU,
		},
	},
	.mosi = {
		.gpio = GPIOA,
		.init = {
			.GPIO_Pin   = GPIO_Pin_7,
			.GPIO_Speed = GPIO_Speed_50MHz,
			.GPIO_Mode  = GPIO_Mode_AF_PP,
		},
	},
	.slave_count = 1,
	.ssel = {{
		.gpio = GPIOA,
		.init = {
			.GPIO_Pin   = GPIO_Pin_4,
			.GPIO_Speed = GPIO_Speed_50MHz,
			.GPIO_Mode  = GPIO_Mode_Out_PP,
		},
	}},	
};

static uint32_t pios_spi_gyro_id;
void PIOS_SPI_gyro_irq_handler(void)
{
	/* Call into the generic code to handle the IRQ for this specific device */
	PIOS_SPI_IRQ_Handler(pios_spi_gyro_id);
}


/* Flash/Accel Interface
 * 
 * NOTE: Leave this declared as const data so that it ends up in the 
 * .rodata section (ie. Flash) rather than in the .bss section (RAM).
 */
void PIOS_SPI_flash_accel_irq_handler(void);
void DMA1_Channel4_IRQHandler() __attribute__ ((alias ("PIOS_SPI_flash_accel_irq_handler")));
void DMA1_Channel5_IRQHandler() __attribute__ ((alias ("PIOS_SPI_flash_accel_irq_handler")));
static const struct pios_spi_cfg pios_spi_flash_accel_cfg = {
	.regs   = SPI2,
	.init   = {
		.SPI_Mode              = SPI_Mode_Master,
		.SPI_Direction         = SPI_Direction_2Lines_FullDuplex,
		.SPI_DataSize          = SPI_DataSize_8b,
		.SPI_NSS               = SPI_NSS_Soft,
		.SPI_FirstBit          = SPI_FirstBit_MSB,
		.SPI_CRCPolynomial     = 7,
		.SPI_CPOL              = SPI_CPOL_High,
		.SPI_CPHA              = SPI_CPHA_2Edge,
		.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2, 
	},
	.use_crc = FALSE,
	.dma = {
		.ahb_clk  = RCC_AHBPeriph_DMA1,
		
		.irq = {
			.flags   = (DMA1_FLAG_TC4 | DMA1_FLAG_TE4 | DMA1_FLAG_HT4 | DMA1_FLAG_GL4),
			.init    = {
				.NVIC_IRQChannel                   = DMA1_Channel4_IRQn,
				.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
				.NVIC_IRQChannelSubPriority        = 0,
				.NVIC_IRQChannelCmd                = ENABLE,
			},
		},
		
		.rx = {
			.channel = DMA1_Channel4,
			.init    = {
				.DMA_PeripheralBaseAddr = (uint32_t)&(SPI2->DR),
				.DMA_DIR                = DMA_DIR_PeripheralSRC,
				.DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
				.DMA_MemoryInc          = DMA_MemoryInc_Enable,
				.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
				.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
				.DMA_Mode               = DMA_Mode_Normal,
				.DMA_Priority           = DMA_Priority_High,
				.DMA_M2M                = DMA_M2M_Disable,
			},
		},
		.tx = {
			.channel = DMA1_Channel5,
			.init    = {
				.DMA_PeripheralBaseAddr = (uint32_t)&(SPI2->DR),
				.DMA_DIR                = DMA_DIR_PeripheralDST,
				.DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
				.DMA_MemoryInc          = DMA_MemoryInc_Enable,
				.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
				.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
				.DMA_Mode               = DMA_Mode_Normal,
				.DMA_Priority           = DMA_Priority_High,
				.DMA_M2M                = DMA_M2M_Disable,
			},
		},
	},
	.sclk = {
		.gpio = GPIOB,
		.init = {
			.GPIO_Pin   = GPIO_Pin_13,
			.GPIO_Speed = GPIO_Speed_10MHz,
			.GPIO_Mode  = GPIO_Mode_AF_PP,
		},
	},
	.miso = {
		.gpio = GPIOB,
		.init = {
			.GPIO_Pin   = GPIO_Pin_14,
			.GPIO_Speed = GPIO_Speed_10MHz,
			.GPIO_Mode  = GPIO_Mode_IN_FLOATING,
		},
	},
	.mosi = {
		.gpio = GPIOB,
		.init = {
			.GPIO_Pin   = GPIO_Pin_15,
			.GPIO_Speed = GPIO_Speed_10MHz,
			.GPIO_Mode  = GPIO_Mode_AF_PP,
		},
	},
	.slave_count = 2,
	.ssel = {{
		.gpio = GPIOB,
		.init = {
			.GPIO_Pin   = GPIO_Pin_12,
			.GPIO_Speed = GPIO_Speed_50MHz,
			.GPIO_Mode  = GPIO_Mode_Out_PP,
		}},{
			.gpio = GPIOA,
			.init = {
				.GPIO_Pin   = GPIO_Pin_7,
				.GPIO_Speed = GPIO_Speed_50MHz,
				.GPIO_Mode  = GPIO_Mode_Out_PP,
			},
		}},
};

static uint32_t pios_spi_flash_accel_id;
void PIOS_SPI_flash_accel_irq_handler(void)
{
  /* Call into the generic code to handle the IRQ for this specific device */
	PIOS_SPI_IRQ_Handler(pios_spi_flash_accel_id);
}

#endif	/* PIOS_INCLUDE_SPI */

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

#include "pios_tim_priv.h"

static const TIM_TimeBaseInitTypeDef tim_1_2_3_4_time_base = {
	.TIM_Prescaler = (PIOS_MASTER_CLOCK / 1000000) - 1,
	.TIM_ClockDivision = TIM_CKD_DIV1,
	.TIM_CounterMode = TIM_CounterMode_Up,
	.TIM_Period = ((1000000 / PIOS_SERVO_UPDATE_HZ) - 1),
	.TIM_RepetitionCounter = 0x0000,
};

static const struct pios_tim_clock_cfg tim_1_cfg = {
	.timer = TIM1,
	.time_base_init = &tim_1_2_3_4_time_base,
	.irq = {
		.init = {
			.NVIC_IRQChannel                   = TIM1_CC_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
			.NVIC_IRQChannelSubPriority        = 0,
			.NVIC_IRQChannelCmd                = ENABLE,
		},
	},
};

static const struct pios_tim_clock_cfg tim_2_cfg = {
	.timer = TIM2,
	.time_base_init = &tim_1_2_3_4_time_base,
	.irq = {
		.init = {
			.NVIC_IRQChannel                   = TIM2_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
			.NVIC_IRQChannelSubPriority        = 0,
			.NVIC_IRQChannelCmd                = ENABLE,
		},
	},
};

static const struct pios_tim_clock_cfg tim_3_cfg = {
	.timer = TIM3,
	.time_base_init = &tim_1_2_3_4_time_base,
	.irq = {
		.init = {
			.NVIC_IRQChannel                   = TIM3_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
			.NVIC_IRQChannelSubPriority        = 0,
			.NVIC_IRQChannelCmd                = ENABLE,
		},
	},
};

static const struct pios_tim_clock_cfg tim_4_cfg = {
	.timer = TIM4,
	.time_base_init = &tim_1_2_3_4_time_base,
	.irq = {
		.init = {
			.NVIC_IRQChannel                   = TIM4_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
			.NVIC_IRQChannelSubPriority        = 0,
			.NVIC_IRQChannelCmd                = ENABLE,
		},
	},
};

static const struct pios_tim_channel pios_tim_rcvrport_all_channels[] = {
	{
		.timer = TIM4,
		.timer_chan = TIM_Channel_1,
		.pin = {
			.gpio = GPIOB,
			.init = {
				.GPIO_Pin   = GPIO_Pin_6,
				.GPIO_Mode  = GPIO_Mode_IPD,
				.GPIO_Speed = GPIO_Speed_2MHz,
			},
		},
	},
	{
		.timer = TIM3,
		.timer_chan = TIM_Channel_2,
		.pin = {
			.gpio = GPIOB,
			.init = {
				.GPIO_Pin   = GPIO_Pin_5,
				.GPIO_Mode  = GPIO_Mode_IPD,
				.GPIO_Speed = GPIO_Speed_2MHz,
			},
		},
		.remap = GPIO_PartialRemap_TIM3,
	},
	{
		.timer = TIM3,
		.timer_chan = TIM_Channel_3,
		.pin = {
			.gpio = GPIOB,
			.init = {
				.GPIO_Pin   = GPIO_Pin_0,
				.GPIO_Mode  = GPIO_Mode_IPD,
				.GPIO_Speed = GPIO_Speed_2MHz,
			},
		},
	}, 
	{
		.timer = TIM3,
		.timer_chan = TIM_Channel_4,
		.pin = {
			.gpio = GPIOB,
			.init = {
				.GPIO_Pin   = GPIO_Pin_1,
				.GPIO_Mode  = GPIO_Mode_IPD,
				.GPIO_Speed = GPIO_Speed_2MHz,
			},
		},
	}, 
	{ 
		.timer = TIM2,
		.timer_chan = TIM_Channel_1,
		.pin = {
			.gpio = GPIOA,
			.init = {
				.GPIO_Pin   = GPIO_Pin_0,
				.GPIO_Mode  = GPIO_Mode_IPD,
				.GPIO_Speed = GPIO_Speed_2MHz,
			},
		},
	},  	
	{
		.timer = TIM2,
		.timer_chan = TIM_Channel_2,
		.pin = {
			.gpio = GPIOA,
			.init = {
				.GPIO_Pin   = GPIO_Pin_1,
				.GPIO_Mode  = GPIO_Mode_IPD,
				.GPIO_Speed = GPIO_Speed_2MHz,
			},
		},
	}, 		
};

static const struct pios_tim_channel pios_tim_servoport_all_pins[] = {
	{
		.timer = TIM4,
		.timer_chan = TIM_Channel_4,
		.pin = {
			.gpio = GPIOB,
			.init = {
				.GPIO_Pin   = GPIO_Pin_9,
				.GPIO_Mode  = GPIO_Mode_AF_PP,
				.GPIO_Speed = GPIO_Speed_2MHz,
			},
		},
	},
	{
		.timer = TIM4,
		.timer_chan = TIM_Channel_3,
		.pin = {
			.gpio = GPIOB,
			.init = {
				.GPIO_Pin   = GPIO_Pin_8,
				.GPIO_Mode  = GPIO_Mode_AF_PP,
				.GPIO_Speed = GPIO_Speed_2MHz,
			},
		},
	}, 
	{
		.timer = TIM4,
		.timer_chan = TIM_Channel_2,
		.pin = {
			.gpio = GPIOB,
			.init = {
				.GPIO_Pin   = GPIO_Pin_7,
				.GPIO_Mode  = GPIO_Mode_AF_PP,
				.GPIO_Speed = GPIO_Speed_2MHz,
			},
		},
	}, 
	{
		.timer = TIM1,
		.timer_chan = TIM_Channel_1,
		.pin = {
			.gpio = GPIOA,
			.init = {
				.GPIO_Pin   = GPIO_Pin_8,
				.GPIO_Mode  = GPIO_Mode_AF_PP,
				.GPIO_Speed = GPIO_Speed_2MHz,
			},
		},
	}, 
	{
		.timer = TIM3,
		.timer_chan = TIM_Channel_1,
		.pin = {
			.gpio = GPIOB,
			.init = {
				.GPIO_Pin   = GPIO_Pin_4,
				.GPIO_Mode  = GPIO_Mode_AF_PP,
				.GPIO_Speed = GPIO_Speed_2MHz,
			},
		},
		.remap = GPIO_PartialRemap_TIM3,
	},  	
	{
		.timer = TIM2,
		.timer_chan = TIM_Channel_3,
		.pin = {
			.gpio = GPIOA,
			.init = {
				.GPIO_Pin   = GPIO_Pin_2,
				.GPIO_Mode  = GPIO_Mode_AF_PP,
				.GPIO_Speed = GPIO_Speed_2MHz,
			},
		},
	},
};


static const struct pios_tim_channel pios_tim_servoport_rcvrport_pins[] = {
	{
		.timer = TIM4,
		.timer_chan = TIM_Channel_4,
		.pin = {
			.gpio = GPIOB,
			.init = {
				.GPIO_Pin   = GPIO_Pin_9,
				.GPIO_Mode  = GPIO_Mode_AF_PP,
				.GPIO_Speed = GPIO_Speed_2MHz,
			},
		},
	},
	{
		.timer = TIM4,
		.timer_chan = TIM_Channel_3,
		.pin = {
			.gpio = GPIOB,
			.init = {
				.GPIO_Pin   = GPIO_Pin_8,
				.GPIO_Mode  = GPIO_Mode_AF_PP,
				.GPIO_Speed = GPIO_Speed_2MHz,
			},
		},
	},
	{
		.timer = TIM4,
		.timer_chan = TIM_Channel_2,
		.pin = {
			.gpio = GPIOB,
			.init = {
				.GPIO_Pin   = GPIO_Pin_7,
				.GPIO_Mode  = GPIO_Mode_AF_PP,
				.GPIO_Speed = GPIO_Speed_2MHz,
			},
		},
	},
	{
		.timer = TIM1,
		.timer_chan = TIM_Channel_1,
		.pin = {
			.gpio = GPIOA,
			.init = {
				.GPIO_Pin   = GPIO_Pin_8,
				.GPIO_Mode  = GPIO_Mode_AF_PP,
				.GPIO_Speed = GPIO_Speed_2MHz,
			},
		},
	},
	{
		.timer = TIM3,
		.timer_chan = TIM_Channel_1,
		.pin = {
			.gpio = GPIOB,
			.init = {
				.GPIO_Pin   = GPIO_Pin_4,
				.GPIO_Mode  = GPIO_Mode_AF_PP,
				.GPIO_Speed = GPIO_Speed_2MHz,
			},
		},
		.remap = GPIO_PartialRemap_TIM3,
	},
	{
		.timer = TIM2,
		.timer_chan = TIM_Channel_3,
		.pin = {
			.gpio = GPIOA,
			.init = {
				.GPIO_Pin   = GPIO_Pin_2,
				.GPIO_Mode  = GPIO_Mode_AF_PP,
				.GPIO_Speed = GPIO_Speed_2MHz,
			},
		},
	},

	// Receiver port pins
	// S3-S6 inputs are used as outputs in this case
	{
		.timer = TIM3,
		.timer_chan = TIM_Channel_3,
		.pin = {
			.gpio = GPIOB,
			.init = {
				.GPIO_Pin   = GPIO_Pin_0,
				.GPIO_Mode  = GPIO_Mode_AF_PP,
				.GPIO_Speed = GPIO_Speed_2MHz,
			},
		},
	},
	{
		.timer = TIM3,
		.timer_chan = TIM_Channel_4,
		.pin = {
			.gpio = GPIOB,
			.init = {
				.GPIO_Pin   = GPIO_Pin_1,
				.GPIO_Mode  = GPIO_Mode_AF_PP,
				.GPIO_Speed = GPIO_Speed_2MHz,
			},
		},
	},
	{
		.timer = TIM2,
		.timer_chan = TIM_Channel_1,
		.pin = {
			.gpio = GPIOA,
			.init = {
				.GPIO_Pin   = GPIO_Pin_0,
				.GPIO_Mode  = GPIO_Mode_AF_PP,
				.GPIO_Speed = GPIO_Speed_2MHz,
			},
		},
	},
	{
		.timer = TIM2,
		.timer_chan = TIM_Channel_2,
		.pin = {
			.gpio = GPIOA,
			.init = {
				.GPIO_Pin   = GPIO_Pin_1,
				.GPIO_Mode  = GPIO_Mode_AF_PP,
				.GPIO_Speed = GPIO_Speed_2MHz,
			},
		},
	},
};
#if defined(PIOS_INCLUDE_USART)

#include "pios_usart_priv.h"

static const struct pios_usart_cfg pios_usart_generic_main_cfg = {
  .regs  = USART1,
  .init = {
    .USART_BaudRate            = 57600,
    .USART_WordLength          = USART_WordLength_8b,
    .USART_Parity              = USART_Parity_No,
    .USART_StopBits            = USART_StopBits_1,
    .USART_HardwareFlowControl = USART_HardwareFlowControl_None,
    .USART_Mode                = USART_Mode_Rx | USART_Mode_Tx,
  },
  .irq = {
    .init    = {
      .NVIC_IRQChannel                   = USART1_IRQn,
      .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
      .NVIC_IRQChannelSubPriority        = 0,
      .NVIC_IRQChannelCmd                = ENABLE,
    },
  },
  .rx   = {
    .gpio = GPIOA,
    .init = {
      .GPIO_Pin   = GPIO_Pin_10,
      .GPIO_Speed = GPIO_Speed_2MHz,
      .GPIO_Mode  = GPIO_Mode_IPU,
    },
  },
  .tx   = {
    .gpio = GPIOA,
    .init = {
      .GPIO_Pin   = GPIO_Pin_9,
      .GPIO_Speed = GPIO_Speed_2MHz,
      .GPIO_Mode  = GPIO_Mode_AF_PP,
    },
  },
};

static const struct pios_usart_cfg pios_usart_generic_flexi_cfg = {
  .regs  = USART3,
  .init = {
    .USART_BaudRate            = 57600,
    .USART_WordLength          = USART_WordLength_8b,
    .USART_Parity              = USART_Parity_No,
    .USART_StopBits            = USART_StopBits_1,
    .USART_HardwareFlowControl = USART_HardwareFlowControl_None,
    .USART_Mode                = USART_Mode_Rx | USART_Mode_Tx,
  },
  .irq = {
    .init    = {
      .NVIC_IRQChannel                   = USART3_IRQn,
      .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
      .NVIC_IRQChannelSubPriority        = 0,
      .NVIC_IRQChannelCmd                = ENABLE,
    },
  },
  .rx   = {
    .gpio = GPIOB,
    .init = {
      .GPIO_Pin   = GPIO_Pin_11,
      .GPIO_Speed = GPIO_Speed_2MHz,
      .GPIO_Mode  = GPIO_Mode_IPU,
    },
  },
  .tx   = {
    .gpio = GPIOB,
    .init = {
      .GPIO_Pin   = GPIO_Pin_10,
      .GPIO_Speed = GPIO_Speed_2MHz,
      .GPIO_Mode  = GPIO_Mode_AF_PP,
    },
  },
};

#if defined(PIOS_INCLUDE_DSM)
/*
 * Spektrum/JR DSM USART
 */
#include <pios_dsm_priv.h>

static const struct pios_usart_cfg pios_usart_dsm_main_cfg = {
	.regs = USART1,
	.init = {
		.USART_BaudRate            = 115200,
		.USART_WordLength          = USART_WordLength_8b,
		.USART_Parity              = USART_Parity_No,
		.USART_StopBits            = USART_StopBits_1,
		.USART_HardwareFlowControl = USART_HardwareFlowControl_None,
		.USART_Mode                = USART_Mode_Rx,
	},
	.irq = {
		.init = {
			.NVIC_IRQChannel                   = USART1_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
			.NVIC_IRQChannelSubPriority        = 0,
			.NVIC_IRQChannelCmd                = ENABLE,
		  },
	},
	.rx = {
		.gpio = GPIOA,
		.init = {
			.GPIO_Pin   = GPIO_Pin_10,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Mode  = GPIO_Mode_IPU,
		},
	},
	.tx = {
		.gpio = GPIOA,
		.init = {
			.GPIO_Pin   = GPIO_Pin_9,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Mode  = GPIO_Mode_IN_FLOATING,
		},
	},
};

static const struct pios_dsm_cfg pios_dsm_main_cfg = {
	.bind = {
		.gpio = GPIOA,
		.init = {
			.GPIO_Pin   = GPIO_Pin_10,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Mode  = GPIO_Mode_Out_PP,
		},
	},
};

static const struct pios_usart_cfg pios_usart_dsm_flexi_cfg = {
	.regs = USART3,
	.init = {
		.USART_BaudRate            = 115200,
		.USART_WordLength          = USART_WordLength_8b,
		.USART_Parity              = USART_Parity_No,
		.USART_StopBits            = USART_StopBits_1,
		.USART_HardwareFlowControl = USART_HardwareFlowControl_None,
		.USART_Mode                = USART_Mode_Rx,
	},
	.irq = {
		.init = {
			.NVIC_IRQChannel                   = USART3_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
			.NVIC_IRQChannelSubPriority        = 0,
			.NVIC_IRQChannelCmd                = ENABLE,
		  },
	},
	.rx = {
		.gpio = GPIOB,
		.init = {
			.GPIO_Pin   = GPIO_Pin_11,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Mode  = GPIO_Mode_IPU,
		},
	},
	.tx = {
		.gpio = GPIOB,
		.init = {
			.GPIO_Pin   = GPIO_Pin_10,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Mode  = GPIO_Mode_IN_FLOATING,
		},
	},
};

static const struct pios_dsm_cfg pios_dsm_flexi_cfg = {
	.bind = {
		.gpio = GPIOB,
		.init = {
			.GPIO_Pin   = GPIO_Pin_11,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Mode  = GPIO_Mode_Out_PP,
		},
	},
};

#endif	/* PIOS_INCLUDE_DSM */

/* One slot per selectable receiver group.
 *  eg. PWM, PPM, GCS, DSMMAINPORT, DSMFLEXIPORT, SBUS
 * NOTE: No slot in this map for NONE.
 */
uint32_t pios_rcvr_group_map[MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE];

#define PIOS_COM_TELEM_RF_RX_BUF_LEN 32
#define PIOS_COM_TELEM_RF_TX_BUF_LEN 12

#define PIOS_COM_GPS_RX_BUF_LEN 32

#define PIOS_COM_TELEM_USB_RX_BUF_LEN 65
#define PIOS_COM_TELEM_USB_TX_BUF_LEN 65

#define PIOS_COM_BRIDGE_RX_BUF_LEN 65
#define PIOS_COM_BRIDGE_TX_BUF_LEN 12

uint32_t pios_com_telem_rf_id;
uint32_t pios_com_telem_usb_id;
uint32_t pios_com_vcp_id;
uint32_t pios_com_gps_id;
uint32_t pios_com_bridge_id;

/**
 * Configuration for the BMA180 chip
 */
#if defined(PIOS_INCLUDE_BMA180)
#include "pios_bma180.h"
static const struct pios_exti_cfg pios_exti_bma180_cfg __exti_config = {
	.vector = PIOS_BMA180_IRQHandler,
	.line = EXTI_Line13,
	.pin = {
		.gpio = GPIOC,
		.init = {
			.GPIO_Pin = GPIO_Pin_13,
			.GPIO_Speed = GPIO_Speed_10MHz,
			.GPIO_Mode = GPIO_Mode_IPU,
		},
	},
	.irq = {
		.init = {
			.NVIC_IRQChannel = EXTI15_10_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_LOW,
			.NVIC_IRQChannelSubPriority = 0,
			.NVIC_IRQChannelCmd = ENABLE,
		},
	},
	.exti = {
		.init = {
			.EXTI_Line = EXTI_Line13, // matches above GPIO pin
			.EXTI_Mode = EXTI_Mode_Interrupt,
			.EXTI_Trigger = EXTI_Trigger_Rising,
			.EXTI_LineCmd = ENABLE,
		},
	},
};
static const struct pios_bma180_cfg pios_bma180_cfg = {
	.exti_cfg = &pios_exti_bma180_cfg,
};
#endif /* PIOS_INCLUDE_BMA180 */

/**
 * Configuration for L3GD20 chip
 */
#if defined(PIOS_INCLUDE_L3GD20)
#include "pios_l3gd20.h"
static const struct pios_exti_cfg pios_exti_l3gd20_cfg __exti_config = {
	.vector = PIOS_L3GD20_IRQHandler,
	.line = EXTI_Line3,
	.pin = {
		.gpio = GPIOA,
		.init = {
			.GPIO_Pin   = GPIO_Pin_3,
			.GPIO_Speed = GPIO_Speed_10MHz,
			.GPIO_Mode  = GPIO_Mode_IPU,
		},
	},
	.irq = {
		.init = {
			.NVIC_IRQChannel = EXTI3_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
			.NVIC_IRQChannelSubPriority = 0,
			.NVIC_IRQChannelCmd = ENABLE,
		},
	},
	.exti = {
		.init = {
			.EXTI_Line = EXTI_Line3, // matches above GPIO pin
			.EXTI_Mode = EXTI_Mode_Interrupt,
			.EXTI_Trigger = EXTI_Trigger_Rising,
			.EXTI_LineCmd = ENABLE,
		},
	},
};

static const struct pios_l3gd20_cfg pios_l3gd20_cfg = {
	.exti_cfg = &pios_exti_l3gd20_cfg,
	.gyro_range = PIOS_L3GD20_SCALE_500_DEG,
};
#endif /* PIOS_INCLUDE_L3GD20 */


#include <pios_board_info.h>
/**
 * PIOS_Board_Init()
 * initializes all the core subsystems on this specific hardware
 * called from System/openpilot.c
 */
void PIOS_Board_Init(void) {

	/* Delay system */
	PIOS_DELAY_Init();

#if defined(PIOS_INCLUDE_SPI)
	/* Set up the SPI interface to the serial flash */
	if (PIOS_SPI_Init(&pios_spi_flash_accel_id, &pios_spi_flash_accel_cfg)) {
		PIOS_Assert(0);
	}
#endif
	PIOS_Flash_W25X_Init(pios_spi_flash_accel_id, 1);	

	PIOS_FLASHFS_Init();

	/* Initialize UAVObject libraries */
	EventDispatcherInitialize();
	UAVObjInitialize();

#if defined(PIOS_INCLUDE_RTC)
	/* Initialize the real-time clock and its associated tick */
	PIOS_RTC_Init(&pios_rtc_main_cfg);
#endif

#if defined(PIOS_INCLUDE_LED)
	PIOS_LED_Init(&pios_led_cfg);
#endif	/* PIOS_INCLUDE_LED */

	HwSettingsInitialize();

#ifndef ERASE_FLASH
	/* Initialize watchdog as early as possible to catch faults during init */
	PIOS_WDG_Init();
#endif

	/* Initialize the alarms library */
	AlarmsInitialize();

	/* Check for repeated boot failures */
	PIOS_IAP_Init();
	uint16_t boot_count = PIOS_IAP_ReadBootCount();
	if (boot_count < 3) {
		PIOS_IAP_WriteBootCount(++boot_count);
		AlarmsClear(SYSTEMALARMS_ALARM_BOOTFAULT);
	} else {
		/* Too many failed boot attempts, force hwsettings to defaults */
		HwSettingsSetDefaults(HwSettingsHandle(), 0);
		AlarmsSet(SYSTEMALARMS_ALARM_BOOTFAULT, SYSTEMALARMS_ALARM_CRITICAL);
	}

	/* Initialize the task monitor library */
	TaskMonitorInitialize();

	/* Set up pulse timers */
	PIOS_TIM_InitClock(&tim_1_cfg);
	PIOS_TIM_InitClock(&tim_2_cfg);
	PIOS_TIM_InitClock(&tim_3_cfg);
	PIOS_TIM_InitClock(&tim_4_cfg);

#if defined(PIOS_INCLUDE_USB)
	/* Initialize board specific USB data */
	PIOS_USB_BOARD_DATA_Init();

	/* Flags to determine if various USB interfaces are advertised */
	bool usb_hid_present = false;
	bool usb_cdc_present = false;

	uint8_t hwsettings_usb_devicetype;
	HwSettingsUSB_DeviceTypeGet(&hwsettings_usb_devicetype);

	switch (hwsettings_usb_devicetype) {
	case HWSETTINGS_USB_DEVICETYPE_HIDONLY:
		if (PIOS_USB_DESC_HID_ONLY_Init()) {
			PIOS_Assert(0);
		}
		usb_hid_present = true;
		break;
	case HWSETTINGS_USB_DEVICETYPE_HIDVCP:
		if (PIOS_USB_DESC_HID_CDC_Init()) {
			PIOS_Assert(0);
		}
		usb_hid_present = true;
		usb_cdc_present = true;
		break;
	case HWSETTINGS_USB_DEVICETYPE_VCPONLY:
		break;
	default:
		PIOS_Assert(0);
	}

	uint32_t pios_usb_id;
	PIOS_USB_Init(&pios_usb_id, &pios_usb_main_cfg);

#if defined(PIOS_INCLUDE_USB_CDC)

	uint8_t hwsettings_usb_vcpport;
	/* Configure the USB VCP port */
	HwSettingsUSB_VCPPortGet(&hwsettings_usb_vcpport);

	if (!usb_cdc_present) {
		/* Force VCP port function to disabled if we haven't advertised VCP in our USB descriptor */
		hwsettings_usb_vcpport = HWSETTINGS_USB_VCPPORT_DISABLED;
	}

	switch (hwsettings_usb_vcpport) {
	case HWSETTINGS_USB_VCPPORT_DISABLED:
		break;
	case HWSETTINGS_USB_VCPPORT_USBTELEMETRY:
#if defined(PIOS_INCLUDE_COM)
		{
			uint32_t pios_usb_cdc_id;
			if (PIOS_USB_CDC_Init(&pios_usb_cdc_id, &pios_usb_cdc_cfg, pios_usb_id)) {
				PIOS_Assert(0);
			}
			uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_USB_RX_BUF_LEN);
			uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_USB_TX_BUF_LEN);
			PIOS_Assert(rx_buffer);
			PIOS_Assert(tx_buffer);
			if (PIOS_COM_Init(&pios_com_telem_usb_id, &pios_usb_cdc_com_driver, pios_usb_cdc_id,
						rx_buffer, PIOS_COM_TELEM_USB_RX_BUF_LEN,
						tx_buffer, PIOS_COM_TELEM_USB_TX_BUF_LEN)) {
				PIOS_Assert(0);
			}
		}
#endif	/* PIOS_INCLUDE_COM */
		break;
	case HWSETTINGS_USB_VCPPORT_COMBRIDGE:
#if defined(PIOS_INCLUDE_COM)
		{
			uint32_t pios_usb_cdc_id;
			if (PIOS_USB_CDC_Init(&pios_usb_cdc_id, &pios_usb_cdc_cfg, pios_usb_id)) {
				PIOS_Assert(0);
			}
			uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_BRIDGE_RX_BUF_LEN);
			uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_BRIDGE_TX_BUF_LEN);
			PIOS_Assert(rx_buffer);
			PIOS_Assert(tx_buffer);
			if (PIOS_COM_Init(&pios_com_vcp_id, &pios_usb_cdc_com_driver, pios_usb_cdc_id,
						rx_buffer, PIOS_COM_BRIDGE_RX_BUF_LEN,
						tx_buffer, PIOS_COM_BRIDGE_TX_BUF_LEN)) {
				PIOS_Assert(0);
			}
		}
#endif	/* PIOS_INCLUDE_COM */
		break;
	}
#endif	/* PIOS_INCLUDE_USB_CDC */

#if defined(PIOS_INCLUDE_USB_HID)
	/* Configure the usb HID port */
	uint8_t hwsettings_usb_hidport;
	HwSettingsUSB_HIDPortGet(&hwsettings_usb_hidport);

	if (!usb_hid_present) {
		/* Force HID port function to disabled if we haven't advertised HID in our USB descriptor */
		hwsettings_usb_hidport = HWSETTINGS_USB_HIDPORT_DISABLED;
	}

	switch (hwsettings_usb_hidport) {
	case HWSETTINGS_USB_HIDPORT_DISABLED:
		break;
	case HWSETTINGS_USB_HIDPORT_USBTELEMETRY:
#if defined(PIOS_INCLUDE_COM)
		{
			uint32_t pios_usb_hid_id;
			if (PIOS_USB_HID_Init(&pios_usb_hid_id, &pios_usb_hid_cfg, pios_usb_id)) {
				PIOS_Assert(0);
			}
			uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_USB_RX_BUF_LEN);
			uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_USB_TX_BUF_LEN);
			PIOS_Assert(rx_buffer);
			PIOS_Assert(tx_buffer);
			if (PIOS_COM_Init(&pios_com_telem_usb_id, &pios_usb_hid_com_driver, pios_usb_hid_id,
						rx_buffer, PIOS_COM_TELEM_USB_RX_BUF_LEN,
						tx_buffer, PIOS_COM_TELEM_USB_TX_BUF_LEN)) {
				PIOS_Assert(0);
			}
		}
#endif	/* PIOS_INCLUDE_COM */
		break;
	}

#endif	/* PIOS_INCLUDE_USB_HID */

#endif	/* PIOS_INCLUDE_USB */

	/* Configure the main IO port */
	uint8_t hwsettings_DSMxBind;
	HwSettingsDSMxBindGet(&hwsettings_DSMxBind);
	uint8_t hwsettings_cc_mainport;
	HwSettingsCC_MainPortGet(&hwsettings_cc_mainport);

	switch (hwsettings_cc_mainport) {
	case HWSETTINGS_CC_MAINPORT_DISABLED:
		break;
	case HWSETTINGS_CC_MAINPORT_TELEMETRY:
#if defined(PIOS_INCLUDE_TELEMETRY_RF)
		{
			uint32_t pios_usart_generic_id;
			if (PIOS_USART_Init(&pios_usart_generic_id, &pios_usart_generic_main_cfg)) {
				PIOS_Assert(0);
			}

			uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_RX_BUF_LEN);
			uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_TX_BUF_LEN);
			PIOS_Assert(rx_buffer);
			PIOS_Assert(tx_buffer);
			if (PIOS_COM_Init(&pios_com_telem_rf_id, &pios_usart_com_driver, pios_usart_generic_id,
					  rx_buffer, PIOS_COM_TELEM_RF_RX_BUF_LEN,
					  tx_buffer, PIOS_COM_TELEM_RF_TX_BUF_LEN)) {
				PIOS_Assert(0);
			}
		}
#endif	/* PIOS_INCLUDE_TELEMETRY_RF */
		break;
	case HWSETTINGS_CC_MAINPORT_SBUS:
#if defined(PIOS_INCLUDE_SBUS)
		{
			uint32_t pios_usart_sbus_id;
			if (PIOS_USART_Init(&pios_usart_sbus_id, &pios_usart_sbus_main_cfg)) {
				PIOS_Assert(0);
			}

			uint32_t pios_sbus_id;
			if (PIOS_SBus_Init(&pios_sbus_id, &pios_sbus_cfg, &pios_usart_com_driver, pios_usart_sbus_id)) {
				PIOS_Assert(0);
			}

			uint32_t pios_sbus_rcvr_id;
			if (PIOS_RCVR_Init(&pios_sbus_rcvr_id, &pios_sbus_rcvr_driver, pios_sbus_id)) {
				PIOS_Assert(0);
			}
			pios_rcvr_group_map[MANUALCONTROLSETTINGS_CHANNELGROUPS_SBUS] = pios_sbus_rcvr_id;

		}
#endif	/* PIOS_INCLUDE_SBUS */
		break;
	case HWSETTINGS_CC_MAINPORT_GPS:
#if defined(PIOS_INCLUDE_GPS)
		{
			uint32_t pios_usart_generic_id;
			if (PIOS_USART_Init(&pios_usart_generic_id, &pios_usart_generic_main_cfg)) {
				PIOS_Assert(0);
			}

			uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_GPS_RX_BUF_LEN);
			PIOS_Assert(rx_buffer);
			if (PIOS_COM_Init(&pios_com_gps_id, &pios_usart_com_driver, pios_usart_generic_id,
					  rx_buffer, PIOS_COM_GPS_RX_BUF_LEN,
					  NULL, 0)) {
				PIOS_Assert(0);
			}
		}
#endif	/* PIOS_INCLUDE_GPS */
		break;
	case HWSETTINGS_CC_MAINPORT_DSM2:
	case HWSETTINGS_CC_MAINPORT_DSMX10BIT:
	case HWSETTINGS_CC_MAINPORT_DSMX11BIT:
#if defined(PIOS_INCLUDE_DSM)
		{
			enum pios_dsm_proto proto;
			switch (hwsettings_cc_mainport) {
			case HWSETTINGS_CC_MAINPORT_DSM2:
				proto = PIOS_DSM_PROTO_DSM2;
				break;
			case HWSETTINGS_CC_MAINPORT_DSMX10BIT:
				proto = PIOS_DSM_PROTO_DSMX10BIT;
				break;
			case HWSETTINGS_CC_MAINPORT_DSMX11BIT:
				proto = PIOS_DSM_PROTO_DSMX11BIT;
				break;
			default:
				PIOS_Assert(0);
				break;
			}

			uint32_t pios_usart_dsm_id;
			if (PIOS_USART_Init(&pios_usart_dsm_id, &pios_usart_dsm_main_cfg)) {
				PIOS_Assert(0);
			}

			uint32_t pios_dsm_id;
			if (PIOS_DSM_Init(&pios_dsm_id,
					  &pios_dsm_main_cfg,
					  &pios_usart_com_driver,
					  pios_usart_dsm_id,
					  proto, 0)) {
				PIOS_Assert(0);
			}

			uint32_t pios_dsm_rcvr_id;
			if (PIOS_RCVR_Init(&pios_dsm_rcvr_id, &pios_dsm_rcvr_driver, pios_dsm_id)) {
				PIOS_Assert(0);
			}
			pios_rcvr_group_map[MANUALCONTROLSETTINGS_CHANNELGROUPS_DSMMAINPORT] = pios_dsm_rcvr_id;
		}
#endif	/* PIOS_INCLUDE_DSM */
		break;
	case HWSETTINGS_CC_MAINPORT_COMAUX:
		break;
	case HWSETTINGS_CC_MAINPORT_COMBRIDGE:
		{
			uint32_t pios_usart_generic_id;
			if (PIOS_USART_Init(&pios_usart_generic_id, &pios_usart_generic_main_cfg)) {
				PIOS_Assert(0);
			}

			uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_BRIDGE_RX_BUF_LEN);
			PIOS_Assert(rx_buffer);
			uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_BRIDGE_TX_BUF_LEN);
			PIOS_Assert(tx_buffer);
			if (PIOS_COM_Init(&pios_com_bridge_id, &pios_usart_com_driver, pios_usart_generic_id,
						rx_buffer, PIOS_COM_BRIDGE_RX_BUF_LEN,
						tx_buffer, PIOS_COM_BRIDGE_TX_BUF_LEN)) {
				PIOS_Assert(0);
			}
		}
		break;
	}

	/* Configure the flexi port */
	uint8_t hwsettings_cc_flexiport;
	HwSettingsCC_FlexiPortGet(&hwsettings_cc_flexiport);

	switch (hwsettings_cc_flexiport) {
	case HWSETTINGS_CC_FLEXIPORT_DISABLED:
		break;
	case HWSETTINGS_CC_FLEXIPORT_TELEMETRY:
#if defined(PIOS_INCLUDE_TELEMETRY_RF)
		{
			uint32_t pios_usart_generic_id;
			if (PIOS_USART_Init(&pios_usart_generic_id, &pios_usart_generic_flexi_cfg)) {
				PIOS_Assert(0);
			}
			uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_RX_BUF_LEN);
			uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_TX_BUF_LEN);
			PIOS_Assert(rx_buffer);
			PIOS_Assert(tx_buffer);
			if (PIOS_COM_Init(&pios_com_telem_rf_id, &pios_usart_com_driver, pios_usart_generic_id,
  					  rx_buffer, PIOS_COM_TELEM_RF_RX_BUF_LEN,
					  tx_buffer, PIOS_COM_TELEM_RF_TX_BUF_LEN)) {
				PIOS_Assert(0);
			}
		}
#endif /* PIOS_INCLUDE_TELEMETRY_RF */
		break;
	case HWSETTINGS_CC_FLEXIPORT_COMBRIDGE:
		{
			uint32_t pios_usart_generic_id;
			if (PIOS_USART_Init(&pios_usart_generic_id, &pios_usart_generic_flexi_cfg)) {
				PIOS_Assert(0);
			}

			uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_BRIDGE_RX_BUF_LEN);
			uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_BRIDGE_TX_BUF_LEN);
			PIOS_Assert(rx_buffer);
			PIOS_Assert(tx_buffer);
			if (PIOS_COM_Init(&pios_com_bridge_id, &pios_usart_com_driver, pios_usart_generic_id,
						rx_buffer, PIOS_COM_BRIDGE_RX_BUF_LEN,
						tx_buffer, PIOS_COM_BRIDGE_TX_BUF_LEN)) {
				PIOS_Assert(0);
			}
		}
		break;
	case HWSETTINGS_CC_FLEXIPORT_GPS:
#if defined(PIOS_INCLUDE_GPS)
		{
			uint32_t pios_usart_generic_id;
			if (PIOS_USART_Init(&pios_usart_generic_id, &pios_usart_generic_flexi_cfg)) {
				PIOS_Assert(0);
			}
			uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_GPS_RX_BUF_LEN);
			PIOS_Assert(rx_buffer);
			if (PIOS_COM_Init(&pios_com_gps_id, &pios_usart_com_driver, pios_usart_generic_id,
					  rx_buffer, PIOS_COM_GPS_RX_BUF_LEN,
					  NULL, 0)) {
				PIOS_Assert(0);
			}
		}
#endif	/* PIOS_INCLUDE_GPS */
		break;
	case HWSETTINGS_CC_FLEXIPORT_DSM2:
	case HWSETTINGS_CC_FLEXIPORT_DSMX10BIT:
	case HWSETTINGS_CC_FLEXIPORT_DSMX11BIT:
#if defined(PIOS_INCLUDE_DSM)
		{
			enum pios_dsm_proto proto;
			switch (hwsettings_cc_flexiport) {
			case HWSETTINGS_CC_FLEXIPORT_DSM2:
				proto = PIOS_DSM_PROTO_DSM2;
				break;
			case HWSETTINGS_CC_FLEXIPORT_DSMX10BIT:
				proto = PIOS_DSM_PROTO_DSMX10BIT;
				break;
			case HWSETTINGS_CC_FLEXIPORT_DSMX11BIT:
				proto = PIOS_DSM_PROTO_DSMX11BIT;
				break;
			default:
				PIOS_Assert(0);
				break;
			}

			uint32_t pios_usart_dsm_id;
			if (PIOS_USART_Init(&pios_usart_dsm_id, &pios_usart_dsm_flexi_cfg)) {
				PIOS_Assert(0);
			}

			uint32_t pios_dsm_id;
			if (PIOS_DSM_Init(&pios_dsm_id,
					  &pios_dsm_flexi_cfg,
					  &pios_usart_com_driver,
					  pios_usart_dsm_id,
					  proto, hwsettings_DSMxBind)) {
				PIOS_Assert(0);
			}

			uint32_t pios_dsm_rcvr_id;
			if (PIOS_RCVR_Init(&pios_dsm_rcvr_id, &pios_dsm_rcvr_driver, pios_dsm_id)) {
				PIOS_Assert(0);
			}
			pios_rcvr_group_map[MANUALCONTROLSETTINGS_CHANNELGROUPS_DSMFLEXIPORT] = pios_dsm_rcvr_id;
		}
#endif	/* PIOS_INCLUDE_DSM */
		break;
	case HWSETTINGS_CC_FLEXIPORT_COMAUX:
		break;
	case HWSETTINGS_CC_FLEXIPORT_I2C:
#if defined(PIOS_INCLUDE_I2C)
		{
			if (PIOS_I2C_Init(&pios_i2c_flexi_adapter_id, &pios_i2c_flexi_adapter_cfg)) {
				PIOS_Assert(0);
			}
		}
#endif	/* PIOS_INCLUDE_I2C */
		break;
	}

	/* Configure the rcvr port */
	uint8_t hwsettings_rcvrport;
	HwSettingsCC_RcvrPortGet(&hwsettings_rcvrport);

	switch (hwsettings_rcvrport) {
	case HWSETTINGS_CC_RCVRPORT_DISABLED:
		break;
	case HWSETTINGS_CC_RCVRPORT_PWM:
#if defined(PIOS_INCLUDE_PWM)
		{
			uint32_t pios_pwm_id;
			PIOS_PWM_Init(&pios_pwm_id, &pios_pwm_cfg);

			uint32_t pios_pwm_rcvr_id;
			if (PIOS_RCVR_Init(&pios_pwm_rcvr_id, &pios_pwm_rcvr_driver, pios_pwm_id)) {
				PIOS_Assert(0);
			}
			pios_rcvr_group_map[MANUALCONTROLSETTINGS_CHANNELGROUPS_PWM] = pios_pwm_rcvr_id;
		}
#endif	/* PIOS_INCLUDE_PWM */
		break;
	case HWSETTINGS_CC_RCVRPORT_PPM:
	case HWSETTINGS_CC_RCVRPORT_PPMOUTPUTS:
#if defined(PIOS_INCLUDE_PPM)
		{
			uint32_t pios_ppm_id;
			PIOS_PPM_Init(&pios_ppm_id, &pios_ppm_cfg);

			uint32_t pios_ppm_rcvr_id;
			if (PIOS_RCVR_Init(&pios_ppm_rcvr_id, &pios_ppm_rcvr_driver, pios_ppm_id)) {
				PIOS_Assert(0);
			}
			pios_rcvr_group_map[MANUALCONTROLSETTINGS_CHANNELGROUPS_PPM] = pios_ppm_rcvr_id;
		}
#endif	/* PIOS_INCLUDE_PPM */
		break;
	}

#if defined(PIOS_INCLUDE_GCSRCVR)
	GCSReceiverInitialize();
	PIOS_GCSRCVR_Init();
	uint32_t pios_gcsrcvr_rcvr_id;
	if (PIOS_RCVR_Init(&pios_gcsrcvr_rcvr_id, &pios_gcsrcvr_rcvr_driver, 0)) {
		PIOS_Assert(0);
	}
	pios_rcvr_group_map[MANUALCONTROLSETTINGS_CHANNELGROUPS_GCS] = pios_gcsrcvr_rcvr_id;
#endif	/* PIOS_INCLUDE_GCSRCVR */

	/* Remap AFIO pin for PB4 (Servo 5 Out)*/
	GPIO_PinRemapConfig( GPIO_Remap_SWJ_NoJTRST, ENABLE);

#ifndef PIOS_DEBUG_ENABLE_DEBUG_PINS
	switch (hwsettings_rcvrport) {
		case HWSETTINGS_CC_RCVRPORT_DISABLED:
		case HWSETTINGS_CC_RCVRPORT_PWM:
		case HWSETTINGS_CC_RCVRPORT_PPM:
			PIOS_Servo_Init(&pios_servo_cfg);
			break;
		case HWSETTINGS_CC_RCVRPORT_PPMOUTPUTS:
		case HWSETTINGS_CC_RCVRPORT_OUTPUTS:
			PIOS_Servo_Init(&pios_servo_rcvr_cfg);
			break;
	}
#else
	PIOS_DEBUG_Init(&pios_tim_servo_all_channels, NELEMENTS(pios_tim_servo_all_channels));
#endif	/* PIOS_DEBUG_ENABLE_DEBUG_PINS */

	const struct pios_board_info * bdinfo = &pios_board_info_blob;

	switch(bdinfo->board_rev) {
		case 0x01:
			// Revision 1 with invensense gyros, start the ADC
			PIOS_ADC_Init();
#if defined(PIOS_INCLUDE_ADXL345)
			PIOS_ADXL345_Init(pios_spi_flash_accel_id, 0);
#endif
			break;
		case 0x02:
			// Revision 2 with L3GD20 gyros, start a SPI interface and connect to it
			GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

#if defined(PIOS_INCLUDE_L3GD20)
			// Set up the SPI interface to the serial flash 
			if (PIOS_SPI_Init(&pios_spi_gyro_id, &pios_spi_gyro_cfg)) {
				PIOS_Assert(0);
			}
			PIOS_L3GD20_Attach(pios_spi_gyro_id);
			PIOS_L3GD20_Init(&pios_l3gd20_cfg);
#endif /* PIOS_INCLUDE_L3GD20 */

#if defined(PIOS_INCLUDE_BMA180)
			PIOS_BMA180_Attach(pios_spi_flash_accel_id);
			PIOS_BMA180_Init(&pios_bma180_cfg);
#endif /* PIOS_INCLUDE_BMA180 */

			break;
		default:
			PIOS_Assert(0);
	}

	PIOS_GPIO_Init();

	/* Make sure we have at least one telemetry link configured or else fail initialization */
	PIOS_Assert(pios_com_telem_rf_id || pios_com_telem_usb_id);
}

/**
 * @}
 */
